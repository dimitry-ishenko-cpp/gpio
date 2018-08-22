////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "chip_base.hpp"
#include <stdexcept>

////////////////////////////////////////////////////////////////////////////////
namespace gpio
{

////////////////////////////////////////////////////////////////////////////////
chip_base::~chip_base() { }

chip_base::chip_base(std::string type) noexcept :
    type_(std::move(type))
{ }

////////////////////////////////////////////////////////////////////////////////
gpio::pin* chip_base::pin(gpio::pos n)
{
    throw_range(n);
    return pins_[n].get();
}

const gpio::pin* chip_base::pin(gpio::pos n) const
{
    throw_range(n);
    return pins_[n].get();
}

////////////////////////////////////////////////////////////////////////////////
std::string chip_base::type_id() const
{
    return id().size() ? type() + ':' + id() : type();
}

////////////////////////////////////////////////////////////////////////////////
void chip_base::throw_range(gpio::pos n) const
{
    if(n >= pin_count()) throw std::out_of_range(
        type_id() + ": Invalid pin # " + std::to_string(n)
    );
}

////////////////////////////////////////////////////////////////////////////////
}
