#ifndef SOCKET_CONTROL_HPP
#define SOCKET_CONTROL_HPP

#include <string>
#include "kromblast_api.hpp"
#include "kromblast_lib_plugin.hpp"
#include "kromblast_lib_plugin_callback.hpp"
#include "kromblast_api_dispatcher.hpp"
#include "ksocket.hpp"
#include <thread>
#include <memory>


#define SOCKET_DATA_ERROR_NO_HEADER 0b00010001
#define SOCKET_DATA_ERROR_NO_BODY 0b00010010

struct socket_data
{
    // \1 player|tmpId \2 channel:message
    u_int8_t error;
    std::string player;
    std::string tmpId;
    std::string channel;
    std::string message;
};

struct socket_send
{
    std::string tmpId;
    std::string message;
};

class SocketControl : public Kromblast::Class::KromLib, public Kromblast::Api::SignalHandlerInterface
{
private:
    std::unique_ptr<SocketRunner> ksocket;
    std::unique_ptr<std::thread> socket_thread;

    void hand_socket(const std::string& message);
    void handle_kb_command(const std::string& command, const std::string& message);

    socket_data get_socket_data(const std::string& message);

    void send_socket(const std::string& tmpId, const std::string& message);
    void send_socket(const socket_send& data);

public:
    std::string get_version() override;

    void at_start() override;

    void load_functions() override;

    void handle(Kromblast::Api::Signal signal);

    std::string promise(Kromblast::Core::kromblast_callback_called_t *parameters);
    std::string send_to_socket(Kromblast::Core::kromblast_callback_called_t *parameters);
};

#endif