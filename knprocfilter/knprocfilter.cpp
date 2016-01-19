#include "knprocfilter.h"
#include "KnComm.h"
#include "..\Common\KernelUserCommon.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ULONG g_myKnCommId = 0;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EXTERN_C
NTSTATUS
DriverEntry(
	IN	PDRIVER_OBJECT		pDriverObject,
	IN	PUNICODE_STRING		pRegistryPath
	)
{
	UNREFERENCED_PARAMETER(pRegistryPath);

	DbgPrint("[%s:%d]\n", _FN_, _LN_);

	for (int i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; ++i)
	{
		pDriverObject->MajorFunction[i] = PassRoutine;
	}

	pDriverObject->DriverUnload = UnloadRoutine;


	//
	// KNCOMM에 연결하자.
	//
	
	KNCOMM_CB_INFO info;
	info.pfnOnUserConnect = OnUserConnect;
	info.pfnOnUserDisconnect = OnUserDisconnect;
	info.pfnOnUserIoctl = nullptr;
	info.pCallbackContext = nullptr;

	NTSTATUS status = ConnectToKnComm(L"KnProcFilter", &info, &g_myKnCommId);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("[%s:%d] Error on ConnectToKnComm. status : 0x%X\n", _FN_, _LN_, status);
		return status;
	}
	
	return STATUS_SUCCESS;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
UnloadRoutine(
	IN	PDRIVER_OBJECT		pDriverObject
	)
{
	UNREFERENCED_PARAMETER(pDriverObject);

	DbgPrint("[%s:%d]\n", _FN_, _LN_);

	//
	// KNCOMM과 연결을 끊자.
	//

	DisconnectFromKnComm(g_myKnCommId);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NTSTATUS
PassRoutine(
	IN	PDEVICE_OBJECT		pDeviceObject,
	IN	PIRP				pIrp
	)
{
	UNREFERENCED_PARAMETER(pDeviceObject);

	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
NTAPI
OnUserConnect(
	IN	HANDLE				processId,
	IN	PVOID				pContext
	)
{
	UNREFERENCED_PARAMETER(pContext);

	DbgPrint("[%s:%d] User Process ID : %d\n", _FN_, _LN_, processId);
	
	NTSTATUS status = PsSetCreateProcessNotifyRoutineEx(KnProcessNotifyRoutineEx, FALSE);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("[%s:%d] Error on PsSetCreateProcessNotifyRoutineEx(FALSE). status : 0x%X\n", _FN_, _LN_, status);
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
NTAPI
OnUserDisconnect(
	IN	HANDLE				processId,
	IN	PVOID				pContext
	)
{
	UNREFERENCED_PARAMETER(pContext);

	DbgPrint("[%s:%d] User Process ID : %d\n", _FN_, _LN_, processId);
	
	NTSTATUS status = PsSetCreateProcessNotifyRoutineEx(KnProcessNotifyRoutineEx, TRUE);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("[%s:%d] Error on PsSetCreateProcessNotifyRoutineEx(TRUE). status : 0x%X\n", _FN_, _LN_, status);
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
KnProcessNotifyRoutineEx(
	PEPROCESS				pProcess,
	HANDLE					processId,
	PPS_CREATE_NOTIFY_INFO	pCreateInfo
	)
{
	UNREFERENCED_PARAMETER(pProcess);

	PKN_PROCESS_INFO pProcessInfo = nullptr;
	PVOID pReply = nullptr;

	do
	{
		if (pCreateInfo == nullptr)
		{
			break;
		}

		DbgPrint("[%s:%d] New Process ID : %d\n", _FN_, _LN_, processId);

		if (pCreateInfo->ImageFileName == nullptr || pCreateInfo->ImageFileName->Length >= KN_MAX_PATH * sizeof(WCHAR))
		{
			break;
		}

		pProcessInfo = (PKN_PROCESS_INFO)ExAllocatePoolWithTag(PagedPool, sizeof(KN_PROCESS_INFO), TAG_NAME);
		if (pProcessInfo == nullptr)
		{
			break;
		}

		RtlZeroMemory(pProcessInfo, sizeof(KN_PROCESS_INFO));

		pProcessInfo->parentProcessId = (ULONG)pCreateInfo->ParentProcessId;
		pProcessInfo->processId = (ULONG)processId;
		RtlCopyMemory(pProcessInfo->processPath, pCreateInfo->ImageFileName->Buffer, pCreateInfo->ImageFileName->Length);

		SIZE_T replyDataSize = 0;

		//
		// 프로세스 실행 여부를 응용프로그램에 쿼리한다. (타임아웃 10초)
		//

		if (SendToUserViaKnComm(g_myKnCommId, pProcessInfo, sizeof(KN_PROCESS_INFO), TRUE, 10000, &pReply, &replyDataSize) != STATUS_SUCCESS)
		{
			break;
		}

		if (replyDataSize != sizeof(KN_PROCESS_DECISION))
		{
			break;
		}

		PKN_PROCESS_DECISION pUserDecision = (PKN_PROCESS_DECISION)pReply;

		DbgPrint("[%s:%d] Process Id : %d ==> isAllowed : %d\n", _FN_, _LN_, processId, pUserDecision->isAllowed);

		if (pUserDecision->isAllowed == FALSE)
		{
			//
			// 허용되지 않은 프로세스 실행이다.
			//

			DbgPrint("[%s:%d] Not Allowed Process!!!\n", _FN_, _LN_);

			pCreateInfo->CreationStatus = STATUS_ACCESS_DENIED;
		}

	} while (FALSE);

	if (pReply != nullptr)
	{
		ReleaseKnCommDataBuffer(pReply);
		pReply = nullptr;
	}

	if (pProcessInfo != nullptr)
	{
		ExFreePoolWithTag(pProcessInfo, TAG_NAME);
		pProcessInfo = nullptr;
	}
}