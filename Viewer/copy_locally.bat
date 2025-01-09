@echo off
echo:
echo *************************
echo *      COPY ASSETS      *
echo *************************
echo:

if "%~1"=="" goto :END_1
if "%~2"=="" goto :END_2
if "%~3"=="" goto :END_3

if not exist "%~2Assets\Shaders" (
    echo Folder %~2Assets\Shaders not found, creating it.
    mkdir "%~2Assets\Shaders"
)

if not exist "%~2Assets\Models" (
    echo Folder %~2Assets\Models not found, creating it.
    mkdir "%~2Assets\Models"
)

if not exist "%~2Assets\Textures" (
    echo Folder %~2Assets\Textures not found, creating it.
    mkdir "%~2Assets\Textures"
)
echo:

echo Copying shader files from %~1Assets\Shaders\ to %~2Assets\Shaders\:
copy "%~1Assets\Shaders\*.spv" "%~2Assets\Shaders\"
echo:

echo Copying model files from %~1Assets\Models\ to %~2Assets\Models\:
copy "%~1Assets\Models\*.*" "%~2Assets\Models\"
echo:

echo Copying texture files from %~1Assets\Textures\ to %~2Assets\Textures\:
copy "%~1Assets\Textures\*.*" "%~2Assets\Textures\"
echo:

:: Create the timestamp file here if everything is successful
echo. > "%~3copy_locally_build.timestamp"

goto :END_OK

:END_1
echo No $(ProjectDir) provided as parameter!
exit /b 1

:END_2
echo No $(OutDir) provided as parameter!
exit /b 1

:END_3
echo No $(IntDirFullPath) provided as parameter!
exit /b 1

:END_OK
echo Copy operation completed successfully!
exit /b 0
