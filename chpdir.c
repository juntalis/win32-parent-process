#include <WinSDKVer.h>
// Speed up build process with minimal headers.
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <SDKDDKVer.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "parent.inl"

int main(int argc, char* argv[])
{

	// Main DLL we will need to load
	HANDLE hProcess = NULL;
	LPCSTR pszPathRemote = NULL;
	HANDLE hRemoteThread = NULL; 
	HMODULE kernel32 = NULL;
	PROCESS_INFORMATION pi;
	
	// Main function we will need to import
	FARPROC rchdir = NULL;
	int cch = 0; int cb = 0;
	int result = 0;
	const char* pathStr;

	if (argc == 1) {
		printf("Usage: %s path\n", argv[0]);
		return 0;
	}
	pathStr = (const char*)argv[1];
	
	// Get the address of the main DLL
	kernel32 = LoadLibraryA("kernel32.dll");
	
	// Get our functions
	rchdir = GetProcAddress(kernel32, "SetCurrentDirectoryA");
	
	if (!GetParentProcessInfo( &pi )) {
		printf("Could not get parent process.");
		return 1;
	}
	
	hProcess = OpenProcess(
		PROCESS_QUERY_INFORMATION |   
		PROCESS_CREATE_THREAD     | 
		PROCESS_VM_OPERATION |
		PROCESS_VM_WRITE,  // For CreateRemoteThread
		FALSE, pi.dwProcessId
	);
	
	if( !hProcess ) {
		printf("Error: Couldn't open parent process.\n");
		return 1;
	}
	
	// len(var) + 1
	cch = lstrlenA(pathStr) + 1;
	cb  = cch * sizeof(const char);
	
	pszPathRemote = (LPCSTR) VirtualAllocEx( hProcess, NULL, cb, MEM_COMMIT, PAGE_READWRITE);
	if ( pszPathRemote == NULL) 
	{
		printf("Error: Unable to allocate memory\n");
		result = 1;
		goto cleanup;
	}
	
	if (!WriteProcessMemory(hProcess, (LPVOID)pszPathRemote,
		(LPCVOID)pathStr, (SIZE_T)cb, NULL)) 
	{
		printf("Error: Failed to write.\n");
		result = 1;
		goto cleanup;
	};
	
	// Create remote thread and inject the function call.
	hRemoteThread = CreateRemoteThread( hProcess, NULL, 0, 
		(LPTHREAD_START_ROUTINE)rchdir, 
		(LPVOID)pszPathRemote, 0L, NULL);

	if( !hRemoteThread )
	{
		printf("Error: Failed to update process.\n");
		result = 1;
		goto cleanup;
	}

	WaitForSingleObject( hRemoteThread, INFINITE );
	printf("Success.\n");
	goto cleanup;
cleanup:
	// Free the remote memory that contained the DLL's pathname
	if (pszPathRemote != NULL) 
		VirtualFreeEx(hProcess, (LPVOID)pszPathRemote, 0, MEM_RELEASE);

	if ( hRemoteThread != NULL) 
		CloseHandle(hRemoteThread);
	
	if ( hProcess != NULL) 
		CloseHandle(hProcess);
	return result;
}