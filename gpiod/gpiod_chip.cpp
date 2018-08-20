////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "gpiod_chip.hpp"
#include <stdexcept>

////////////////////////////////////////////////////////////////////////////////
namespace gpio
{

////////////////////////////////////////////////////////////////////////////////
gpiod_chip::gpiod_chip(std::string param)
{
    constexpr auto npos = std::string::npos;

    // discard any extra parameters
    auto pos = param.find(':');
    if(pos != npos) param.erase(pos);

    if(param.find_first_not_of("0123456789") != npos
    || param.size() < 1 || param.size() > 3)
    throw std::invalid_argument("Missing or invalid gpiod chip id " + param);

    id_ = std::move(param);
}

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
extern "C" gpio::chip* create_chip(const char* param) { return new gpio::gpiod_chip(param); }
extern "C" void delete_chip(gpio::chip* chip) { if(chip) delete chip; }
