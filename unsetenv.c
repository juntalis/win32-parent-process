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
	LPCSTR pszVarNameRemote = NULL;
	HANDLE hRemoteThread = NULL; 
	HMODULE msvcrt = NULL;
	PROCESS_INFORMATION pi;
	
	// Main function we will need to import
	FARPROC rputenv = NULL;
	int cch = 0; int cb = 0;
	int result = 0;
	const char* buffer = NULL;
	const char* varName;

	if (argc == 1) {
		printf("Usage: %s varname\n", argv[0]);
		return 0;
	}
	varName = (const char*)argv[1];
	
	// Get the address of the main DLL
	msvcrt = LoadLibraryA("msvcrt.dll");
	if(!msvcrt) {
		printf("Could not load MSVCRT.dll\n");
		return 1;
	}
	
	// Get our functions
	rputenv = GetProcAddress(msvcrt, "_putenv");
	
	if (!GetParentProcessInfo( &pi )) {
		printf("Could not get parent process.");
		goto cleanup;
	}
	
	hProcess = OpenProcess(
		PROCESS_QUERY_INFORMATION |	 
		PROCESS_CREATE_THREAD		 | 
		PROCESS_VM_OPERATION |
		PROCESS_VM_WRITE,	// For CreateRemoteThread
		FALSE, pi.dwProcessId
	);
	
	if( !hProcess ) {
		printf("Error: Couldn't open parent process.\n");
		return 1;
	}
	
	// len(var) + len(=) + 1
	cch = lstrlenA(varName) + 2;
	cb	= cch * sizeof(const char);
	
	buffer = (const char*)malloc(cb);
	if(buffer == NULL) {
		printf("Could not allocate memory for our buffer.\n");
		goto cleanup;
	}
	
	memset((void*)buffer, 0, cb);
	sprintf((char*)buffer, "%s=", varName);
	printf("Buffer: %s\n", buffer);
	
	pszVarNameRemote = (LPCSTR) VirtualAllocEx( hProcess, NULL, cb, MEM_COMMIT, PAGE_READWRITE);
	if ( pszVarNameRemote == NULL) 
	{
		printf("Error: Unable to allocate memory\n");
		result = 1;
		goto cleanup;
	}
	
	if (!WriteProcessMemory(hProcess, (LPVOID)pszVarNameRemote,
		(LPCVOID)buffer, (SIZE_T)cb, NULL)) 
	{
		printf("Error: Failed to write.\n");
		result = 1;
		goto cleanup;
	};
	
	// Create remote thread and inject the function call.
	hRemoteThread = CreateRemoteThread( hProcess, NULL, 0, 
		(LPTHREAD_START_ROUTINE)rputenv, 
		(LPVOID)pszVarNameRemote, 0L, NULL);

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
	if (pszVarNameRemote != NULL) 
		VirtualFreeEx(hProcess, (LPVOID)pszVarNameRemote, 0, MEM_RELEASE);

	if ( hRemoteThread != NULL) 
		CloseHandle(hRemoteThread);

	if ( buffer != NULL)
		free((void*)buffer);
	
	if ( hProcess != NULL) 
		CloseHandle(hProcess);
	
	if ( msvcrt != NULL)
		FreeLibrary(msvcrt);
	return result;
}