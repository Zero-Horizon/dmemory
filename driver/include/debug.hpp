#pragma once
#include <ntifs.h>
#include <ntstrsafe.h>

inline void debug_print(const char* format, ...) {
#ifndef DEBUG
    char buffer[512];

    va_list args;
    va_start(args, format);
    RtlStringCchVPrintfA(buffer, sizeof(buffer), format, args);
    va_end(args);

    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "%s", buffer));
#else
    UNREFERENCED_PARAMETER(format);
#endif
}
