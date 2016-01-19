#pragma once

#include "python.h"


static PyObject*  start(PyObject* pSelf, PyObject* pArgs);

static PyObject*  stop(PyObject* pSelf, PyObject* pArgs);

static void CALLBACK OnDataRecvWrapper(
	IN	ULONG			dataId,
	IN	PVOID			pData,
	IN	SIZE_T			dataSize,
	IN	BOOL			isReplyRequired,
	IN	PVOID			pContext
	);

static void CALLBACK OnDataRecv(
	IN	ULONG			dataId,
	IN	PVOID			pData,
	IN	SIZE_T			dataSize,
	IN	BOOL			isReplyRequired,
	IN	PVOID			pContext
	);