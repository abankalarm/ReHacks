/* by karan bamal */

#include <iostream>
#include <Windows.h>
#include <sddl.h>
#include <TlHelp32.h>

int main()
{
	printf("# By Karan Bamal, feel free to use, just dont remove credits.\n");
	// open handle to current process, this gets leaked
	HANDLE hParent = ::OpenProcess(PROCESS_ALL_ACCESS, TRUE, GetCurrentProcessId());
	printf("[+] Leaky Handle spawned, process handle = %p\n", hParent);
	
	// open unprivileged/child process with user permissions 
	// get low permission token, explorer.exe is ever present process we can copy token of
	PROCESSENTRY32 pt;
	DWORD pid = 0;
	HANDLE hSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pt.dwSize = sizeof(PROCESSENTRY32);
	if (::Process32First(hSnap, &pt)) {
		do {
			if (!wcscmp(pt.szExeFile, L"explorer.exe")) {
				pid = pt.th32ProcessID;
				break;
			}
		} while (::Process32Next(hSnap, &pt));
		::CloseHandle(hSnap);
	}
	if (pid == 0) {
		printf("[-] Failed to acquire PID for explorer.exe, errcode: %d\n", ::GetLastError());
		return -1;
	}

	//open explorer.exe process with user permissions and gets its token
	// and then use this token to create child process with user perms.
	HANDLE hUsertoken, hUserproc, hProcess;

	hUserproc = ::OpenProcess(PROCESS_ALL_ACCESS, false, pid);
	if (!::OpenProcessToken(hUserproc, TOKEN_ALL_ACCESS, &hUsertoken)) {
		printf("[-] Failed to open explorer.exe, errcode: %d\n", ::GetLastError());
		::CloseHandle(hUserproc);
		return -1;
	}	

	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	WCHAR child_proc_name[] = L"C:\\leaky_handle_child.exe";

	if (!::CreateProcessAsUser(hUsertoken, child_proc_name, nullptr, nullptr, nullptr, TRUE, CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi)) {
		printf("[-] Failed to spawn CHILD process, errcode: %d\n", ::GetLastError());
		::CloseHandle(hUsertoken);
		return(-1);
	}
	printf("[+] Child process spawned, proc id: %d\n", pi.dwProcessId);
	printf("[!] This child process inherits priveleged handles of the parent process.\n");
	// wait for child proc to end
	::WaitForSingleObject(pi.hProcess, INFINITE);

	::CloseHandle(pi.hProcess);
	::CloseHandle(pi.hThread);

	// just to stop cmd from closing quikly
	int end;std::cin >> end;
	return 0;
}
