////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef GPIO_GENERIC_PIN_HPP
#define GPIO_GENERIC_PIN_HPP

////////////////////////////////////////////////////////////////////////////////
#include "pin_base.hpp"

#include <asio/io_service.hpp>
#include <asio/posix/stream_descriptor.hpp>
#include <atomic>
#include <cstdint>
#include <future>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace gpio
{
namespace generic
{

////////////////////////////////////////////////////////////////////////////////
class chip;

////////////////////////////////////////////////////////////////////////////////
class pin : public pin_base
{
public:
    ////////////////////
    pin(asio::io_service&, generic::chip*, gpio::pos);
    virtual ~pin() override;

    ////////////////////
    virtual void mode(gpio::mode, gpio::flag, gpio::state) override;
    virtual gpio::mode mode() const noexcept override
    { return pwm_started() ? pwm : mode_; }

    virtual void detach() override;
    virtual bool is_detached() const noexcept override { return !fd_.is_open(); }

    ////////////////////
    using pin_base::set;

    virtual void set(gpio::state = on) override;
    virtual gpio::state state() override;

    virtual void period(nsec) override;
    virtual void set(nsec) override;

private:
    ////////////////////
    asio::posix::stream_descriptor fd_;

    void update();
    void mode_digital_in(std::uint32_t flags);
    void mode_digital_out(std::uint32_t flags, gpio::state);

    std::vector<char> buffer_;
    void sched_read();

    ////////////////////
    using ticks = nsec::rep;
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
}

////////////////////////////////////////////////////////////////////////////////
#endif
