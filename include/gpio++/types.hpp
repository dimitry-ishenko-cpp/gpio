////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef GPIO_TYPES_HPP
#define GPIO_TYPES_HPP

////////////////////////////////////////////////////////////////////////////////
#include <chrono>
#include <cstddef>
#include <set>

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

// pin modes
using modes = std::set<mode>;

constexpr auto digital_modes = { digital_in, digital_out, pwm };
constexpr auto analog_modes = { analog_in, analog_out };

constexpr auto input_modes = { digital_in, analog_in };
constexpr auto output_modes = { digital_out, analog_out, pwm };

////////////////////////////////////////////////////////////////////////////////
// pin mode flag
enum flag
{
    // digital_in, digital_out
    active_low,

    // digital_in
    pull_up,
    pull_down,

    // digital_out
    open_drain,
    open_source,

    // pwm
    soft_pwm,
    hard_pwm,
};

// pin mode flags
using flags = std::set<flag>;

////////////////////////////////////////////////////////////////////////////////
// pin value
using value = int;

// pwm period & pulse
using usec = std::chrono::microseconds;

// duty cycle
using pct = double;

constexpr inline
auto operator"" _pct(long double p) noexcept { return static_cast<pct>(p); }

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
