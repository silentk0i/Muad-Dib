#pragma once
#include <Windows.h>
#include <iostream>
#include <stdint.h>
#include "functionPtrs.h"
#include "structures.h"
#include "defs.h"

bool InjectViaWorkerFactoryStartRoutine(_In_ HANDLE targetProcess, _In_ HANDLE hWorkerFactory, _In_ void* localPayloadAddress, _In_ size_t payloadSize);
int LoadShell(unsigned char*, size_t, wchar_t*);