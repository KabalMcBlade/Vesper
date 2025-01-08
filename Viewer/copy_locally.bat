@echo off
echo:
echo *************************
echo *      COPY ASSETS      *
echo *************************
echo:

if "%~1"=="" goto :END_1
if "%~2"=="" goto :END_2
if "%~3"=="" goto :END_3
if "%~4"=="" goto :END_4

if not exist "%~3Assets\%~1" (
    echo Folder %~3Assets\%~1 not found, creating it.
    mkdir "%~3Assets\%~1"
)

echo:

echo Copying assets files from %~2Assets\%~1\ to %~3Assets\%~1\:
copy "%~2Assets\%~1\*.*" "%~3Assets\%~1\"
echo:

:: Create the timestamp file here if everything is successful
echo. > "%~4copy_%~1_build.timestamp"

goto :END_OK

:END_1
echo No asset folder provided as parameter!
exit /b 1

:END_2
echo No $(ProjectDir) provided as parameter!
exit /b 1

:END_3
echo No $(OutDir) provided as parameter!
exit /b 1

:END_4
echo No $(IntDirFullPath) provided as parameter!
exit /b 1

:END_OK
echo Copy operation completed successfully!
exit /b 0
