@echo off
setlocal EnableExtensions DisableDelayedExpansion
cd /d "%~dp0"

set "LOG=%CD%\Build.log"
set "VSINSTALL="
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
set "WORKDIR=%TEMP%\ArcanaAI_Build_%RANDOM%_%RANDOM%_%RANDOM%"

if not exist "src\Proxy.cpp" goto :project_error
if not exist "src\ComVsCom.cpp" goto :project_error
if not exist "src\AITrace.cpp" goto :project_error
if not exist "src\AICommandTrace.cpp" goto :project_error
if not exist "src\AIQueueTrace.cpp" goto :project_error
if not exist "src\AICommandSiteTrace.cpp" goto :project_error
if not exist "src\AIProducerContextTrace.cpp" goto :project_error
if not exist "src\AIProducerContextTrace.h" goto :project_error
if not exist "src\AIParentFlowTrace.cpp" goto :project_error
if not exist "src\AIParentFlowTrace.h" goto :project_error
if not exist "src\dinput8.def" goto :project_error

if not exist "%WORKDIR%" mkdir "%WORKDIR%" >nul 2>&1
if not exist "%WORKDIR%" (
  echo ERROR: Cannot create temporary build directory: "%WORKDIR%"
  exit /b 1
)

>"%LOG%" echo ArcanaAIOverhaul Stage 11 build log
>>"%LOG%" echo Started: %DATE% %TIME%
>>"%LOG%" echo Project: %CD%
>>"%LOG%" echo Temporary work: %WORKDIR%
>>"%LOG%" echo.

echo [1/6] Checking project files...
>>"%LOG%" echo [1/6] Project files: OK

echo [2/6] Locating Visual Studio...
if not exist "%VSWHERE%" goto :toolchain_error
>>"%LOG%" echo [2/6] Using vswhere: %VSWHERE%
"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath >"%WORKDIR%\vsinstall.txt" 2>"%WORKDIR%\vswhere.err"
set "RC=%ERRORLEVEL%"
if exist "%WORKDIR%\vswhere.err" type "%WORKDIR%\vswhere.err" >>"%LOG%"
if not "%RC%"=="0" goto :toolchain_error
if not exist "%WORKDIR%\vsinstall.txt" goto :toolchain_error
set /p VSINSTALL=<"%WORKDIR%\vsinstall.txt"
if not defined VSINSTALL goto :toolchain_error
set "VCVARS=%VSINSTALL%\VC\Auxiliary\Build\vcvarsall.bat"
>>"%LOG%" echo VSINSTALL=%VSINSTALL%
>>"%LOG%" echo VCVARS=%VCVARS%
if not exist "%VCVARS%" goto :toolchain_error

echo [3/6] Initializing MSVC x86...
>>"%LOG%" echo [3/6] Calling vcvarsall x86. Its output is intentionally NOT redirected to Build.log.
call "%VCVARS%" x86
set "RC=%ERRORLEVEL%"
echo vcvarsall exit code: %RC%
>>"%LOG%" echo vcvarsall exit code: %RC%
if not "%RC%"=="0" goto :vcvars_error

echo [4/6] Checking compiler and linker...
where cl.exe >"%WORKDIR%\where_cl.txt" 2>&1
set "RC=%ERRORLEVEL%"
type "%WORKDIR%\where_cl.txt"
type "%WORKDIR%\where_cl.txt" >>"%LOG%"
if not "%RC%"=="0" goto :compiler_error
where link.exe >"%WORKDIR%\where_link.txt" 2>&1
set "RC=%ERRORLEVEL%"
type "%WORKDIR%\where_link.txt"
type "%WORKDIR%\where_link.txt" >>"%LOG%"
if not "%RC%"=="0" goto :linker_error
rem Do not invoke cl.exe /Bv without an input file. MSVC prints its
rem component versions but then returns D8003 (missing source file), which
rem falsely aborts this build before any real translation unit is compiled.
rem The where checks above prove that the x86 compiler and linker are on PATH.
>>"%LOG%" echo [4/6] Compiler and linker path checks passed.
>>"%LOG%" echo Compiler version will be validated by compiling the real source files.
echo Compiler and linker paths are valid; proceeding to real source compilation.

if not exist "obj" mkdir "obj" >nul 2>&1
if not exist "build" mkdir "build" >nul 2>&1
if not exist "obj" goto :directory_error
if not exist "build" goto :directory_error
del /q "obj\Proxy.obj" "obj\ComVsCom.obj" "obj\AITrace.obj" "obj\AICommandTrace.obj" "obj\AIQueueTrace.obj" "obj\AICommandSiteTrace.obj" "obj\AIProducerContextTrace.obj" "obj\AIParentFlowTrace.obj" >nul 2>&1
del /q "build\dinput8.dll" >nul 2>&1
set "CXXFLAGS=/nologo /std:c++17 /EHsc /O2 /MT /W4 /DWIN32 /D_WINDOWS /DUNICODE /D_UNICODE /D_WIN32_WINNT=0x0601"

echo [5/6] Compiling C++ sources...
>>"%LOG%" echo.
>>"%LOG%" echo [5/6] Compiling C++ sources

