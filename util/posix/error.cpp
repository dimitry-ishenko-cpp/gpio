////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017-2018 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "posix/error.hpp"
#include <cerrno>
#include <cstring>

////////////////////////////////////////////////////////////////////////////////
namespace posix
{

////////////////////////////////////////////////////////////////////////////////
const char* category::name() const noexcept { return "posix"; }

////////////////////////////////////////////////////////////////////////////////
#define CASE(value) case static_cast<int>(errc::value): return std::errc::value

std::error_condition category::default_error_condition(int ev) const noexcept
{
    switch(ev)
    {
    CASE(address_family_not_supported      );
    CASE(address_in_use                    );
    CASE(address_not_available             );
    CASE(already_connected                 );
    CASE(argument_list_too_long            );
    CASE(argument_out_of_domain            );
    CASE(bad_address                       );
    CASE(bad_file_descriptor               );
    CASE(bad_message                       );
    CASE(broken_pipe                       );
    CASE(connection_aborted                );
    CASE(connection_already_in_progress    );
    CASE(connection_refused                );
    CASE(connection_reset                  );
    CASE(cross_device_link                 );
    CASE(destination_address_required      );
    CASE(device_or_resource_busy           );
    CASE(directory_not_empty               );
    CASE(executable_format_error           );
    CASE(file_exists                       );
    CASE(file_too_large                    );
    CASE(filename_too_long                 );
    CASE(function_not_supported            );
    CASE(host_unreachable                  );
    CASE(identifier_removed                );
    CASE(illegal_byte_sequence             );
    CASE(inappropriate_io_control_operation);
    CASE(interrupted                       );
    CASE(invalid_argument                  );
    CASE(invalid_seek                      );
    CASE(io_error                          );
    CASE(is_a_directory                    );
    CASE(message_size                      );
    CASE(network_down                      );
    CASE(network_reset                     );
    CASE(network_unreachable               );
    CASE(no_buffer_space                   );
    CASE(no_child_process                  );
    CASE(no_link                           );
    CASE(no_lock_available                 );
    CASE(no_message_available              );
    CASE(no_message                        );
    CASE(no_protocol_option                );
    CASE(no_space_on_device                );
    CASE(no_stream_resources               );
    CASE(no_such_device_or_address         );
    CASE(no_such_device                    );
    CASE(no_such_file_or_directory         );
    CASE(no_such_process                   );
    CASE(not_a_directory                   );
    CASE(not_a_socket                      );
    CASE(not_a_stream                      );
    CASE(not_connected                     );
    CASE(not_enough_memory                 );
#if ENOTSUP != EOPNOTSUPP
    CASE(not_supported                     );
#endif
    CASE(operation_canceled                );
    CASE(operation_in_progress             );
    CASE(operation_not_permitted           );
    CASE(operation_not_supported           );
#if EWOULDBLOCK != EAGAIN
    CASE(operation_would_block             );
#endif
    CASE(owner_dead                        );
    CASE(permission_denied                 );
    CASE(protocol_error                    );
    CASE(protocol_not_supported            );
    CASE(read_only_file_system             );
    CASE(resource_deadlock_would_occur     );
    CASE(resource_unavailable_try_again    );
    CASE(result_out_of_range               );
    CASE(state_not_recoverable             );
    CASE(stream_timeout                    );
    CASE(text_file_busy                    );
    CASE(timed_out                         );
    CASE(too_many_files_open_in_system     );
    CASE(too_many_files_open               );
    CASE(too_many_links                    );
    CASE(too_many_symbolic_link_levels     );
    CASE(value_too_large                   );
    CASE(wrong_protocol_type               );

    default: return std::error_condition(ev, *this);
    }
}

////////////////////////////////////////////////////////////////////////////////
std::string category::message(int ev) const { return strerror(ev); }

////////////////////////////////////////////////////////////////////////////////
const std::error_category& category()
{
    static class category instance;
    return instance;
}

////////////////////////////////////////////////////////////////////////////////
errno_error::errno_error() :
    std::system_error(static_cast<errc>(errno))
{ }

errno_error::errno_error(const char* msg) :
    std::system_error(static_cast<errc>(errno), msg)
{ }

errno_error::errno_error(const std::string& msg) :
    std::system_error(static_cast<errc>(errno), msg)
{ }

////////////////////////////////////////////////////////////////////////////////
}
