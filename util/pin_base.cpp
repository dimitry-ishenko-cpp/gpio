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

inline bool pin_base::is_digital() const noexcept { return find(gpio::digital_modes, mode_); }
inline bool pin_base::is_analog() const noexcept { return find(gpio::analog_modes, mode_); }

inline bool pin_base::is_input() const noexcept { return find(gpio::input_modes, mode_); }
inline bool pin_base::is_output() const noexcept { return find(gpio::output_modes, mode_); }

////////////////////////////////////////////////////////////////////////////////
void pin_base::period(gpio::nsec period)
{
    using namespace std::chrono_literals;
    period_ = std::max(period, 1ns);
}

////////////////////////////////////////////////////////////////////////////////
void pin_base::set(gpio::nsec pulse)
{
    using namespace std::chrono_literals;
    pulse_ = std::min(std::max(pulse, 0ns), period_);
}

////////////////////////////////////////////////////////////////////////////////
void pin_base::set(gpio::percent pc)
{
    pc = std::max(0.0, std::min(pc, 100.0));
    set(nsec( static_cast<nsec::rep>(period_.count() * pc / 100.0 + 0.5) ));
}

////////////////////////////////////////////////////////////////////////////////
gpio::percent pin_base::duty_cycle() const noexcept
{
    return 100.0 * pulse_.count() / period_.count();
}

////////////////////////////////////////////////////////////////////////////////
void pin_base::on_state_changed(state_changed fn)
{
    state_changed_ = std::move(fn);
}

////////////////////////////////////////////////////////////////////////////////
void pin_base::on_state_on(state_on fn)
{
    on_state_changed([fn_ = std::move(fn)](gpio::state state)
        { if(state == gpio::on) fn_(); }
    );
}

////////////////////////////////////////////////////////////////////////////////
void pin_base::on_state_off(state_off fn)
{
    on_state_changed([fn_ = std::move(fn)](gpio::state state)
        { if(state == gpio::off) fn_(); }
    );
}

////////////////////////////////////////////////////////////////////////////////
}
