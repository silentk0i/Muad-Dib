#pragma once
#include <Windows.h>
#include <iostream>
#include <string>
#include <Psapi.h>
#include "functionPtrs.h"
#include "defs.h"

HANDLE hijackProcessWorkerFactory(HANDLE processHandle);
HANDLE hijackProcessHandle(_In_ HANDLE targetProcess, _In_ const wchar_t* handleTypeName, _In_ uint32_t desiredAccess);
HANDLE enumerateProcess(_In_ wchar_t* processName, _Outptr_opt_ uint32_t* pPid);