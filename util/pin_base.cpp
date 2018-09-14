////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "chip_base.hpp"
#include "pin_base.hpp"

#include <algorithm>
#include <utility>

////////////////////////////////////////////////////////////////////////////////
namespace gpio
{

////////////////////////////////////////////////////////////////////////////////
pin_base::pin_base(gpio::chip* chip, gpio::pos n) noexcept :
    chip_(chip), pos_(n)
{ }

////////////////////////////////////////////////////////////////////////////////
pin_base::~pin_base() { }

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

inline bool pin_base::is_digital() const noexcept { return find(digital_modes, mode_); }
inline bool pin_base::is_analog() const noexcept { return find(analog_modes, mode_); }

inline bool pin_base::is_input() const noexcept { return find(input_modes, mode_); }
inline bool pin_base::is_output() const noexcept { return find(output_modes, mode_); }

////////////////////////////////////////////////////////////////////////////////
void pin_base::period(nsec period)
{
    period_ = std::max(period, 1ns);
}

////////////////////////////////////////////////////////////////////////////////
void pin_base::pulse(nsec pulse)
{
    pulse_ = std::min(std::max(pulse, 0ns), period_);
}

////////////////////////////////////////////////////////////////////////////////
void pin_base::duty_cycle(percent pc)
{
    pc = std::max(0.0, std::min(pc, 100.0));
    pulse(nsec( static_cast<nsec::rep>(period_.count() * pc / 100.0 + 0.5) ));
}

////////////////////////////////////////////////////////////////////////////////
percent pin_base::duty_cycle() const noexcept
{
    return 100.0 * pulse_.count() / period_.count();
}

////////////////////////////////////////////////////////////////////////////////
void pin_base::value(gpio::value n) { pulse(nsec(n)); }

////////////////////////////////////////////////////////////////////////////////
cid pin_base::on_state_changed(fn_state_changed fn)
{
    return state_changed_.add(std::move(fn));
}

////////////////////////////////////////////////////////////////////////////////
cid pin_base::on_state_on(fn_state_on fn)
{
    return on_state_changed(
        [fn_ = std::move(fn)](gpio::state state)
        { if(state == on) fn_(); }
    );
}

////////////////////////////////////////////////////////////////////////////////
cid pin_base::on_state_off(fn_state_off fn)
{
    return on_state_changed(
        [fn_ = std::move(fn)](gpio::state state)
        { if(state == off) fn_(); }
    );
}

////////////////////////////////////////////////////////////////////////////////
cid pin_base::on_value_changed(fn_value_changed fn)
{
    return value_changed_.add(std::move(fn));
}

////////////////////////////////////////////////////////////////////////////////
bool pin_base::remove(cid id)
{
    return state_changed_.remove(id)
        || value_changed_.remove(id);
}

////////////////////////////////////////////////////////////////////////////////
}
