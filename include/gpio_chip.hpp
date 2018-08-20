////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef GPIO_CHIP_HPP
#define GPIO_CHIP_HPP

////////////////////////////////////////////////////////////////////////////////
#include <gpio_pin.hpp>
#include <gpio_types.hpp>

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace gpio
{

////////////////////////////////////////////////////////////////////////////////
struct chip
{
    ////////////////////
    auto const& id() const { return id_; }
    auto const& name() const { return name_; }

    ////////////////////
    auto pin_count() const noexcept { return pins_.size(); }

    gpio::pin& pin(gpio::pos n)
    {
        throw_range(n);
        return *pins_[n];
    }
    gpio::pin const& pin(gpio::pos n) const
    {
        throw_range(n);
        return *pins_[n];
    }

protected:
    ////////////////////
    std::string id_, name_;

    using pin_ptr = std::unique_ptr<gpio::pin>;
    std::vector<pin_ptr> pins_;

    ////////////////////
    chip() noexcept = default;

    void throw_range(gpio::pos n) const
    {
        if(n >= pin_count()) throw std::out_of_range(
            "Invalid pin # " + std::to_string(n)
        );
    }
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
extern "C" gpio::chip* create_chip(const char* param);
extern "C" void delete_chip(gpio::chip*);

////////////////////////////////////////////////////////////////////////////////
#endif