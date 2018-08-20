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
gpiod_chip::gpiod_chip(const char* param) : chip(param)
{
    constexpr auto npos = std::string::npos;

    // discard any extra parameters
    auto pos = id_.find(':');
    if(pos != npos) id_.erase(pos);

    if(id_.find_first_not_of("0123456789") != npos
    || id_.size() < 1 || id_.size() > 3)
    throw std::invalid_argument("Missing or invalid gpiod chip id " + id_);
}

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
extern "C" gpio::chip* create_chip(const char* param) { return new gpio::gpiod_chip(param); }
extern "C" void delete_chip(gpio::chip* chip) { if(chip) delete chip; }
