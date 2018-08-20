////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef GPIO_TYPES_HPP
#define GPIO_TYPES_HPP

////////////////////////////////////////////////////////////////////////////////
#include <cstddef>

////////////////////////////////////////////////////////////////////////////////
namespace gpio
{

////////////////////////////////////////////////////////////////////////////////
// pin #
using pos = std::size_t;

// invalid pin #
constexpr pos npos = static_cast<pos>(-1);

////////////////////////////////////////////////////////////////////////////////
// pin mode
enum mode
{
    detached,
    digital_in,
    digital_out,
    analog_in,
    analog_out,
    pwm,
};

constexpr mode digital_modes[] = { digital_in, digital_out, pwm };
constexpr mode analog_modes[] = { analog_in, analog_out };

constexpr mode input_modes[] = { digital_in, analog_in };
constexpr mode output_modes[] = { digital_out, analog_out, pwm };

////////////////////////////////////////////////////////////////////////////////
// pin mode flags
enum flag
{
    // digital_in, digital_out
    active_low = 0x0001,

    // digital_in
    pull_up = 0x0010,
    pull_down = 0x0020,

    // digital_out
    open_drain = 0x0100,
    open_source = 0x0200
};

inline constexpr flag operator|(flag f1, flag f2) noexcept
{ return static_cast<flag>(static_cast<int>(f1) | static_cast<int>(f2)); }

inline constexpr flag operator&(flag f1, flag f2) noexcept
{ return static_cast<flag>(static_cast<int>(f1) & static_cast<int>(f2)); }

////////////////////////////////////////////////////////////////////////////////
using value = int;

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
