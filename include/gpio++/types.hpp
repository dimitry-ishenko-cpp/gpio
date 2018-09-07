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
#include <functional>
#include <initializer_list>
#include <tuple>

////////////////////////////////////////////////////////////////////////////////
namespace gpio
{

////////////////////////////////////////////////////////////////////////////////
namespace literals { }
using namespace literals;

////////////////////////////////////////////////////////////////////////////////
// pin #
using pos = std::size_t;

// invalid pin #
constexpr pos npos = static_cast<pos>(-1);

////////////////////////////////////////////////////////////////////////////////
namespace literals
{

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

constexpr auto digital_modes = { digital_in, digital_out, pwm };
constexpr auto analog_modes = { analog_in, analog_out };

constexpr auto input_modes = { digital_in, analog_in };
constexpr auto output_modes = { digital_out, analog_out, pwm };

}

////////////////////////////////////////////////////////////////////////////////
namespace literals
{

// pin mode flag
enum flag
{
    // digital_in, digital_out
    active_low  = 0x0001,

    // digital_in
    pull_up     = 0x0010,
    pull_down   = 0x0020,

    // digital_out
    open_drain  = 0x0100,
    open_source = 0x0200,
};

inline constexpr
flag operator|(flag x, flag y) noexcept
{ return static_cast<flag>(static_cast<int>(x) | static_cast<int>(y)); }

inline constexpr
flag operator&(flag x, flag y) noexcept
{ return static_cast<flag>(static_cast<int>(x) & static_cast<int>(y)); }

inline constexpr
flag& operator|=(flag& x, flag y) noexcept { return x = x | y; }

inline constexpr
flag& operator&=(flag& x, flag y) noexcept { return x = x & y; }

inline constexpr
flag operator~(flag x) noexcept { return static_cast<flag>(~static_cast<int>(x)); }

}

////////////////////////////////////////////////////////////////////////////////
namespace literals
{

// digital pin state
enum state : bool { on = true, off = false };

}

// pwm pin period & pulse
using nsec = std::chrono::nanoseconds;

// pwm pin duty cycle
using percent = double;

namespace literals
{

constexpr inline
auto operator""_pc(long double pc) noexcept
{ return static_cast<percent>(pc); }

constexpr inline
auto operator""_pc(unsigned long long pc) noexcept
{ return static_cast<percent>(pc); }

}

////////////////////////////////////////////////////////////////////////////////
// analog value
using value = int;

////////////////////////////////////////////////////////////////////////////////
// call id
using cid = std::tuple<int, int>;

inline cid& operator++(cid& id) { ++std::get<1>(id); return id; }
inline cid operator++(cid& id, int) { cid prev = id; ++std::get<1>(id); return prev; }

// digital callback
using state_changed = std::function<void(state)>;
using state_on = std::function<void()>;
using state_off = std::function<void()>;

// analog callback
using value_changed = std::function<void(value)>;

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
