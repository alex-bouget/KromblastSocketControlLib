#ifndef SOCKET_CONTROL_HPP
#define SOCKET_CONTROL_HPP

#include <string>
#include "kromblast_api.hpp"
#include "kromblast_lib_plugin.hpp"
#include "kromblast_lib_plugin_callback.hpp"
#include "kromblast_api_dispatcher.hpp"
#include "ksocket.hpp"
#include <thread>

class SocketControl : public Kromblast::Class::KromLib, public Kromblast::Api::SignalHandlerInterface
{
private:
    Kromblast::Api::KromblastInterface *kromblast;
    SocketRunner *ksocket;
    std::thread *socket_thread;

    void hand_socket(const std::string& message);

public:
    std::string get_version();

    void set_kromblast(void *kromblast);

    void load_functions();

    void handle(Kromblast::Api::Signal signal);
};

#endif