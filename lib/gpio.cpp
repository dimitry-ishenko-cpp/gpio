////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "gpio.hpp"

#include <stdexcept>
#include <dlfcn.h>

////////////////////////////////////////////////////////////////////////////////
namespace gpio
{

void chip_deleter::operator()(gpio::chip* chip)
{
    auto delete_chip = reinterpret_cast<decltype (::delete_chip)*>(
        dlsym(handle.get(), "delete_chip")
    );
    if(delete_chip) delete_chip(chip);
}

////////////////////////////////////////////////////////////////////////////////
unique_chip create_chip(std::string name)
{
    std::string param;
    auto pos = name.find(':');
    if(pos != std::string::npos)
    {
        param = name.substr(pos + 1);
        name.erase(pos);
    }
    name = "libgpio-" + name + ".so";

    auto handle = shared_lib(dlopen(name.data(), RTLD_LAZY), dlclose);
    if(!handle) throw std::invalid_argument(dlerror());

    dlerror();
    auto create_chip = reinterpret_cast<decltype (::create_chip)*>(
        dlsym(handle.get(), "create_chip")
    );
    if(!create_chip) throw std::invalid_argument(dlerror());

    auto chip = unique_chip(create_chip(param.data()), { handle });
    if(!chip) throw std::invalid_argument(
        name + ": create_chip returned nullptr"
    );
    return chip;
}

////////////////////////////////////////////////////////////////////////////////
}
