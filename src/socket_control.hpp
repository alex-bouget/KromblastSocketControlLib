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

class SocketControl : public Kromblast::Class::KromLib, public Kromblast::Api::SignalHandlerInterface
{
private:
    std::unique_ptr<SocketRunner> ksocket;
    std::unique_ptr<std::thread> socket_thread;

    void hand_socket(const std::string& message);
    void handle_kb_command(const std::string& command, const std::string& message);

public:
    std::string get_version() override;

    void at_start() override;

    void load_functions() override;

    void handle(Kromblast::Api::Signal signal);

    std::string send_to_socket(Kromblast::Core::kromblast_callback_called_t *parameters);
};

#endif