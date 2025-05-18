Вот строгая и лаконичная версия `README.md` на английском без эмодзи:

---

# Memory

**Memory** is a lightweight C++ wrapper for interacting with a custom Windows kernel-mode driver. It allows reading and writing memory of external processes by communicating with the driver through device control codes.

## Features

* Read memory from an external process
* Write memory to an external process
* Retrieve base address of loaded modules
* Simple and type-safe templated API
* Works via a custom kernel-mode driver (e.g. `ZeroDriver`)

## Requirements

* A properly signed and loaded custom driver that supports IOCTL for attach, read, and write (e.g. `ZeroDriver.sys`)
* Administrator privileges

## Usage Example

```cpp
#include "Memory.hpp"
#include <iostream>

int main() {
    try {
        // Attach to target process by name
        Memory mem(L"notepad.exe");

        // Example: read an integer from memory
        uintptr_t address = 0x7FF6B0000000;
        int value = mem.Read<int>(address);
        std::cout << "Read value: " << value << std::endl;

        // Example: write a new integer value
        mem.Write<int>(address, 1337);
        std::cout << "Wrote 1337 to memory." << std::endl;
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }

    system("pause");
    return 0;
}
```

## Driver Interface

The driver must support the following control codes:

```cpp
constexpr ULONG attach = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x696, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
constexpr ULONG read   = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x697, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
constexpr ULONG write  = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x698, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
```

Each request must conform to this structure:

```cpp
struct Request {
    HANDLE processId;
    void* address;
    void* buffer;
    SIZE_T size;
};
```

## Notes

* The target process must be running before attempting to attach
* Make sure the specified address is valid and accessible
* This code assumes the kernel driver is already loaded and accessible via a symbolic link (e.g. `\\.\ZeroDriver`)

## License

This project is licensed under the MIT License.
