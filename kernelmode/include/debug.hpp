#pragma once
#include <ntddk.h>

inline void debug_print(PCSTR format, ...) {
#ifdef DEBUG
    va_list args;
    va_start(args, format);
    vDbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, format, args);
    va_end(args);
#else
    UNREFERENCED_PARAMETER(format);
#endif
}