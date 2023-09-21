@echo off
echo:
echo *************************
echo *      COPY ASSETS      *
echo *************************
echo:

if "%~1"=="" goto :END_1
if "%~2"=="" goto :END_2

if not exist %~1Assets\Shaders (
    echo Folder %~1Assets\Shaders not found, creating it.
    mkdir "%~1Assets\Shaders"
)

if not exist %~1Assets\Models (
    echo Folder %~1Assets\Models not found, creating it.
    mkdir "%~1Assets\Models"
)

if not exist %~1Assets\Textures (
    echo Folder %~1Assets\Textures not found, creating it.
    mkdir "%~1Assets\Textures"
)
echo:


echo Copying shaders files:
copy %~2\Assets\Shaders\*.spv %~1Assets\Shaders
echo:

::echo Copying models files:
::copy %~2\Assets\Models %~1Assets\Models
::echo:

::echo Copying textures files:
::copy %~2\Assets\Textures %~1Assets\Textures
::echo:

goto :END_OK


:END_1
echo No $(ProjectDir) provided as parameter!

:END_1
echo No $(OutDir) provided as parameter!


:END_OK

::exit