echo   - Proxy.cpp
>>"%LOG%" echo.
>>"%LOG%" echo ===== Compiling Proxy.cpp =====
cl.exe %CXXFLAGS% /c "src\Proxy.cpp" /Fo"obj\Proxy.obj" >"%WORKDIR%\Proxy.compile.txt" 2>&1
set "RC=%ERRORLEVEL%"
type "%WORKDIR%\Proxy.compile.txt"
type "%WORKDIR%\Proxy.compile.txt" >>"%LOG%"
if not "%RC%"=="0" goto :proxy_compile_error
if not exist "obj\Proxy.obj" goto :object_missing

echo   - ComVsCom.cpp
>>"%LOG%" echo.
>>"%LOG%" echo ===== Compiling ComVsCom.cpp =====
cl.exe %CXXFLAGS% /c "src\ComVsCom.cpp" /Fo"obj\ComVsCom.obj" >"%WORKDIR%\ComVsCom.compile.txt" 2>&1
set "RC=%ERRORLEVEL%"
type "%WORKDIR%\ComVsCom.compile.txt"
type "%WORKDIR%\ComVsCom.compile.txt" >>"%LOG%"
if not "%RC%"=="0" goto :com_compile_error
if not exist "obj\ComVsCom.obj" goto :object_missing

echo   - AITrace.cpp
>>"%LOG%" echo.
>>"%LOG%" echo ===== Compiling AITrace.cpp =====
cl.exe %CXXFLAGS% /c "src\AITrace.cpp" /Fo"obj\AITrace.obj" >"%WORKDIR%\AITrace.compile.txt" 2>&1
set "RC=%ERRORLEVEL%"
type "%WORKDIR%\AITrace.compile.txt"
type "%WORKDIR%\AITrace.compile.txt" >>"%LOG%"
if not "%RC%"=="0" goto :trace_compile_error
if not exist "obj\AITrace.obj" goto :object_missing

echo   - AICommandTrace.cpp
>>"%LOG%" echo.
>>"%LOG%" echo ===== Compiling AICommandTrace.cpp =====
cl.exe %CXXFLAGS% /c "src\AICommandTrace.cpp" /Fo"obj\AICommandTrace.obj" >"%WORKDIR%\AICommandTrace.compile.txt" 2>&1
set "RC=%ERRORLEVEL%"
type "%WORKDIR%\AICommandTrace.compile.txt"
type "%WORKDIR%\AICommandTrace.compile.txt" >>"%LOG%"
if not "%RC%"=="0" goto :command_trace_compile_error
if not exist "obj\AICommandTrace.obj" goto :object_missing

echo   - AIQueueTrace.cpp
>>"%LOG%" echo.
>>"%LOG%" echo ===== Compiling AIQueueTrace.cpp =====
cl.exe %CXXFLAGS% /Oy- /c "src\AIQueueTrace.cpp" /Fo"obj\AIQueueTrace.obj" >"%WORKDIR%\AIQueueTrace.compile.txt" 2>&1
set "RC=%ERRORLEVEL%"
type "%WORKDIR%\AIQueueTrace.compile.txt"
type "%WORKDIR%\AIQueueTrace.compile.txt" >>"%LOG%"
if not "%RC%"=="0" goto :queue_trace_compile_error
if not exist "obj\AIQueueTrace.obj" goto :object_missing

echo   - AICommandSiteTrace.cpp
>>"%LOG%" echo.
>>"%LOG%" echo ===== Compiling AICommandSiteTrace.cpp =====
cl.exe %CXXFLAGS% /c "src\AICommandSiteTrace.cpp" /Fo"obj\AICommandSiteTrace.obj" >"%WORKDIR%\AICommandSiteTrace.compile.txt" 2>&1
set "RC=%ERRORLEVEL%"
type "%WORKDIR%\AICommandSiteTrace.compile.txt"
type "%WORKDIR%\AICommandSiteTrace.compile.txt" >>"%LOG%"
if not "%RC%"=="0" goto :command_site_trace_compile_error
if not exist "obj\AICommandSiteTrace.obj" goto :object_missing

echo   - AIProducerContextTrace.cpp
>>"%LOG%" echo.
>>"%LOG%" echo ===== Compiling AIProducerContextTrace.cpp =====
cl.exe %CXXFLAGS% /c "src\AIProducerContextTrace.cpp" /Fo"obj\AIProducerContextTrace.obj" >"%WORKDIR%\AIProducerContextTrace.compile.txt" 2>&1
set "RC=%ERRORLEVEL%"
type "%WORKDIR%\AIProducerContextTrace.compile.txt"
type "%WORKDIR%\AIProducerContextTrace.compile.txt" >>"%LOG%"
if not "%RC%"=="0" goto :producer_context_compile_error
if not exist "obj\AIProducerContextTrace.obj" goto :object_missing

