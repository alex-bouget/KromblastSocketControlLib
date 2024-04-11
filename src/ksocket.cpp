#include "ksocket.hpp"
#include <arpa/inet.h>
#include <iostream>


Socket::Socket(int portno) {
    this->portno = portno;
}

Socket::~Socket() {
    close_server();
}

void Socket::create() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->sockfd < 0) {
        std::cout << "ERROR opening socket." << std::endl;
        std::exit(1);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "ERROR on binding." << std::endl;
        exit(1);
    }

    std::cout << "Server started." << std::endl;
    std::cout << "Socket: " << sockfd << std::endl;
    std::cout << "Port: " << portno << std::endl;
}

void Socket::listen() {
    ::listen(sockfd, 5);
}

Client* Socket::accept() {
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    int newsockfd = ::accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0)
        std::cout << "ERROR on accept." << std::endl;
    std::cout << "Connection accepted." << std::endl;
    std::cout << "Socket: " << newsockfd << std::endl;
    std::cout << "Port: " << portno << std::endl;
    std::cout << "IP: " << inet_ntoa(cli_addr.sin_addr) << std::endl;
    return new Client(newsockfd, cli_addr, clilen);
}

void Socket::close_server() {
    int result = ::close(sockfd);
    if (result < 0)
        std::cout << "ERROR closing socket." << std::endl;
    else
        std::cout << "socket disconnected." << std::endl;
}

std::string Client::read() {
    char buffer[256];
    bzero(buffer, 256);
    n = ::read(sockfd, buffer, 255);
    if (n < 0) 
        std::cout << "ERROR reading from socket." << std::endl;
    return std::string(buffer);
}

Client::Client(int sockfd, struct sockaddr_in cli_addr, socklen_t clilen) {
    this->sockfd = sockfd;
    this->cli_addr = cli_addr;
    this->clilen = clilen;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
}

Client::~Client() {
    // close_connection();
}

void Client::write(const std::string& message) {
    n = ::write(sockfd, message.c_str(), message.length());
    if (n < 0)
        std::cout << "ERROR writing to socket." << std::endl;
}

void Client::close_connection() {
    int result = ::close(sockfd);
    if (result < 0)
        std::cout << "ERROR closing client socket." << std::endl;
    else
        std::cout << "client socket disconnected." << std::endl;
}

bool Client::is_open() {
    fd_set rsd = readfds;

    int sel = select(sockfd + 1, &rsd, 0, 0, 0);

    return sel > 0;
}

SocketRunner::SocketRunner(int portno) {
    socket = new Socket(portno);
}

SocketRunner::~SocketRunner() {
    delete socket;
}

void SocketRunner::run() {
    socket->create();
    socket->listen();
    while (true) {
        Client* client = socket->accept();
        threads.push_back(std::thread([client, this] {
            while (client->is_open()) {
                std::string message = client->read();
                this->callback(message);
            }
            client->close_connection();
            this->mtx.lock();
            clients.erase(std::find(clients.begin(), clients.end(), client));
            this->mtx.unlock();
        }));
        mtx.lock();
        clients.push_back(client);
        mtx.unlock();
    }
}

void SocketRunner::stop() {
    this->mtx.lock();
    for (auto client : clients) {
        client->close_connection();
    }
    this->mtx.unlock();
    for (auto& thread : threads) {
        thread.join();
    }
    socket->close_server();
}

void SocketRunner::callback(const std::string& message) {
    _callback(message);
}

void SocketRunner::set_callback(std::function<void(const std::string&)> callback) {
    _callback = callback;
}

void SocketRunner::send_to_clients(const std::string& message) {
    this->mtx.lock();
    for (auto client : clients) {
        client->write(message);
    }
    this->mtx.unlock();
}