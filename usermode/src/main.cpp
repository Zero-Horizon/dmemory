#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <iostream>

namespace driver {
    namespace codes {
        constexpr ULONG attach = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x696, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
        constexpr ULONG read = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x697, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
        constexpr ULONG write = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x698, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
    }

    struct Request {
        HANDLE processId;    
        void* address;       
        void* buffer;        
        size_t size;         
    };

    bool attachToProcess(HANDLE hDevice, DWORD processId) {
        Request req = {};
        req.processId = reinterpret_cast<HANDLE>(processId);
        req.address = nullptr;
        req.buffer = nullptr;
        req.size = 0;

        DWORD bytesReturned = 0;
        BOOL success = DeviceIoControl(
            hDevice,
            codes::attach,
            &req,
            sizeof(req),
            &req,
            sizeof(req),
            &bytesReturned,
            nullptr
        );

        return success && bytesReturned == sizeof(req);
    }

    bool readMemory(HANDLE hDevice, void* address, void* buffer, size_t size) {
        Request req = {};
        req.address = address;
        req.buffer = buffer;
        req.size = size;
        req.processId = nullptr;

        DWORD bytesReturned = 0;
        BOOL success = DeviceIoControl(
            hDevice,
            codes::read,
            &req,
            sizeof(req),
            &req,
            sizeof(req),
            &bytesReturned,
            nullptr
        );

        return success && bytesReturned == sizeof(req);
    }

    bool writeMemory(HANDLE hDevice, void* address, const void* buffer, size_t size) {
        struct WriteRequest {
            driver::Request base;
            char data[4096];
        };

        if (size > sizeof(WriteRequest::data)) {
            std::cerr << "[-] Write size too large\n";
            return false;
        }

        WriteRequest req = {};
        req.base.address = address;
        req.base.size = size;
        req.base.processId = nullptr;

        memcpy(req.data, buffer, size);

        DWORD bytesReturned = 0;
        BOOL success = DeviceIoControl(
            hDevice,
            driver::codes::write,
            &req,
            sizeof(driver::Request) + (DWORD)size,
            nullptr,
            0,
            &bytesReturned,
            nullptr
        );

        return success;
    }
}

DWORD getProcessIdByName(const std::wstring& processName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(snapshot, &entry)) {
        do {
            if (_wcsicmp(entry.szExeFile, processName.c_str()) == 0) {
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        } while (Process32NextW(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return 0;
}

int main() {
    HANDLE driver = CreateFileW(
        L"\\\\.\\ZeroDriver",
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (driver == INVALID_HANDLE_VALUE) {
        std::cerr << "[-] Failed to open driver handle: " << GetLastError() << "\n";
        system("pause");
        return 1;
    }

    DWORD pid = getProcessIdByName(L"notepad.exe");
    if (pid == 0) {
        std::cerr << "[-] Could not find process\n";
        CloseHandle(driver);
        system("pause");
        return 1;
    }

    if (!driver::attachToProcess(driver, pid)) {
        std::cerr << "[-] Attach failed: " << GetLastError() << "\n";
        CloseHandle(driver);
        system("pause");
        return 1;
    }
    std::cout << "[+] Attached to process " << pid << "\n";

    int buffer = 0;
    if (driver::readMemory(driver, (void*)0x7FF6B0000000, &buffer, sizeof(buffer))) {
        std::cout << "[+] Read success: " << buffer << "\n";
    }
    else {
        std::cerr << "[-] Read failed\n";
    }
    
    int newValue = 12345;
    if (driver::writeMemory(driver, (void*)0x7FF6B0000000, &newValue, sizeof(newValue))) {
        std::cout << "[+] Write success\n";
    }
    else {
        std::cerr << "[-] Write failed\n";
    }

    CloseHandle(driver);
    system("pause");
    return 0;
}
