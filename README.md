# C++ GPIO Library for Linux

The library is implemented as a "thin" layer provided by a static library called `libgpio++.a` and a number of backends provided by dynamic libraries, which can be loaded at runtime.

The default backend provided by `libgpio++-gpiod.so` uses [new GPIO API](https://github.com/torvalds/linux/blob/v4.8/include/uapi/linux/gpio.h) introduced in the Linux v4.8. This backend should work on any platform running kernel v4.8 or later. The backend has a few limitations, such as lack of pull-up/pull-down resistor control and no hardware PWM.

Chip-specific backends can provide additional functionality supported by the given chip.

## Getting Started

### Prerequisites

* Linux kernel >= 4.8
* Linux headers >= 4.8
* CMake >= 3.1

### Installation

Binary (Debian/Ubuntu/etc):
```console
$ version=1.1
$ arch=$(uname -p)
$ wget https://github.com/dimitry-ishenko/gpioxx/releases/download/v${version}/gpio++_${version}_Linux_${arch}.deb
$ sudo apt install ./gpio++_${version}_Linux_${arch}.deb
```

Compile from source:
```console
$ version=1.1
$ wget https://github.com/dimitry-ishenko/gpioxx/releases/download/v${version}/gpio++-${version}-Source.zip
$ unzip gpio++-${version}-Source.zip
$ mkdir gpio++-${version}-Source/build
$ cd gpio++-${version}-Source/build
$ cmake ..
$ make
$ sudo make install
```

### Usage

Example:
```cpp
#include <gpio++/gpio.hpp>
#include <iostream>
#include <stdexcept>

int main(int argc, char* argv[])
try
{
    if(argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <chip>" << std::endl;
        throw std::invalid_argument("Missing <chip> argument");
    }

    auto chip = gpio::get_chip(argv[1]);
    std::cout << "Chip info:" << std::endl;
    std::cout << "  type: " << chip->type() << std::endl;
    std::cout << "    id: " << chip->id() << std::endl;
    std::cout << "  name: " << chip->name() << std::endl;
    std::cout << "  pins: " << chip->pin_count() << std::endl;

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
$ g++ example.cpp -o example -lgpio++ -ldl
$ ./example gpiod:0
```

## Authors

* **Dimitry Ishenko** - dimitry (dot) ishenko (at) (gee) mail (dot) com

## License

This project is distributed under the GNU GPL license. See the
[LICENSE.md](LICENSE.md) file for details.
