#pragma once

extern "C"
{
#include <ntddk.h>
#include <wdm.h>
}


#define _FN_				__FUNCTION__

#define _LN_				__LINE__

#define TAG_NAME			'FPNK'


EXTERN_C
NTSTATUS
DriverEntry(
	IN	PDRIVER_OBJECT		pDriverObject,
	IN	PUNICODE_STRING		pRegistryPath
	);

void
UnloadRoutine(
	IN	PDRIVER_OBJECT		pDriverObject
	);

NTSTATUS
PassRoutine(
	IN	PDEVICE_OBJECT		pDeviceObject,
	IN	PIRP				pIrp
	);

void
NTAPI
OnUserConnect(
	IN	HANDLE				processId,
	IN	PVOID				pContext
	);

void
NTAPI
OnUserDisconnect(
	IN	HANDLE				processId,
	IN	PVOID				pContext
	);

void
KnProcessNotifyRoutineEx(
	PEPROCESS				pProcess,
	HANDLE					processId,
	PPS_CREATE_NOTIFY_INFO	pCreateInfo
	);
