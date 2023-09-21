@echo off
echo:
echo *************************
echo *      COPY ASSETS      *
echo *************************
echo:

if "%~1"=="" goto :END_1

if not exist %~1\Assets\Shaders (
    echo Folder %~1\Assets\Shaders not found, creating it.
    mkdir "%~1\Assets\Shaders"
)

if not exist %~1\Assets\Models (
    echo Folder %~1\Assets\Models not found, creating it.
    mkdir "%~1\Assets\Models"
)

if not exist %~1\Assets\Textures (
    echo Folder %~1\Assets\Textures not found, creating it.
    mkdir "%~1\Assets\Textures"
)
echo:


echo Copying shaders files:
copy Assets\Shaders\*.spv %~1\Assets\Shaders
echo:

::echo Copying models files:
::copy Assets\Models %~1\Assets\Models
::echo:

::echo Copying textures files:
::copy Assets\Textures %~1\Assets\Textures
::echo:

goto :END_OK


:END_1
echo No $(OutDir) provided as parameter!


:END_OK

::exit