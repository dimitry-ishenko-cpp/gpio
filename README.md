# C++ GPIO Library for Linux

The library is implemented as a "thin" layer provided by a static library called `libgpio++.a` and a number of backends provided by dynamic libraries, which can be loaded at runtime.

The default backend provided by `libgpio++-chip.so` uses [new GPIO API](https://github.com/torvalds/linux/blob/v4.8/include/uapi/linux/gpio.h) introduced in Linux v4.8. This backend should work on any platform running kernel v4.8 or later. The backend has a few limitations, such as lack of pull-up/pull-down resistor control and no hardware PWM.

Chip-specific backends can provide additional functionality supported by the given chip.

## Getting Started

### Prerequisites

* Linux kernel >= 4.8
* [asio C++ Library](https://think-async.com/) >= 1.10.10
* Linux headers >= 4.8
* CMake >= 3.1

NB: asio 1.12.1 has a bug that may cause SIGSEGV due to null pointer deference, when using callbacks, eg. `gpio::pin::on_state_changed()`. It is recommended that you install an unofficial version 1.10.10 from [here](https://github.com/dimitry-ishenko-cpp/asio/releases/tag/asio-1-10-10).

### Installation

Binary (Debian/Ubuntu/etc):
```console
$ version=1.0
$ arch=$(uname -p)
$ wget https://github.com/dimitry-ishenko-cpp/gpio/releases/download/v${version}/gpio++_${version}_Linux_${arch}.deb
$ sudo apt install ./gpio++_${version}_Linux_${arch}.deb
```

Compile from source:
```console
$ version=1.0
$ wget https://github.com/dimitry-ishenko-cpp/gpio/releases/download/v${version}/gpio++-${version}-Source.tar.bz2
$ tar xjf gpio++-${version}-Source.tar.bz2
$ mkdir gpio++-${version}-Source/build
$ cd gpio++-${version}-Source/build
$ cmake .. -DCMAKE_INSTALL_PREFIX=/usr
$ make
$ sudo make install
```

Clone and compile from repository:
```console
$ git clone https://github.com/dimitry-ishenko-cpp/gpio.git gpio++
$ mkdir gpio++/build
$ cd gpio++/build
$ cmake .. -DCMAKE_INSTALL_PREFIX=/usr
$ make
$ sudo make install
```

### Usage

Example 1:
```cpp
#include <gpio++/gpio.hpp>

#include <asio.hpp>
#include <iostream>

int main(int argc, char* argv[])
{
    asio::io_service io;

    auto chip = gpio::get_chip(io, "chip:0");
    std::cout << "Chip info:" << std::endl;
    std::cout << "  type: " << chip->type() << std::endl;
    std::cout << "    id: " << chip->id() << std::endl;
    std::cout << "  name: " << chip->name() << std::endl;
    std::cout << "  pins: " << chip->pin_count() << std::endl;
    std::cout << std::endl;

    for(std::size_t n = 0; n < chip->pin_count(); ++n)
    {
        auto pin = chip->pin(n);
        std::cout << "Pin #" << pin->pos() << " info:" << std::endl;
        std::cout << "  name: " << pin->name() << std::endl;
        std::cout << "  mode: " << pin->mode();

        std::cout << " [";
        if(pin->is_digital()) std::cout << " digital";
        if(pin->is_analog() ) std::cout << " analog";
        if(pin->is_input()  ) std::cout << " input";
        if(pin->is_output() ) std::cout << " output";
        std::cout << " ]" << std::endl;

        std::cout << std::endl;
    }

    return 0;
}
```

Compile and run:
```console
$ g++ example1.cpp -o example1 -DASIO_STANDALONE -lgpio++ -ldl
$ ./example1
```

Example 2:
```cpp
#include <gpio++/gpio.hpp>

#include <asio.hpp>
#include <chrono>
#include <iostream>
#include <thread>

using namespace std::chrono_literals;
using namespace gpio::literals;

int main()
{
    asio::io_service io;
    auto chip = gpio::get_chip(io, "chip:0");

    auto pin = chip->pin(2);
    pin->mode(gpio::pwm);
    pin->period(10ms);

    std::cout << "Fading pin up and down" << std::endl;
    for(int n = 0; n < 10; ++n)
    {
        for(auto pc = 0_pc; pc < 100_pc; pc += 1)
        {
            pin->duty_cycle(pc);
            std::this_thread::sleep_for(30ms);
        }

        for(auto time = 10ms; time > 0ms; time -= 1ms)
        {
            pin->pulse(time);
            std::this_thread::sleep_for(30ms);
        }
    }
    std::cout << "Done" << std::endl;

    return 0;
}
```

Compile and run:
```console
$ g++ example2.cpp -o example2 -DASIO_STANDALONE -lgpio++ -ldl
$ ./example2
```

Example 3 (callback):
```cpp
#include <gpio++/gpio.hpp>

#include <asio.hpp>
#include <iostream>

int main()
{
    asio::io_service io;
    auto chip = gpio::get_chip(io, "chip:0");

    auto pin = chip->pin(2);
    pin->mode(gpio::digital_in);

    std::cout << "Monitoring pin:" << std::endl;
    pin->on_state_changed([](gpio::state s)
    {
        std::cout << "State=" << s << std::endl;
    });

    io.run();
    return 0;
}
```

Compile and run:
```console
$ g++ example3.cpp -o example3 -DASIO_STANDALONE -lgpio++ -ldl
$ ./example3
```

## Authors

* **Dimitry Ishenko** - dimitry (dot) ishenko (at) (gee) mail (dot) com

## License

This project is distributed under the GNU GPL license. See the
[LICENSE.md](LICENSE.md) file for details.
