#include "stdafx.h"
#include "pyprocfilter.h"
#include "KnCommUser.h"
#include "..\Common\KernelUserCommon.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static PyMethodDef g_pyprocfilterMethods[] =
{
	{ "start", start, METH_VARARGS, "start process filtering"},
	{ "stop", stop, METH_VARARGS, "start process filtering" },
	{ NULL, NULL, 0, NULL },
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static struct PyModuleDef g_pyprocfilterInfo =
{
	PyModuleDef_HEAD_INIT,
	"pyprocfilter",
	"Process Filtering Module for Python 3.5 (version : 1.0.0.0)",
	-1,
	g_pyprocfilterMethods
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HMODULE						g_knCommUserDll = nullptr;
KNCOMM						g_knComm = nullptr;
FN_ConnectToKnComm			g_pfnConnectToKnComm = nullptr;
FN_DisconnectFromKnComm		g_pfnDisconnectFromKnComm = nullptr;
FN_ReplyDataViaKnComm		g_pfnReplyDataViaKnComm = nullptr;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject*					g_pCallbackObject = nullptr;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PyMODINIT_FUNC
PyInit_pyprocfilter(void)
{
	bool success = false;
	
	PyObject* pErrorType = PyExc_Exception;
	PCSTR pErrorMsg = "Unknown Error";

	do
	{

		g_knCommUserDll = LoadLibrary(L"KnCommUser.dll");
		if (g_knCommUserDll == nullptr)
		{
			pErrorMsg = "Failed to LoadLibrary KnCommUser.dll";
			break;
		}

		g_pfnConnectToKnComm = (FN_ConnectToKnComm)GetProcAddress(g_knCommUserDll, "ConnectToKnComm");
		g_pfnDisconnectFromKnComm = (FN_DisconnectFromKnComm)GetProcAddress(g_knCommUserDll, "DisconnectFromKnComm");
		g_pfnReplyDataViaKnComm = (FN_ReplyDataViaKnComm)GetProcAddress(g_knCommUserDll, "ReplyDataViaKnComm");

		if (g_pfnConnectToKnComm == nullptr || g_pfnDisconnectFromKnComm == nullptr || g_pfnReplyDataViaKnComm == nullptr)
		{
			pErrorMsg = "Failed to get functions of KnCommUser";
			break;
		}

		success = true;

	} while (false);
	
	if (success == false)
	{
		PyErr_SetString(pErrorType, pErrorMsg);
	}

	return success ? PyModule_Create(&g_pyprocfilterInfo) : nullptr;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject*  
start(PyObject* pSelf, PyObject* pArgs)
{
	bool success = false;

	PyObject* pErrorType = PyExc_Exception;
	PCSTR pErrorMsg = "Unknown Error";

	do
	{
		if (g_knComm != nullptr)
		{
			pErrorMsg = "Already Started...";
			break;
		}

		if (g_pCallbackObject != nullptr)
		{
			pErrorMsg = "Already Callback was Configured ...";
			break;
		}

		if (PyArg_ParseTuple(pArgs, "O", &g_pCallbackObject) == FALSE)
		{
			pErrorMsg = "Invalid Parameter...";
			break;
		}

		if (PyCallable_Check(g_pCallbackObject) == FALSE)
		{
			pErrorMsg = "Invalid Callback...";
			break;
		}

		KNCOMM_CB_INFO info;
		info.pfnOnDataRecv = OnDataRecv;
		info.pCallbackContext = nullptr;
		if (g_pfnConnectToKnComm(L"KnProcFilter", &info, &g_knComm) == FALSE)
		{
			pErrorMsg = "Error on ConnectToKnComm";
			break;
		}

		Py_XINCREF(g_pCallbackObject);

		success = true;

	} while (false);

	if (success == false)
	{
		PyErr_SetString(pErrorType, pErrorMsg);
		return nullptr;
	}

	Py_RETURN_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject*
stop(PyObject* pSelf, PyObject* pArgs)
{
	bool success = false;

	PyObject* pErrorType = PyExc_Exception;
	PCSTR pErrorMsg = "Unknown Error";

	do
	{
		if (g_knComm == nullptr)
		{
			pErrorMsg = "Not Started...";
			break;
		}

		g_pfnDisconnectFromKnComm(g_knComm);
		g_knComm = nullptr;

		Py_XDECREF(g_pCallbackObject);
		g_pCallbackObject = nullptr;
		
		success = true;

	} while (false);

	if (success == false)
	{
		PyErr_SetString(pErrorType, pErrorMsg);
		return nullptr;
	}

	Py_RETURN_TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CALLBACK OnDataRecvWrapper(
	IN	ULONG			dataId,
	IN	PVOID			pData,
	IN	SIZE_T			dataSize,
	IN	BOOL			isReplyRequired,
	IN	PVOID			pContext
	)
{
	__try
	{
		OnDataRecv(dataId, pData, dataSize, isReplyRequired, pContext);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		;
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CALLBACK OnDataRecv(
	IN	ULONG			dataId,
	IN	PVOID			pData,
	IN	SIZE_T			dataSize,
	IN	BOOL			isReplyRequired,
	IN	PVOID			pContext
	)
{
	PyObject* pArgList = nullptr;
	PyObject* pResult = nullptr;

	do
	{
		if (dataSize != sizeof(KN_PROCESS_INFO))
		{
			break;
		}

		PKN_PROCESS_INFO pProcessInfo = static_cast<PKN_PROCESS_INFO>(pData);
		if (pProcessInfo == nullptr)
		{
			break;
		}

		static const WCHAR dosPathPrefix[] = { L"\\??\\" };
		static const SIZE_T dosPathPrefixLength = wcslen(dosPathPrefix);

		char processPath[KN_MAX_PATH] = { 0 };

		if (wcsncmp(pProcessInfo->processPath, dosPathPrefix, dosPathPrefixLength) == 0)
		{
			WideCharToMultiByte(CP_ACP, 0, pProcessInfo->processPath + dosPathPrefixLength, -1, processPath, sizeof(processPath), NULL, NULL);
		}
		else
		{
			WideCharToMultiByte(CP_ACP, 0, pProcessInfo->processPath, -1, processPath, sizeof(processPath), NULL, NULL);
		}

		pArgList = Py_BuildValue("(iis)", pProcessInfo->parentProcessId, pProcessInfo->processId, processPath);
		if (pArgList == nullptr)
		{
			break;
		}

		pResult = PyEval_CallObject(g_pCallbackObject, pArgList);
		if (pResult == nullptr)
		{
			break;
		}

		BOOL isAllowed = TRUE;

		long result = PyLong_AsLong(pResult);
		
		if (result == 0)
		{
			isAllowed = FALSE;
		}

		KN_PROCESS_DECISION decision;
		decision.processId = pProcessInfo->processId;
		decision.isAllowed = isAllowed;

		g_pfnReplyDataViaKnComm(g_knComm, dataId, &decision, sizeof(decision));

	} while (false);

	if (pResult != nullptr)
	{
		Py_DECREF(pResult);
		pResult = nullptr;
	}

	if (pArgList != nullptr)
	{
		Py_DECREF(pArgList);
		pArgList = nullptr;
	}
}