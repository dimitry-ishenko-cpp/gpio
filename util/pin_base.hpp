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

#include <set>
#include <string>

////////////////////////////////////////////////////////////////////////////////
namespace gpio
{

////////////////////////////////////////////////////////////////////////////////
struct pin_base : public pin
{
    virtual ~pin_base() override;

    pin_base(const pin_base&) = delete;
    pin_base& operator=(const pin_base&) = delete;

    ////////////////////
    virtual const gpio::chip* chip() const noexcept override { return chip_; }

    virtual gpio::pos pos() const noexcept override { return pos_; }
    virtual const std::string& name() const noexcept override { return name_; }

    ////////////////////
    virtual gpio::mode mode() const noexcept override { return mode_; }
    virtual void mode(gpio::mode mode, gpio::flag flags, gpio::state) override
    { mode_ = mode; flags_ = flags; }
    virtual void mode(gpio::mode mode, gpio::flag flags) override
    { this->mode(mode, flags, 0); }
    virtual void mode(gpio::mode mode, gpio::state value) override
    { this->mode(mode, gpio::flag { }, value); }
    virtual void mode(gpio::mode mode) override
    { this->mode(mode, gpio::flag { }); }

    virtual bool digital() const noexcept override;
    virtual bool analog() const noexcept override;

    virtual bool input() const noexcept override;
    virtual bool output() const noexcept override;

    virtual bool supports(gpio::mode mode) const noexcept override
    { return modes_.count(mode); }

    virtual bool is(gpio::flag flag) const noexcept override
    { return flags_ & flag; }
    virtual bool supports(gpio::flag flag) const noexcept override
    { return valid_ & flag; }

    ////////////////////
    // virtual void detach() = 0;
    // virtual bool detached() const noexcept = 0;

    virtual bool used() const noexcept override { return used_; }

    ////////////////////
    // virtual void set(gpio::state = gpio::on) = 0;
    virtual void reset() override { set(gpio::off); }
    // virtual gpio::state state() = 0;

    // pwm
    virtual void period(gpio::nsec) override;
    virtual gpio::nsec period() const noexcept override { return period_; }

    virtual void pulse(gpio::nsec) override;
    virtual gpio::nsec pulse() const noexcept override { return pulse_; }

    virtual void duty_cycle(gpio::percent) override;
    virtual gpio::percent duty_cycle() const noexcept override;

protected:
    ////////////////////
    gpio::chip* chip_ = nullptr;

    gpio::pos pos_;
    std::string name_;

    gpio::mode mode_ = gpio::detached;
    std::set<gpio::mode> modes_;
    gpio::flag flags_ { }, valid_ { };
    bool used_ = false;

    gpio::nsec period_ { 100000000 }, pulse_ { 0 };

    ////////////////////
    pin_base(gpio::chip*, gpio::pos) noexcept;
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
