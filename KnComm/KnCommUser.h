#pragma once


#include <Windows.h>


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef HANDLE	KNCOMM, *PKNCOMM;

typedef void (CALLBACK* FN_ONDATARECV)(
	IN	ULONG			dataId,
	IN	PVOID			pData,
	IN	SIZE_T			dataSize,
	IN	BOOL			isReplyRequired,
	IN	PVOID			pContext
	);

typedef struct _KNCOMM_CB_INFO
{
	FN_ONDATARECV		pfnOnDataRecv;
	PVOID				pCallbackContext;

} KNCOMM_CB_INFO, *PKNCOMM_CB_INFO;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C"
{
#endif

BOOL 
WINAPI
ConnectToKnComm(
	IN	PCWSTR			pUniqueName,
	IN	PKNCOMM_CB_INFO	pKnCommCallbackInfo,
	OUT	PKNCOMM			pKnCommHandle
	);

BOOL
WINAPI
DisconnectFromKnComm(
	IN	KNCOMM			knCommHandle
	);

BOOL
WINAPI
ReplyDataViaKnComm(
	IN	KNCOMM			knCommHandle,
	IN	ULONG			dataId,
	IN	PVOID			pData,
	IN	SIZE_T			dataSize
	);

BOOL 
WINAPI
IoctlViaKnComm(
	IN	KNCOMM			knCommHandle,
	IN	ULONG			controlCode,
	IN	PVOID			pInputBuffer,
	IN	ULONG			inputBufferSize,
	OUT	PVOID			pOutputBuffer,
	IN	ULONG			outputBufferSize
	);

#ifdef __cplusplus
}
#endif


#ifndef KNCOMMUSER_EXPORTS

typedef BOOL (WINAPI* FN_ConnectToKnComm)(
	IN	PCWSTR			pUniqueName,
	IN	PKNCOMM_CB_INFO	pKnCommCallbackInfo,
	OUT	PKNCOMM			pKnCommHandle
	);

typedef BOOL (WINAPI* FN_DisconnectFromKnComm)(
	IN	KNCOMM			knCommHandle
	);

typedef BOOL (WINAPI* FN_ReplyDataViaKnComm)(
	IN	KNCOMM			knCommHandle,
	IN	ULONG			dataId,
	IN	PVOID			pData,
	IN	SIZE_T			dataSize
	);

typedef BOOL (WINAPI* FN_IoctlViaKnComm)(
	IN	KNCOMM			knCommHandle,
	IN	ULONG			controlCode,
	IN	PVOID			pInputBuffer,
	IN	ULONG			inputBufferSize,
	OUT	PVOID			pOutputBuffer,
	IN	ULONG			outputBufferSize
	);

#endif