echo   - AIParentFlowTrace.cpp
>>"%LOG%" echo.
>>"%LOG%" echo ===== Compiling AIParentFlowTrace.cpp =====
cl.exe %CXXFLAGS% /c "src\AIParentFlowTrace.cpp" /Fo"obj\AIParentFlowTrace.obj" >"%WORKDIR%\AIParentFlowTrace.compile.txt" 2>&1
set "RC=%ERRORLEVEL%"
type "%WORKDIR%\AIParentFlowTrace.compile.txt"
type "%WORKDIR%\AIParentFlowTrace.compile.txt" >>"%LOG%"
if not "%RC%"=="0" goto :parent_flow_trace_compile_error
if not exist "obj\AIParentFlowTrace.obj" goto :object_missing

echo [6/6] Linking build\dinput8.dll...
>>"%LOG%" echo.
>>"%LOG%" echo [6/6] Linking build\dinput8.dll
link.exe /NOLOGO /DLL /DEF:"src\dinput8.def" /OUT:"build\dinput8.dll" /MACHINE:X86 /SUBSYSTEM:WINDOWS /INCREMENTAL:NO /DYNAMICBASE /NXCOMPAT "obj\Proxy.obj" "obj\ComVsCom.obj" "obj\AITrace.obj" "obj\AICommandTrace.obj" "obj\AIQueueTrace.obj" "obj\AICommandSiteTrace.obj" "obj\AIProducerContextTrace.obj" "obj\AIParentFlowTrace.obj" >"%WORKDIR%\link.txt" 2>&1
set "RC=%ERRORLEVEL%"
type "%WORKDIR%\link.txt"
type "%WORKDIR%\link.txt" >>"%LOG%"
if not "%RC%"=="0" goto :link_error
if not exist "build\dinput8.dll" goto :dll_missing
for %%F in ("build\dinput8.dll") do set "DLLSIZE=%%~zF"
if "%DLLSIZE%"=="0" goto :dll_missing
>>"%LOG%" echo.
>>"%LOG%" echo BUILD SUCCEEDED. DLL size: %DLLSIZE% bytes.
echo.
echo [SUCCESS] Created build\dinput8.dll (%DLLSIZE% bytes).
echo Log: Build.log
echo Copy build\dinput8.dll and ArcanaAI.ini beside AALib.exe.
echo Keep a backup of the existing working DLL.
goto :finish_success

:project_error
echo ERROR: One or more required source files are missing.
echo Expected: src\Proxy.cpp, src\ComVsCom.cpp, src\AITrace.cpp, src\AICommandTrace.cpp, src\AIQueueTrace.cpp, src\AICommandSiteTrace.cpp, src\AIProducerContextTrace.cpp, src\AIProducerContextTrace.h, src\AIParentFlowTrace.cpp, src\AIParentFlowTrace.h, src\dinput8.def
exit /b 1
:toolchain_error
>>"%LOG%" echo RESULT: VISUAL STUDIO TOOLCHAIN ERROR
:show_error
echo.
echo [ERROR] Build failed. Full log: "%LOG%"
if exist "%LOG%" type "%LOG%"
echo.
echo Temporary diagnostics: "%WORKDIR%"
pause
exit /b 1
:vcvars_error
>>"%LOG%" echo RESULT: VCVARS INITIALIZATION ERROR
 goto :show_error
:compiler_error
>>"%LOG%" echo RESULT: CL.EXE NOT AVAILABLE OR COMPILER VERSION CHECK FAILED
 goto :show_error
:linker_error
>>"%LOG%" echo RESULT: LINK.EXE NOT AVAILABLE
 goto :show_error
:directory_error
>>"%LOG%" echo RESULT: COULD NOT CREATE obj/build DIRECTORY
 goto :show_error
:proxy_compile_error
>>"%LOG%" echo RESULT: Proxy.cpp COMPILATION FAILED
 goto :show_error
:com_compile_error
>>"%LOG%" echo RESULT: ComVsCom.cpp COMPILATION FAILED
 goto :show_error
:trace_compile_error
>>"%LOG%" echo RESULT: AITrace.cpp COMPILATION FAILED
 goto :show_error
:command_trace_compile_error
>>"%LOG%" echo RESULT: AICommandTrace.cpp COMPILATION FAILED
 goto :show_error
:queue_trace_compile_error
>>"%LOG%" echo RESULT: AIQueueTrace.cpp COMPILATION FAILED
 goto :show_error
:command_site_trace_compile_error
>>"%LOG%" echo RESULT: AICommandSiteTrace.cpp COMPILATION FAILED
 goto :show_error
:producer_context_compile_error
>>"%LOG%" echo RESULT: AIProducerContextTrace.cpp COMPILATION FAILED
 goto :show_error
:parent_flow_trace_compile_error
>>"%LOG%" echo RESULT: AIParentFlowTrace.cpp COMPILATION FAILED
 goto :show_error
:object_missing
>>"%LOG%" echo RESULT: EXPECTED OBJ FILE WAS NOT CREATED
 goto :show_error
:link_error
>>"%LOG%" echo RESULT: DLL LINK FAILED
 goto :show_error
:dll_missing
>>"%LOG%" echo RESULT: DLL NOT CREATED OR EMPTY
 goto :show_error

:finish_success
rmdir /s /q "%WORKDIR%" >nul 2>&1
echo.
pause
exit /b 0
