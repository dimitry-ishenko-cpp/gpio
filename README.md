# C++ GPIO Library for Linux

The library is implemented as a collection of backends to control GPIO pins.

The default backend provided by `libgpio++.so` uses [new GPIO API](https://github.com/torvalds/linux/blob/v4.8/include/uapi/linux/gpio.h) introduced in Linux v4.8. This backend should work on any platform running kernel v4.8 or later. The backend has a few limitations, such as lack of pull-up/pull-down resistor control and no hardware PWM.

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
$ version=3.0
$ arch=$(uname -p)
$ wget https://github.com/dimitry-ishenko-cpp/gpio/releases/download/v${version}/gpio++_${version}_Linux_${arch}.deb
$ sudo apt install ./gpio++_${version}_Linux_${arch}.deb
```

Compile from source:
```console
$ version=3.0
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
#include <gpio++.hpp>

#include <asio.hpp>
#include <iomanip>
#include <iostream>

int main(int argc, char* argv[])
{
    using std::cout;
    using std::endl;
    using std::setw;
    using namespace gpio::literals;

    asio::io_service io;

    auto chip = gpio::get_chip(io, "0");
    cout << "Chip info:" << endl;
    cout << "  type: " << chip->type() << endl;
    cout << "    id: " << chip->id() << endl;
    cout << "  name: " << chip->name() << endl;
    cout << "  pins: " << chip->pin_count() << endl;

    cout << endl;
    cout << " pin | name       | mode     " << endl;
    cout << "-----+------------+----------" << endl;
    for(std::size_t n = 0; n < chip->pin_count(); ++n)
    {
        auto pin = chip->pin(n);

        auto name = pin->name();
        if(name.empty()) name = "-";
        name.resize(10, ' ');

        std::string mode;
        switch(pin->mode())
        {
        case in:  mode = "in";  break;
        case out: mode = "out"; break;
        default:  mode = "-";   break;
        }

        cout << setw(4) << pin->pos() << " | " << name << " | " << mode << endl;
    }

    return 0;
}
```

Compile and run:
```console
$ g++ example1.cpp -o example1 -DASIO_STANDALONE -lgpio++ -pthread
$ ./example1
```

Example 2:
```cpp
#include <gpio++.hpp>

#include <asio.hpp>
#include <chrono>
#include <iostream>
#include <thread>

int main()
{
    using namespace gpio::literals;

    asio::io_service io;
    auto chip = gpio::get_chip(io, "0");

    auto pin = chip->pin(2)->as(gpio::out);
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
$ g++ example2.cpp -o example2 -DASIO_STANDALONE -lgpio++ -pthread
$ ./example2
```

Example 3 (callback):
```cpp
#include <gpio++.hpp>

#include <asio.hpp>
#include <iostream>

int main()
{
    asio::io_service io;
    auto chip = gpio::get_chip(io, "0");

    auto pin = chip->pin(2)->as(gpio::in);

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
$ g++ example3.cpp -o example3 -DASIO_STANDALONE -lgpio++ -pthread
$ ./example3
```

## Authors

* **Dimitry Ishenko** - dimitry (dot) ishenko (at) (gee) mail (dot) com

## License

This project is distributed under the GNU GPL license. See the
[LICENSE.md](LICENSE.md) file for details.
