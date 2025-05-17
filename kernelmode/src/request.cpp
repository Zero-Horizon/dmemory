#include "request.hpp"
#include "debug.hpp"

extern "C" NTSTATUS MmCopyVirtualMemory(
    PEPROCESS SourceProcess,
    PVOID SourceAddress,
    PEPROCESS TargetProcess,
    PVOID TargetAddress,
    SIZE_T BufferSize,
    KPROCESSOR_MODE PreviousMode,
    PSIZE_T ReturnSize
);


namespace driver {

    static PEPROCESS target_process = nullptr;

    NTSTATUS create(PDEVICE_OBJECT device_object, PIRP irp) {
        UNREFERENCED_PARAMETER(device_object);
        IoCompleteRequest(irp, IO_NO_INCREMENT);
        return irp->IoStatus.Status;
    }

    NTSTATUS close(PDEVICE_OBJECT device_object, PIRP irp) {
        UNREFERENCED_PARAMETER(device_object);
        IoCompleteRequest(irp, IO_NO_INCREMENT);
        return irp->IoStatus.Status;
    }

    NTSTATUS device_control(PDEVICE_OBJECT device_object, PIRP irp) {
        UNREFERENCED_PARAMETER(device_object);
        debug_print("[+] Device control called.\n");

        NTSTATUS status = STATUS_UNSUCCESSFUL;
        PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(irp);
        auto request = reinterpret_cast<Request*>(irp->AssociatedIrp.SystemBuffer);

        if (!stack || !request) {
            IoCompleteRequest(irp, IO_NO_INCREMENT);
            return status;
        }

        const ULONG control_code = stack->Parameters.DeviceIoControl.IoControlCode;

        switch (control_code) {
        case codes::attach:
            status = PsLookupProcessByProcessId(request->processId, &target_process);
            debug_print("[+] Attached to process.\n");
            break;

        case codes::read:
            if (target_process && request->address && request->size > 0) {
                SIZE_T bytes = 0;
                status = MmCopyVirtualMemory(
                    target_process,
                    request->address,
                    PsGetCurrentProcess(),
                    request->buffer,
                    request->size,
                    KernelMode,
                    &bytes
                );
                debug_print("[+] Read memory: %llu bytes.\n", bytes);
            }
            break;

        case codes::write:
            if (target_process && request->address && request->size > 0) {
                auto userBuffer = (char*)request + sizeof(Request);
                SIZE_T bytes = 0;
                status = MmCopyVirtualMemory(
                    PsGetCurrentProcess(),
                    userBuffer,
                    target_process,
                    request->address,
                    request->size,
                    KernelMode,
                    &bytes
                );
                debug_print("[+] Wrote memory: %llu bytes.\n", bytes);
            }
            break;

        default:
            debug_print("[-] Unknown control code: 0x%X\n", control_code);
            break;
        }

        irp->IoStatus.Status = status;
        irp->IoStatus.Information = sizeof(Request);
        IoCompleteRequest(irp, IO_NO_INCREMENT);
        return status;
    }
}
