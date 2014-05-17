#include <iostream>
#include <set>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <zmq.hpp>

#include "messages.pb.h"

using namespace std;

class Interrupt {

public:
    Interrupt() {
        s_interrupted = false;
        s_catch_signals();
    }

    bool is_interrupted() {
        return s_interrupted;
    }

private:
    static void s_signal_handler(int /*signal_value*/) {
        s_interrupted = true;
    }

    void s_catch_signals(void){
        struct sigaction action;
        action.sa_handler = s_signal_handler; // to przypisanie wymusza stosowanie statycznego s_signal_handlera, i tyle z hermetyzacji...
        action.sa_flags = 0;
        sigemptyset(&action.sa_mask);
        sigaction(SIGINT, &action, NULL);
        sigaction(SIGTERM, &action, NULL);
    }

    static bool s_interrupted;

};
bool Interrupt::s_interrupted;

class Connection {
public:
    Connection(const string a_ttype, string a_my_id, string a_remote_id)
        : transport_type(a_ttype),
          context(1),
          receiver(context, ZMQ_SUB),
          my_id(a_my_id),
          remote_id(a_remote_id),
          sender(context, ZMQ_PUB),
          receive_waiting(0), send_waiting(1) {
        init();
    }

    void s_recv(zmq::socket_t* socket, Message* msg) {
        zmq::message_t message;
        socket->recv(&message, 0);
        int size = message.size();
        char *s = (char*) malloc(size + 1);
        memcpy(s, message.data(), size);
        s[size] = 0;
        string sstr(s, size);
        free(s);
        msg->ParseFromString(sstr);
    }

    void s_send(zmq::socket_t* socket, Message* msg)
    {
        string smsg;
        msg->SerializeToString(&smsg);
        zmq::message_t message(smsg.size());
        memcpy (message.data(), smsg.c_str(), smsg.size());
        socket->send(message);
    }

    void init() {
        try
        {
            receiver.bind((transport_type + my_id).c_str());
            //receiver.bind("tcp://*:5432");
        }
        catch(exception& e)
        {
            throw string("Błędny adres wewnętrzny lub port");

            //return 1;
        }

        peers.insert(my_id);
        receiver.setsockopt(ZMQ_SUBSCRIBE, "", 0); // przyjmujemy wszyswtkie wiadomosci

        int reconnect_time_ms = 1000;
        sender.setsockopt(ZMQ_RECONNECT_IVL_MAX, &reconnect_time_ms, sizeof(reconnect_time_ms));

        if(remote_id != "") {
            peers.insert(remote_id);

            try{
                sender.connect((transport_type + remote_id).c_str());
            }
            catch(exception& e) {
                throw string("Błędny adres zewnętrzny lub port");
            }

            Message msg;
            msg.set_type(Message::JOIN);
            msg.set_src(my_id);
            Connection::s_send(&sender, &msg);
        }


        items[0].socket  = receiver;   items[1].socket  = NULL;
        items[0].fd      = 0;          items[1].fd      = 0;
        items[0].events  = ZMQ_POLLIN; items[1].events  = ZMQ_POLLIN;
        items[0].revents = 0;          items[1].revents = 0;

        cout << "Podłączono do czatu! Można wpisywać wiadomości + ENTER. CTRL+C wyłącza czat." << endl;
    }

    Interrupt interrupt;
    const string transport_type;
    set<string> peers;
    zmq::context_t context;
    zmq::socket_t receiver;
    string my_id;
    string remote_id;
    zmq::pollitem_t items[2];
    zmq::socket_t sender;
    const int receive_waiting, send_waiting;
};

class DChat  {
public:
    DChat(const int argc, char** const argv) {
        checkArgs(argc, argv);
        connection = new Connection("tcp://", argv[1], argv[2]);
        mainLoop();
    }
    void checkArgs(const int argc, char** const argv) throw(string)  {
        if(argc < 2 || argc > 3) {
            cerr << endl;
            cerr << "Sposób użycia: " << string(argv[0]) << " adres_lokalny:port [ adres_zewnętrzny:port ]" << endl;
            cerr << "- adres_lokalny:port    = lokalny adres IP, który będzie używany jako identyfikator" << endl;
            cerr << "- adres_zewnętrzny:port = zewnętrzny adres IP rozmówcy. Jeżeli zostanie pominięty, program spróbuje połączyć się z dowolnym klientem w sieci lokalnej" << endl;
            cerr << endl;
            throw string("Zbyt mało argumentów.");
        }
    }

    ~DChat() {
        delete connection;
    }

private:
    void send(Message & msg) {
        connection->s_send(&connection->sender, &msg);
    }

    void receive(Message & msg) {
        connection->s_recv(&connection->receiver, &msg);
    }

    short message_is_waiting_to_receive() {
        return connection->items[connection->receive_waiting].revents & ZMQ_POLLIN;
    }

    short message_is_waiting_to_send() {
        return connection->items[connection->send_waiting].revents & ZMQ_POLLIN;
    }

    void create_and_send_message(Message::CommandType message_type, string data) {
        Message msg;
        msg.set_type(message_type);
        msg.set_src(connection->my_id);
        msg.set_data(data);
        send(msg);
    }

    void mainLoop() {
        while(true) {
            try {
                if(connection->interrupt.is_interrupted()) {
                    cout << "Wyłączanie czatu" << endl;
                    create_and_send_message(Message::LEAVE, "");
                    break;
                }
                if(zmq::poll(connection->items, 2, 5000) == 0) continue;

                if(message_is_waiting_to_receive()) {
                    Message received_msg;
                    receive(received_msg);

                    switch(received_msg.type()) {

                        case Message::JOIN:{
                            cout << "Rozmówca '" << received_msg.src() << "' dołączył do czatu" << endl;
                            pair<set<string>::iterator, bool> p = connection->peers.insert(received_msg.src());
                            if(p.second)
                                connection->sender.connect((connection->transport_type + received_msg.src()).c_str());
                            for(set<string>::iterator it = connection->peers.begin(); it != connection->peers.end(); ++it)
                                create_and_send_message(Message::HOST, *it);
                            break;
                        }

                        case Message::LEAVE: {
                            cout << "Rozmówca '" << received_msg.src() << "' opuścił grupę" << endl;
                            connection->peers.erase(received_msg.src());
                            break;
                        }

                        case Message::HOST:{
                            if(connection->peers.find(received_msg.data()) == connection->peers.end()) {
                                cout << "Rozmówca '" << received_msg.data() << "' dołączył do grupy" << endl;
                                connection->peers.insert(received_msg.data());
                                connection->sender.connect((connection->transport_type + received_msg.data()).c_str());
                            }
                            break;
                        }

                        case Message::MSG:{
                            string message = "[" + received_msg.src() + "] " + received_msg.data();
                            cout << message << endl;
                            break;
                        }

                        default:
                            assert(false);
                    }
                }

                if(message_is_waiting_to_send()) {
                    string s;
                    getline(cin, s);

                    create_and_send_message(Message::MSG, s);

                    string message = "[" + connection->my_id + "] " + s;
                    cout << message << endl;
                }
            } catch(zmq::error_t& e) { std::cout << "\nOtrzymano próbę wyłączenia czatu..." << std::endl; }
        }
    }

private:
    Connection * connection;

};

int main(int argc, char** argv) {
    try {
        DChat chat(argc, argv);
    } catch(string s) {
        cerr << s << endl;
        return 1;
    }

    return 0;
}
