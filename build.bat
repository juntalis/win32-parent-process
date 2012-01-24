@ECHO OFF
setlocal
set VARS_SET=0
if not "%VS100COMNTOOLS%x"=="x" ((call "%VS100COMNTOOLS%\vsvars32.bat") && (set VARS_SET=1))
if "%VARS_SET%"=="0" (
	if not "%VS90COMNTOOLS%x"=="x" ((call "%VS90COMNTOOLS%\vsvars32.bat") && (set VARS_SET=1))
)
if "%VARS_SET%"=="0" (
	if not "%VS80COMNTOOLS%x"=="x" ((call "%VS80COMNTOOLS%\vsvars32.bat") && (set VARS_SET=1))
)
if not exist "%~dp0obj" mkdir "%~dp0obj"
for %%A IN (.\*.c) do @cl -O2 "-Fo%~dp0obj\%%~nA.obj" "%%~A" -link "-OUT:%%~dpnA.exe"
endlocal