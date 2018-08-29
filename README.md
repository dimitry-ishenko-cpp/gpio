# C++ GPIO Library for Linux

The library is implemented as a "thin" layer provided by a static library called `libgpio++.a` and a number of backends provided by dynamic libraries, which can be loaded at runtime.

The default backend provided by `libgpio++-gpiod.so` uses [new GPIO API](https://github.com/torvalds/linux/blob/v4.8/include/uapi/linux/gpio.h) introduced in the Linux v4.8. This backend should work on any platform running kernel v4.8 or later. The backend has a few limitations, such as lack of pull-up/pull-down resistor control and no hardware PWM.

Chip-specific backends can provide additional functionality supported by the given chip.

## Getting Started

### Prerequisites

* Linux kernel >= 4.8
* [asio C++ Library](https://think-async.com/) >= 1.12.1
* Linux headers >= 4.8
* CMake >= 3.1

NB: asio 1.12.1 has a bug that may cause SIGSEGV due to null pointer deference, when using callbacks, eg. `gpio::pin::on_state_changed()`. It is recommended that you install patched version 1.12.1 from [here](https://github.com/dimitry-ishenko-cpp/asio/releases/tag/asio-1-12-1). Alternatively, you can apply patch [a3afaec](https://github.com/dimitry-ishenko-cpp/asio/commit/a3afaecc1ef6e2f2a72af18132c1b509cd3ebe5b) directly to your existing asio installation.

### Installation

Binary (Debian/Ubuntu/etc):
```console
$ version=3.1
$ arch=$(uname -p)
$ wget https://github.com/dimitry-ishenko-cpp/gpio/releases/download/v${version}/gpio++_${version}_Linux_${arch}.deb
$ sudo apt install ./gpio++_${version}_Linux_${arch}.deb
```

Compile from source:
```console
$ version=3.1
$ wget https://github.com/dimitry-ishenko-cpp/gpio/releases/download/v${version}/gpio++-${version}-Source.zip
$ unzip gpio++-${version}-Source.zip
$ mkdir gpio++-${version}-Source/build
$ cd gpio++-${version}-Source/build
$ cmake ..
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

Example:
```cpp
#include <gpio++/gpio.hpp>

#include <asio.hpp>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>

using namespace std::chrono_literals;
using namespace gpio::literals;

int main(int argc, char* argv[])
try
{
    if(argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <chip>" << std::endl;
        throw std::invalid_argument("Missing <chip> argument");
    }

    asio::io_service io;

    auto chip = gpio::get_chip(io, argv[1]);
    std::cout << "Chip info:" << std::endl;
    std::cout << "    type: " << chip->type() << std::endl;
    std::cout << "      id: " << chip->id() << std::endl;
    std::cout << "    name: " << chip->name() << std::endl;
    std::cout << "    pins: " << chip->pin_count() << std::endl;
    std::cout << std::endl;

    auto pin = chip->pin(2);
    pin->mode(gpio::pwm);
    pin->period(10ms);

    std::cout << "Pin info:" << std::endl;
    std::cout << "     pos: " << pin->pos() << std::endl;
    std::cout << "    name: " << pin->name() << std::endl;
    std::cout << "    mode: " << pin->mode() << std::endl;
    std::cout << "  period: " << pin->period().count() << "ns" << std::endl;
    std::cout << std::endl;

    std::cout << "Fading pin up and down" << std::endl;
    for(int n = 0; n < 10; ++n)
    {
        for(auto pc = 0_pc; pc < 100_pc; pc += 1)
        {
            pin->set(pc);
            std::this_thread::sleep_for(30ms);
        }

        for(auto time = 10ms; time > 0ms; time -= 1ms)
        {
            pin->set(time);
            std::this_thread::sleep_for(30ms);
        }
    }
    std::cout << "Done" << std::endl;

    return 0;
}
catch(std::exception& e)
{
    std::cerr << e.what() << std::endl;
    return 1;
}
```

Compile and run:
```console
$ g++ example.cpp -o example -DASIO_STANDALONE -lgpio++ -ldl
$ ./example gpiod:0
```

## Authors

* **Dimitry Ishenko** - dimitry (dot) ishenko (at) (gee) mail (dot) com

## License

This project is distributed under the GNU GPL license. See the
[LICENSE.md](LICENSE.md) file for details.
