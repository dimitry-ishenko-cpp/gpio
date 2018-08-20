////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef GPIO_PIN_HPP
#define GPIO_PIN_HPP

////////////////////////////////////////////////////////////////////////////////
#include <gpio/types.hpp>

#include <algorithm>
#include <set>
#include <string>
#include <utility>

////////////////////////////////////////////////////////////////////////////////
namespace gpio
{

////////////////////////////////////////////////////////////////////////////////
struct pin
{
    ////////////////////
    virtual ~pin() { }

    ////////////////////
    auto const& type() const noexcept { return type_; }
    auto pos() const noexcept { return pos_; }
    auto type_id() const { return type_ + ".pin:" + std::to_string(pos_); }

    auto const& name() const noexcept { return name_; }

    ////////////////////
    auto mode() const noexcept { return mode_; }
    virtual void mode(gpio::mode, gpio::flag flags, gpio::value);
    virtual void mode(gpio::mode, gpio::flag flags);
    virtual void mode(gpio::mode, gpio::value);
    virtual void mode(gpio::mode);

    virtual void detach() = 0;
    virtual bool detached() const noexcept = 0;

    bool digital() const noexcept;
    bool analog() const noexcept;

    bool input() const noexcept;
    bool output() const noexcept;

    auto const& modes() const noexcept { return modes_; }
    bool supports(gpio::mode) const noexcept;

    bool is(gpio::flag flag) const noexcept { return flags_ & flag; }

    bool used() const noexcept { return used_; }

    ////////////////////
    virtual void value(int) = 0;
    virtual int value() = 0;

protected:
    ////////////////////
    std::string type_;
    gpio::pos pos_;
    std::string name_;

    std::set<gpio::mode> modes_;

    gpio::mode mode_ = gpio::detached;
    gpio::flag flags_ = static_cast<gpio::flag>(0);

    bool used_ = false;

    ////////////////////
    pin(std::string type, gpio::pos n) noexcept : type_(std::move(type)), pos_(n) { }
};

////////////////////////////////////////////////////////////////////////////////
inline void pin::mode(gpio::mode mode, gpio::flag flags, gpio::value)
{ mode_ = mode; flags_ = flags; }

inline void pin::mode(gpio::mode mode, gpio::flag flags)
{ this->mode(mode, flags, 0); }

inline void pin::mode(gpio::mode mode, gpio::value value)
{ this->mode(mode, static_cast<gpio::flag>(0), value); }

inline void pin::mode(gpio::mode mode)
{ this->mode(mode, static_cast<gpio::flag>(0)); }

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

inline bool pin::digital() const noexcept { return find(gpio::digital_modes, mode_); }
inline bool pin::analog() const noexcept { return find(gpio::analog_modes, mode_); }

inline bool pin::input() const noexcept { return find(gpio::input_modes, mode_); }
inline bool pin::output() const noexcept { return find(gpio::output_modes, mode_); }

inline bool pin::supports(gpio::mode mode) const noexcept { return find(modes_, mode); }

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
