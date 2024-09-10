#pragma once
#include <windows.h>
#include <cstdint>
#include "defs.h"
#include "injection.h"
#include "Utils.h"


class Process {

private:

	wchar_t* name = nullptr;
	void* remotePayload = nullptr;
	HANDLE            handleToHijack = nullptr;
	HANDLE            processHandle = nullptr;
	bool              isInitialized = false;
	uint32_t          PID = 0;

	HandleHijackClass hijackType;

public:
	bool ProcessWorkerFactoryInject(unsigned char*, size_t);
	bool init();


	~Process();
	Process(wchar_t* processName, HandleHijackClass hijackType) : name(processName), hijackType(hijackType) {}
};