These examples use a function I found in [ansicon](https://github.com/adoxa/ansicon) for getting information on the parent process of the current executable.

Both examples use simple code injection to make Win32 API calls from the parent process. I'm hoping to use the concept in a python C-extension to allow writing python scripts to improve upon the cmd.exe experience without resorting to hacking up a dynamic batch script.

## chpdir

	Microsoft Windows [Version 6.1.7601]
	Copyright (c) 2009 Microsoft Corporation.  All rights reserved.

	G:\cpp.workspace\sandbox\win32-parent-process>chpdir ..\..
	Success.

	G:\cpp.workspace>

## unsetenv

	Microsoft Windows [Version 6.1.7601]
	Copyright (c) 2009 Microsoft Corporation.  All rights reserved.

	G:\cpp.workspace\sandbox\win32-parent-process>echo %WIX%
	C:\Program Files\Microsoft SDKs\Installer XML v3.5\

	G:\cpp.workspace\sandbox\win32-parent-process>.\unsetenv.exe WIX
	Buffer: WIX=
	Success.

	G:\cpp.workspace\sandbox\win32-parent-process>echo %WIX%
	%WIX%