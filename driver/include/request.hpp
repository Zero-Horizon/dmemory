#pragma once

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
        SIZE_T size;
    };

    NTSTATUS create(PDEVICE_OBJECT device_object, PIRP irp);
    NTSTATUS close(PDEVICE_OBJECT device_object, PIRP irp);
    NTSTATUS device_control(PDEVICE_OBJECT device_object, PIRP irp);
}