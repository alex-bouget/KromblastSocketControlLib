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
    if (sockfd == -1) {
        return;
    }
    int result = ::close(sockfd);
    sockfd = -1;
    if (result < 0)
        std::cout << "ERROR closing socket." << std::endl;
    else
        std::cout << "socket disconnected." << std::endl;
}

bool Client::read(std::string* message) {
    char buffer[1024];
    bzero(buffer, 1024);
    int x = 0;
    do {
        n = ::read(sockfd, buffer + x, 1);
        if (n <= 0) {
            std::cout << "ERROR reading from socket." << std::endl;
            return false;
        }
        x++;
    } while (x < 1023 && buffer[x - 1] != '\0');
    if (n <= 0) {
        std::cout << "ERROR reading from socket." << std::endl;
        return false;
    }
    *message = std::string(buffer);
    return n > 0;
}

Client::Client(int sockfd, struct sockaddr_in cli_addr, socklen_t clilen) {
    this->sockfd = sockfd;
    this->cli_addr = cli_addr;
    this->clilen = clilen;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
}

Client::~Client() {
    close_connection();
}

void Client::write(const std::string& message) {
    n = ::write(sockfd, message.c_str(), message.length());
    n += ::write(sockfd, "\0", 1);
    if (n <= 0)
        std::cout << "ERROR writing to socket." << std::endl;
}

void Client::close_connection() {
    if (!is_open()) {
        return;
    }
    int result = ::close(sockfd);
    sockfd = -1;
    if (result < 0)
        std::cout << "ERROR closing client socket." << std::endl;
    else
        std::cout << "client socket disconnected." << std::endl;
}

bool Client::is_open() {
    if (sockfd == -1) {
        return false;
    }
    fd_set rsd = readfds;

    int sel = select(sockfd + 1, &rsd, 0, 0, 0);

    if (sel < 0) {
        std::cout << "ERROR on select." << std::endl;
        sockfd = -1;
        return false;
    }

    return sel > 0;
}

SocketRunner::SocketRunner(int portno) {
    socket = new Socket(portno);
}

SocketRunner::~SocketRunner() {
    stop();
    delete socket;
}

void SocketRunner::run() {
    socket->create();
    socket->listen();
    while (true) {
        Client* client = socket->accept();
        threads.push_back(std::thread([client, this] {
            while (client->is_open()) {
                std::string message;
                if (!client->read(&message)) {
                    break;
                }
                this->callback(message);
            }
            client->close_connection();
            this->mtx.lock();
            clients.erase(std::find(clients.begin(), clients.end(), client));
            this->mtx.unlock();
            delete client;
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