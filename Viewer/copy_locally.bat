@echo off
echo:
echo *************************
echo *      COPY ASSETS      *
echo *************************
echo:

if "%~1"=="" goto :END_1
if "%~2"=="" goto :END_2

if not exist "%~1Assets\Shaders" (
    echo Folder %~1Assets\Shaders not found, creating it.
    mkdir "%~1Assets\Shaders"
)

if not exist "%~1Assets\Models" (
    echo Folder %~1Assets\Models not found, creating it.
    mkdir "%~1Assets\Models"
)

if not exist "%~1Assets\Textures" (
    echo Folder %~1Assets\Textures not found, creating it.
    mkdir "%~1Assets\Textures"
)
echo:

echo Copying shader files from %~2Assets\Shaders\ to %~1Assets\Shaders\:
copy "%~2Assets\Shaders\*.spv" "%~1Assets\Shaders\"
echo:

echo Copying model files from %~2Assets\Models\ to %~1Assets\Models\:
copy "%~2Assets\Models\*.*" "%~1Assets\Models\"
echo:

echo Copying texture files from %~2Assets\Textures\ to %~1Assets\Textures\:
copy "%~2Assets\Textures\*.*" "%~1Assets\Textures\"
echo:

goto :END_OK

:END_1
echo No $(ProjectDir) provided as parameter!
exit /b 1

:END_2
echo No $(OutDir) provided as parameter!
exit /b 1

:END_OK
echo Copy operation completed successfully!
exit /b 0
