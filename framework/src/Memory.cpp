#include "Memory.hpp"
#include <tlhelp32.h>
#include <stdexcept>

Memory::Memory(const std::wstring& targetProcess, const std::wstring& deviceName) {
    driverHandle_ = CreateFileW(deviceName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (driverHandle_ == INVALID_HANDLE_VALUE)
        throw std::runtime_error("Failed to open driver");

    processId_ = GetProcessId(targetProcess);
    if (!AttachToProcess())
        throw std::runtime_error("Failed to attach to process");
}

Memory::~Memory() {
    if (driverHandle_ != INVALID_HANDLE_VALUE)
        CloseHandle(driverHandle_);
}

bool Memory::AttachToProcess() {
    Request req{ reinterpret_cast<HANDLE>(static_cast<ULONG_PTR>(processId_)), nullptr, nullptr, 0 };
    DWORD out = 0;
    return DeviceIoControl(driverHandle_, attach, &req, sizeof(req), &req, sizeof(req), &out, nullptr);
}

DWORD Memory::GetProcessId(const std::wstring& name) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32W entry{ sizeof(entry) };
    DWORD pid = 0;
    if (Process32FirstW(snap, &entry)) {
        do {
            if (_wcsicmp(entry.szExeFile, name.c_str()) == 0) {
                pid = entry.th32ProcessID;
                break;
            }
        } while (Process32NextW(snap, &entry));
    }
    CloseHandle(snap);
    return pid;
}