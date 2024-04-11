#ifndef KSOCKET_HPP
#define KSOCKET_HPP

/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#include <thread>
#include <vector>
#include <mutex>
#include <functional>

class Client {
    private:
        int sockfd;
        struct sockaddr_in cli_addr;
        socklen_t clilen;
        fd_set readfds;
        int n;

    public:
        Client(int sockfd, struct sockaddr_in cli_addr, socklen_t clilen);
        ~Client();
        std::string read();
        void write(const std::string& message);
        void close_connection();
        bool is_open();
};


class Socket {
    private:
        int sockfd, portno;
        struct sockaddr_in serv_addr;
        int n;

    public:
        Socket(int portno);
        ~Socket();
        void create();
        void listen();
        Client* accept();
        void close_server();
};


class SocketRunner {
    private:
        std::vector<std::thread> threads;
        std::vector<Client*> clients;
        Socket* socket;
        std::mutex mtx;
        std::function<void(const std::string&)> _callback;

    public:
        SocketRunner(int portno);
        ~SocketRunner();
        void callback(const std::string& message);
        void set_callback(std::function<void(const std::string&)> callback);
        void send_to_clients(const std::string& message);
        void run();
        void stop();
};

#endif