////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef GPIO_TYPES_HPP
#define GPIO_TYPES_HPP

////////////////////////////////////////////////////////////////////////////////
#include <atomic>
#include <chrono>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <map>
#include <type_traits>
#include <utility>

////////////////////////////////////////////////////////////////////////////////
namespace gpio
{

////////////////////////////////////////////////////////////////////////////////
// some common bits will be placed in the "literals" namespace,
// so that the user can pull them into their scope without
// pulling in the entire library
namespace literals { }
using namespace literals;

////////////////////////////////////////////////////////////////////////////////
// pin #
using pos = std::size_t;

////////////////////////////////////////////////////////////////////////////////
namespace literals
{

// invalid pin #
constexpr pos npos = static_cast<pos>(-1);

// pin mode
enum mode { detached, in, out };

// pin mode flag
enum flag
{
    // in, out
    active_low  = 0x0001,

    // in
    pull_up     = 0x0010,
    pull_down   = 0x0020,

    // out
    open_drain  = 0x0100,
    open_source = 0x0200,
};

inline constexpr
flag operator|(flag x, flag y) noexcept
{ return static_cast<flag>(static_cast<int>(x) | static_cast<int>(y)); }

inline constexpr
flag operator&(flag x, flag y) noexcept
{ return static_cast<flag>(static_cast<int>(x) & static_cast<int>(y)); }

inline constexpr
flag& operator|=(flag& x, flag y) noexcept { return x = x | y; }

inline constexpr
flag& operator&=(flag& x, flag y) noexcept { return x = x & y; }

inline constexpr
flag operator~(flag x) noexcept { return static_cast<flag>(~static_cast<int>(x)); }

// digital pin state
enum state : bool { on = true, off = false };

}

////////////////////////////////////////////////////////////////////////////////
// pwm pin period & pulse
using nsec = std::chrono::nanoseconds;
namespace literals { using namespace std::chrono_literals; }

// pwm pin duty cycle
using percent = double;

namespace literals
{

constexpr inline
auto operator""_pc(long double pc) noexcept
{ return static_cast<percent>(pc); }

constexpr inline
auto operator""_pc(unsigned long long pc) noexcept
{ return static_cast<percent>(pc); }

}

////////////////////////////////////////////////////////////////////////////////
// digital callback
using fn_state_changed = std::function<void(state)>;
using fn_state_on = std::function<void()>;
using fn_state_off = std::function<void()>;

////////////////////////////////////////////////////////////////////////////////
// call id
using cid = unsigned;

namespace literals
{

// invalid/no call id
constexpr cid ncid = static_cast<cid>(-1);

}

// callback chain
template<typename Fn>
struct call_chain
{
    ////////////////////
    call_chain() = default;

    call_chain(call_chain&&) = default;
    call_chain& operator=(call_chain&&) = default;

    ////////////////////
    template<typename T>
    cid add(T&& fn)
    {
        cid id = get_cid();
        chain_.emplace(id, std::forward<T>(fn));
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
    // get unique call id
    static cid get_cid()
    {
        static std::atomic<cid> seed { 0 };
        return seed++;
    }
    std::map<cid, Fn> chain_;
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
