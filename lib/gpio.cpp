////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "gpio++/gpio.hpp"

#include <stdexcept>
#include <utility>
#include <dlfcn.h>

////////////////////////////////////////////////////////////////////////////////
namespace gpio
{

void chip_deleter::operator()(gpio::chip* chip)
{
    if(handle)
    {
        auto delete_chip = reinterpret_cast<decltype (::delete_chip)*>(
            ::dlsym(handle, "delete_chip")
        );
        if(delete_chip) delete_chip(chip);

        ::dlclose(handle);
        handle = nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////
unique_chip get_chip(asio::io_context& io, std::string type)
{
    std::string param;
    auto pos = type.find(':');
    if(pos != std::string::npos)
    {
        param = type.substr(pos + 1);
        type.erase(pos);
    }
    auto lib = "libgpio++-" + type + ".so";

    auto handle = ::dlopen(lib.data(), RTLD_LAZY);
    if(!handle) throw std::invalid_argument(::dlerror());

    ::dlerror();
    auto create_chip = reinterpret_cast<decltype (::create_chip)*>(
        ::dlsym(handle, "create_chip")
    );
    if(!create_chip) throw std::invalid_argument(::dlerror());

    auto chip = unique_chip(create_chip(io, std::move(param)), { handle });
    if(!chip) throw std::invalid_argument(
        lib + ": create_chip() returned nullptr"
    );
    return chip;
}

////////////////////////////////////////////////////////////////////////////////
}
