////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef GPIO_CHIP_BASE_HPP
#define GPIO_CHIP_BASE_HPP

////////////////////////////////////////////////////////////////////////////////
#include <gpio++/chip.hpp>
#include <gpio++/pin.hpp>
#include <gpio++/types.hpp>

#include <memory>
#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace gpio
{

////////////////////////////////////////////////////////////////////////////////
struct chip_base : public chip
{
    virtual ~chip_base() override;

    chip_base(const chip_base&) = delete;
    chip_base& operator=(const chip_base&) = delete;

    ////////////////////
    virtual const std::string& type() const noexcept override { return type_; }
    virtual const std::string& id() const noexcept override { return id_; }

    virtual const std::string& name() const noexcept override { return name_; }

    ////////////////////
    virtual std::size_t pin_count() const noexcept override { return pins_.size(); }

    virtual gpio::pin* pin(gpio::pos) override;
    virtual const gpio::pin* pin(gpio::pos) const override;

protected:
    ////////////////////
    std::string type_, id_;
    std::string name_;

    using pin_ptr = std::unique_ptr<gpio::pin>;
    std::vector<pin_ptr> pins_;

    ////////////////////
    chip_base(std::string type) noexcept;

    std::string type_id() const;
    void throw_range(gpio::pos) const;

    friend class pin_base;
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
