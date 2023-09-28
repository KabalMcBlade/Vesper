@echo off
echo:
echo *************************
echo *      COPY ASSETS      *
echo *************************
echo:

if "%~1"=="" goto :END_1
if "%~2"=="" goto :END_2

if not exist %~2\Assets\Shaders (
    echo Folder %~2\Assets\Shaders not found, creating it.
    mkdir "%~2\Assets\Shaders"
)

if not exist %~2\Assets\Models (
    echo Folder %~2\Assets\Models not found, creating it.
    mkdir "%~2\Assets\Models"
)

if not exist %~2\Assets\Textures (
    echo Folder %~2\Assets\Textures not found, creating it.
    mkdir "%~2\Assets\Textures"
)
echo:


echo Copying shaders files:
copy %~1\Assets\Shaders\*.spv %~2\Assets\Shaders
echo:

::echo Copying models files:
::copy Assets\Models %~2\Assets\Models
::echo:

::echo Copying textures files:
::copy Assets\Textures %~2\Assets\Textures
::echo:

goto :END_OK


:END_1
echo No $(ProjectDir) provided as parameter!

:END_2
echo No $(OutDir) provided as parameter!


:END_OK

::exit