#include <ntifs.h>
#include <ntstrsafe.h>

#include "request.hpp"
#include "debug.hpp"

extern "C" {
	NTKERNELAPI NTSTATUS IoCreateDriver(
		PUNICODE_STRING DriverName,
		PDRIVER_INITIALIZE InitializationFunction
	);
}

NTSTATUS driver_main(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path) {
	UNREFERENCED_PARAMETER(registry_path);

	UNICODE_STRING device_name = {};
	RtlInitUnicodeString(&device_name, L"\\Device\\ZeroDriver");

	PDEVICE_OBJECT device_object = nullptr;
	NTSTATUS status = IoCreateDevice(
		driver_object, 0, &device_name,
		FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN,
		FALSE, &device_object
	);

	if (!NT_SUCCESS(status)) {
		debug_print("[-] Failed to create driver device.\n");
		return status;
	}

	debug_print("[+] Driver device created successfully.\n");

	UNICODE_STRING symbolic_link = {};
	RtlInitUnicodeString(&symbolic_link, L"\\DosDevices\\ZeroDriver");

	status = IoCreateSymbolicLink(&symbolic_link, &device_name);
	if (!NT_SUCCESS(status)) {
		debug_print("[-] Failed to create symbolic link.\n");
		return status;
	}

	debug_print("[+] Symbolic link created successfully.\n");

	SetFlag(device_object->Flags, DO_BUFFERED_IO);

	driver_object->MajorFunction[IRP_MJ_CREATE] = driver::create;
	driver_object->MajorFunction[IRP_MJ_CLOSE] = driver::close;
	driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = driver::device_control;

	ClearFlag(device_object->Flags, DO_DEVICE_INITIALIZING);

	debug_print("[+] Driver initialized successfully.\n");
	return status;
}

extern "C"
NTSTATUS DriverEntry() {
	debug_print("[*] Loading kernel driver!\n");

	UNICODE_STRING driver_name = {};
	RtlInitUnicodeString(&driver_name, L"\\Driver\\ZeroDriver");

	return IoCreateDriver(&driver_name, &driver_main);
}
