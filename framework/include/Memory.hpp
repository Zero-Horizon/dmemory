#pragma once
#include <windows.h>
#include <string>
#include <vector>

class Memory {
public:
    Memory(const std::wstring& targetProcess, const std::wstring& deviceName = L"\\\\.\\ZeroDriver");
    ~Memory();

    template <typename T>
    T Read(uintptr_t address);

    template <typename T>
    void Write(uintptr_t address, const T& value);

    uintptr_t GetModuleBase(const std::wstring& moduleName);

    Memory(const Memory&) = delete;
    Memory& operator=(const Memory&) = delete;

private:
    HANDLE driverHandle_;
    DWORD processId_;

    bool AttachToProcess();

    struct Request {
        HANDLE processId;
        void* address;
        void* buffer;
        SIZE_T size;
    };

    static constexpr ULONG attach = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x696, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
    static constexpr ULONG read = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x697, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
    static constexpr ULONG write = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x698, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

    DWORD GetProcessId(const std::wstring& name);
};

template <typename T>
T Memory::Read(uintptr_t address) {
    T buffer{};
    Request req{ reinterpret_cast<HANDLE>(processId_), reinterpret_cast<void*>(address), &buffer, sizeof(T) };
    DWORD out = 0;
    DeviceIoControl(driverHandle_, read, &req, sizeof(req), &req, sizeof(req), &out, nullptr);
    return buffer;
}

template <typename T>
void Memory::Write(uintptr_t address, const T& value) {
    std::vector<uint8_t> packet(sizeof(Request) + sizeof(T));
    auto* req = reinterpret_cast<Request*>(packet.data());
    req->processId = reinterpret_cast<HANDLE>(processId_);
    req->address = reinterpret_cast<void*>(address);
    req->size = sizeof(T);
    memcpy(packet.data() + sizeof(Request), &value, sizeof(T));
    DWORD out = 0;
    DeviceIoControl(driverHandle_, write, req, static_cast<DWORD>(packet.size()), nullptr, 0, &out, nullptr);
}