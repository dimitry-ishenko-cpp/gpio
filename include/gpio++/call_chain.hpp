////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef GPIO_CALL_CHAIN_HPP
#define GPIO_CALL_CHAIN_HPP

////////////////////////////////////////////////////////////////////////////////
#include <gpio++/types.hpp>

#include <atomic>
#include <map>
#include <type_traits>
#include <utility>

////////////////////////////////////////////////////////////////////////////////
namespace gpio
{

////////////////////////////////////////////////////////////////////////////////
template<typename Fn>
struct call_chain
{
    ////////////////////
    call_chain() = default;

    call_chain(call_chain&&) = default;
    call_chain& operator=(call_chain&&) = default;

    ////////////////////
    cid add(Fn fn)
    {
        cid id = get_cid();
        chain_.emplace(id, std::move(fn));
        return id;
    }
    bool remove(cid id) { return chain_.erase(id); }

    template<typename... Args>
    void operator()(Args&&... args)
    {
        for(const auto& fn : chain_) fn.second(std::forward<Args>(args)...);
    }

private:
    ////////////////////
    static cid get_cid()
    {
        std::atomic<std::remove_const_t<cid>> seed { 0 };
        return seed++;
    }
    std::map<cid, Fn> chain_;
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
