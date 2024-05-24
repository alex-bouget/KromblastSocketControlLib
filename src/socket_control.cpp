#include <string>
#include "socket_control.hpp"
#include "kromblast_api_plugin_utils.hpp"

std::string SocketControl::get_version()
{
    return "0.1.0";
}

void SocketControl::at_start()
{
    this->ksocket = std::make_unique<SocketRunner>(9434);
    this->ksocket->set_callback([this](const std::string &message)
                                { this->hand_socket(message); });
    socket_thread = std::make_unique<std::thread>([this]()
                                                  { this->ksocket->run(); });
    kromblast().get_logger()->log("SocketControl", "Socket started on port 9434");
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
            kromblast().get_logger()->log("SocketControl", "Client listening to " + msg);
            kromblast().get_dispatcher()->listen(msg, this);
        }
        else if (command == "execute")
        {
            kromblast().get_window()->inject("kromblast.socket.send(" + msg + ");");
        }
        else
        {
            this->handle_kb_command(command, msg);
        }
        return;
    }
    kromblast().get_logger()->log("SocketControl", "Received message from client on channel " + channel + ": " + msg);
    kromblast().get_dispatcher()->dispatch(channel, msg);
}

void SocketControl::load_functions()
{
    kromblast().get_plugin()->claim_callback(
        "kromblast.socket.send",
        1,
        BIND_CALLBACK(SocketControl::send_to_socket),
        std::vector<std::regex>{std::regex("^.*$")});
}

void SocketControl::handle(Kromblast::Api::Signal signal)
{
    if (signal.channel == "_kromblast" && signal.message == "stop")
    {
        this->ksocket->stop();
    }
    this->ksocket->send_to_clients(signal.channel + ",: " + signal.message);
}

void SocketControl::handle_kb_command(const std::string &command, const std::string &message)
{
    if (command == "navigate")
    {
        kromblast().get_window()->navigate(message);
    }
    else if (command == "init_inject")
    {
        kromblast().get_window()->init_inject(message);
    }
    else if (command == "inject")
    {
        kromblast().get_window()->inject(message);
    }
    else if (command == "get_url")
    {
    }
}

std::string SocketControl::send_to_socket(Kromblast::Core::kromblast_callback_called_t *parameters)
{
    ksocket->send_to_clients(parameters->args.at(0));
    return R"({"ok": true})";
}