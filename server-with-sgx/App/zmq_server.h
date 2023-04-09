#include <zmq.hpp>
#include <iostream>
#include <unistd.h>
#include <future>

class ZmqServer{
public:
    ZmqServer();
    void bind(std::string protocol, int port);
    std::string receive();
    void send(std::string msg);
    void sendmore(std::string msg);

private:
    zmq::context_t m_context;
    zmq::socket_t socket;
};