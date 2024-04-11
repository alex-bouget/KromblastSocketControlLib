#include <string>
#include "socket_control.hpp"

std::string SocketControl::get_version()
{
    return "0.1.0";
}

void SocketControl::set_kromblast(void *kromblast)
{
    this->kromblast = (Kromblast::Api::KromblastInterface *)kromblast;
}

void SocketControl::hand_socket(const std::string &message)
{
    if (message.empty())
    {
        return;
    }
    int pos = message.find(",: ");
    if ((unsigned)pos == std::string::npos)
    {
        return;
    }
    std::string channel = message.substr(0, pos);
    std::string msg = message.substr(pos + 3);
    if (channel.starts_with("_socket_control"))
    {
        std::string command = channel.substr(15);
        if (command == "listen")
        {
            this->kromblast->log("SocketControl", "Client listening to " + msg);
            this->kromblast->get_dispatcher()->listen(msg, this);
        } else {
            this->handle_kb_command(command, msg);
        }
        return;
    }
    this->kromblast->log("SocketControl", "Received message from client on channel " + channel + ": " + msg);
    this->kromblast->get_dispatcher()->dispatch(channel, msg);
}

void SocketControl::load_functions()
{
    this->ksocket = new SocketRunner(9434);
    this->ksocket->set_callback([this](const std::string &message)
                                { this->hand_socket(message); });
    socket_thread = new std::thread([this]
                                    { this->ksocket->run(); });
    this->kromblast->log("SocketControl", "Socket started on port 9434");
}

void SocketControl::handle(Kromblast::Api::Signal signal)
{
    if (signal.channel == "_kromblast" && signal.message == "stop")
    {
        this->ksocket->stop();
    }
    this->ksocket->send_to_clients(signal.channel + ",: " + signal.message);
}

void SocketControl::handle_kb_command(const std::string& command, const std::string& message)
{
    if (command == "navigate")
    {
        this->kromblast->get_window()->navigate(message);
    } else if (command == "init_inject")
    {
        this->kromblast->get_window()->init_inject(message);
    } else if (command == "inject")
    {
        this->kromblast->get_window()->inject(message);
    } else if (command == "get_url")
    {
    }

}