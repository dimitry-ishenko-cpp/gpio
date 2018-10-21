////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef GPIO_PIN_BASE_HPP
#define GPIO_PIN_BASE_HPP

////////////////////////////////////////////////////////////////////////////////
#include <gpio++/pin.hpp>
#include <gpio++/types.hpp>

#include <set>
#include <string>

////////////////////////////////////////////////////////////////////////////////
namespace gpio
{

////////////////////////////////////////////////////////////////////////////////
class pin_base : public pin
{
public:
    ////////////////////
    pin_base(gpio::chip*, gpio::pos) noexcept;
    virtual ~pin_base() override;

    pin_base(const pin_base&) = delete;
    pin_base& operator=(const pin_base&) = delete;

    ////////////////////
    virtual const gpio::chip* chip() const noexcept override { return chip_; }

    virtual gpio::pos pos() const noexcept override { return pos_; }
    virtual const std::string& name() const noexcept override { return name_; }

    ////////////////////
    virtual void mode(gpio::mode mode, gpio::flag flags, gpio::state) override
    { mode_ = mode; flags_ = flags; }
    virtual void mode(gpio::mode mode, gpio::flag flags) override
    { this->mode(mode, flags, off); }
    virtual void mode(gpio::mode mode, gpio::state value) override
    { this->mode(mode, gpio::flag { }, value); }
    virtual void mode(gpio::mode mode) override
    { this->mode(mode, gpio::flag { }); }
    virtual gpio::mode mode() const noexcept override { return mode_; }

    virtual bool is(gpio::flag flag) const noexcept override { return flags_ & flag; }

    virtual bool supports(gpio::mode mode) const noexcept override
    { return valid_modes_.count(mode); }
    virtual bool supports(gpio::flag flag) const noexcept override
    { return valid_flags_.count(flag); }

    ////////////////////
    virtual void detach() override { }
    virtual bool is_detached() const noexcept override { return true; }

    virtual bool is_used() const noexcept override { return used_; }

    ////////////////////
    // digital
    virtual void set(gpio::state = on) override;
    virtual void reset() override { set(off); }
    virtual gpio::state state() override;

    // pwm
    virtual void period(nsec) override;
    virtual nsec period() const noexcept override { return period_; }

    virtual void pulse(nsec) override;
    virtual nsec pulse() const noexcept override { return pulse_; }

    virtual void duty_cycle(percent) override;
    virtual percent duty_cycle() const noexcept override;

    ////////////////////
    // digital callback
    virtual cid on_state_changed(fn_state_changed) override;
    virtual cid on_state_on(fn_state_on) override;
    virtual cid on_state_off(fn_state_off) override;

    virtual bool remove(cid) override;

protected:
    ////////////////////
    gpio::chip* chip_ = nullptr;

    gpio::pos pos_;
    std::string name_;

    gpio::mode mode_ = detached;
    gpio::flag flags_ { };
    bool used_ = false;

    std::set<gpio::mode> valid_modes_;
    std::set<gpio::flag> valid_flags_;

    nsec period_ = 10ms, pulse_ = 0ns;

    call_chain<fn_state_changed> state_changed_;
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
