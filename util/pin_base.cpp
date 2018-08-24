////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "chip_base.hpp"
#include "pin_base.hpp"

#include <algorithm>

////////////////////////////////////////////////////////////////////////////////
namespace gpio
{

////////////////////////////////////////////////////////////////////////////////
pin_base::~pin_base() { }

pin_base::pin_base(gpio::chip* chip, gpio::pos n) noexcept :
    chip_(chip), pos_(n)
{ }

////////////////////////////////////////////////////////////////////////////////
namespace
{

template<typename Cont, typename T>
inline auto find(const Cont& cont, const T& value)
{
    using std::begin; using std::end;
    return std::count(begin(cont), end(cont), value);
}

}

inline bool pin_base::digital() const noexcept { return find(gpio::digital_modes, mode_); }
inline bool pin_base::analog() const noexcept { return find(gpio::analog_modes, mode_); }

inline bool pin_base::input() const noexcept { return find(gpio::input_modes, mode_); }
inline bool pin_base::output() const noexcept { return find(gpio::output_modes, mode_); }

////////////////////////////////////////////////////////////////////////////////
void pin_base::period(gpio::nsec period)
{
    using namespace std::chrono_literals;
    period_ = std::max(period, 1ns);
}

////////////////////////////////////////////////////////////////////////////////
void pin_base::pulse(gpio::nsec pulse)
{
    using namespace std::chrono_literals;
    pulse_ = std::min(std::max(pulse, 0ns), period_);
}

////////////////////////////////////////////////////////////////////////////////
void pin_base::duty_cycle(gpio::percent pc)
{
    pc = std::max(0.0, std::min(pc, 100.0));
    pulse(nsec( static_cast<nsec::rep>(period_.count() * pc / 100.0 + 0.5) ));
}

////////////////////////////////////////////////////////////////////////////////
gpio::percent pin_base::duty_cycle() const noexcept
{
    return 100.0 * pulse_.count() / period_.count();
}

////////////////////////////////////////////////////////////////////////////////
}
