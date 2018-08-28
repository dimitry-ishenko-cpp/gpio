////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef GPIO_GPIOD_PIN_HPP
#define GPIO_GPIOD_PIN_HPP

////////////////////////////////////////////////////////////////////////////////
#include "pin_base.hpp"

#include <asio/posix/stream_descriptor.hpp>
#include <atomic>
#include <cstdint>
#include <future>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace gpio
{

////////////////////////////////////////////////////////////////////////////////
class gpiod_chip;

////////////////////////////////////////////////////////////////////////////////
class gpiod_pin : public pin_base
{
public:
    ////////////////////
    gpiod_pin(gpiod_chip*, gpio::pos);
    virtual ~gpiod_pin() override;

    ////////////////////
    virtual gpio::mode mode() const noexcept override
    { return pwm_started() ? gpio::pwm : mode_; }
    virtual void mode(gpio::mode, gpio::flag, gpio::state) override;

    virtual void detach() override;
    virtual bool detached() const noexcept override { return !fd_.is_open(); }

    ////////////////////
    using pin_base::set;

    virtual void set(gpio::state = gpio::on) override;
    virtual gpio::state state() override;

    virtual void period(gpio::nsec) override;
    virtual void set(gpio::nsec) override;

private:
    ////////////////////
    asio::posix::stream_descriptor fd_;

    void update();
    void mode_digital_in(std::uint32_t flags);
    void mode_digital_out(std::uint32_t flags, gpio::state);

    std::vector<char> buffer_;
    void sched_read();

    ////////////////////
    using ticks = gpio::nsec::rep;
    std::atomic<ticks> high_ticks_ { pulse_.count() };
    std::atomic<ticks> low_ticks_ { period_.count() - high_ticks_ };

    std::future<void> pwm_;
    std::atomic<bool> stop_ { false };

    void pwm_start();
    void pwm_stop();
    bool pwm_started() const noexcept { return pwm_.valid(); }
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
