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

socket_data SocketControl::get_socket_data(const std::string &message)
{
    socket_data data;

    data.error = 0;

    // \1 player|tmpId \2 channel:message
    int startHeader = message.find("\1");
    int middleHeader = message.find("|", startHeader);
    int endHeader = message.find("\2", middleHeader);

    if (startHeader == std::string::npos || middleHeader == std::string::npos || endHeader == std::string::npos)
    {
        data.error = SOCKET_DATA_ERROR_NO_HEADER;
        return data;
    }

    data.player = message.substr(startHeader + 1, middleHeader - startHeader - 1);
    data.tmpId = message.substr(middleHeader + 1, endHeader - middleHeader - 1);

    int startMessage = endHeader;
    int middleMessage = message.find(":", startMessage);
    int endMessage = message.size();

    if (startMessage == std::string::npos || middleMessage == std::string::npos || endMessage == std::string::npos)
    {
        data.error = SOCKET_DATA_ERROR_NO_BODY;
        return data;
    }

    data.channel = message.substr(startMessage + 1, middleMessage - startMessage - 1);
    data.message = message.substr(middleMessage + 1, endMessage - middleMessage - 1);

    return data;
}

void SocketControl::hand_socket(const std::string &message)
{
    if (message.empty())
    {
        return;
    }
    socket_data data = get_socket_data(message);
    if (data.error)
    {
        kromblast().get_logger()->log("SocketControl", "Error " + std::to_string(data.error) + " parsing message: " + message);
        return;
    }
    if (data.player == "_socket_control")
    {
        if (data.channel == "listen")
        {
            kromblast().get_logger()->log("SocketControl", "Client listening to " + data.message);
            kromblast().get_dispatcher()->listen(data.message, this);
        }
        else if (data.channel == "execute")
        {
            kromblast().get_window()->inject("kromblast.socket.send(" + data.message + ")");
        }
        else if (data.channel == "promise")
        {
            kromblast().get_window()->inject("kromblast.socket.promise(\"" + data.tmpId + "\", " + data.message + ")");
        }
    }
    else if (data.player == "_kromblast")
    {
        this->handle_kb_command(data.channel, data.message);
    }
    else if (data.player == "_dispatcher")
    {
        kromblast().get_logger()->log("SocketControl", "Received message from client on channel " + data.channel + ": " + data.message);
        kromblast().get_dispatcher()->dispatch(data.channel, data.message);
    }
}

void SocketControl::load_functions()
{
    kromblast().get_plugin()->claim_callback(
        "kromblast.socket.send",
        1,
        BIND_CALLBACK(SocketControl::send_to_socket),
        std::vector<std::regex>{std::regex("^.*$")});
    kromblast().get_plugin()->claim_callback(
        "kromblast.socket.promise",
        2,
        BIND_CALLBACK(SocketControl::promise),
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

void SocketControl::send_socket(const std::string &tmpId, const std::string &message)
{
    send_socket({tmpId, message});
}

void SocketControl::send_socket(const socket_send &data)
{
    std::string message = "\1response|" + data.tmpId + "\2" + data.message;
    ksocket->send_to_clients(message);
}

std::string SocketControl::promise(Kromblast::Core::kromblast_callback_called_t *parameters)
{
    socket_send data;
    data.tmpId = parameters->args.at(0);
    data.message = parameters->args.at(1);
    send_socket(data);
    return R"({"ok": true})";
}

std::string SocketControl::send_to_socket(Kromblast::Core::kromblast_callback_called_t *parameters)
{
    send_socket("0000", parameters->args.at(0));
    return R"({"ok": true})";
}