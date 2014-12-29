# lib605

This is a library for interacting with the MSR605/606 Magnetic Strip Reader/Writer

## Building

To build just run make, then copy the header `lib605.hpp` into `/usr/local/include` and `lib605.so` into `/usr/local/lib` then run a quick `ldconfig` to update the library cache

## Usage

Prior to use, you must first load the `usbserial` and `pl2303` modules so that when you connect the device it will create a `/dev/ttyUSBX` where `X` is the device number. After that you are free to use the library.

In the future it might be possible for the library to load the modules if they are not loaded already, but that is for another day.

To use the library, assuming you followed the building steps, just include the `lib605.hpp` header and create a new `lib605::MSR` object. Then call the `lib604::MSR.Initialize()` Method, this should initialize the device and preform a self test. The method will return true if the initialization succeeded.
