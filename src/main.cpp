#include <string>
#include "kromblast_lib_plugin.hpp"
#include "socket_control.hpp"

extern "C" Kromblast::Class::KromLib *kromblast_lib_get_class()
{
    return (new SocketControl());
}