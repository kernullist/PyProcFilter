#pragma once


#define KN_MAX_PATH		(512)


#pragma pack (push, 1)

typedef struct _KN_PROCESS_INFO
{
	ULONG		parentProcessId;
	ULONG		processId;
	wchar_t		processPath[KN_MAX_PATH];

} KN_PROCESS_INFO, *PKN_PROCESS_INFO;


typedef struct _KN_PROCESS_DECISION
{
	ULONG		processId;
	ULONG		isAllowed;

} KN_PROCESS_DECISION, *PKN_PROCESS_DECISION;


#pragma pack (pop)