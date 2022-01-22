
#include <iostream>
#include <stdio.h>
#include <Windows.h>

BOOL IsElevated() {
    BOOL IsElevated = FALSE;
    HANDLE hToken = NULL;

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION Elevation;
        DWORD cbSize = sizeof(TOKEN_ELEVATION);
        if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize)) {
            IsElevated = Elevation.TokenIsElevated;
        }
    }
    if (hToken) {
        CloseHandle(hToken);
    }
    return IsElevated;
}

#define ERROR(x) {printf("[-] Exiting, Reason: %s",x);Sleep(10000); return 1;}

int main()
{
    std::cout << "Hello! This is client\n";
    std::cout << "This client works with the server by communicating over a named pipe\n";
    std::cout << "Since both Client and server use admin priveleges, It is protected from low priveleges malicious processes\n";
    
    if (!IsElevated())
        ERROR("The process is not running with ADMIN priveleges\n");

    const WCHAR* PipeName = L"\\\\.\\pipe\\supersecurechannel";

    std::cout << "Waiting for Server to come online";

    if (!::WaitNamedPipe(PipeName, 10000)) {
        ERROR("Failed to connect to SERVER, make sure its running.");
    }
    
    HANDLE NamedPipe = ::CreateFile(PipeName, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (NamedPipe == INVALID_HANDLE_VALUE) {
        ERROR("Failed to send data.");
    }


    WCHAR DATA[] = TEXT("\nFLAG-REHACKS_NAMED_PIPES_PWNED\n");
    DWORD ByteWritten;
    if (!WriteFile(NamedPipe, DATA, sizeof(DATA), &ByteWritten, NULL))
        ERROR("Failed to write to the pipe");

    std::cout << "[+] EXITING, TASK COMPLETED\n";
    Sleep(10000);
    return 0;
}
