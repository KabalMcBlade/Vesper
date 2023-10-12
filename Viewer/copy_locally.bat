@echo off
echo:
echo *************************
echo *      COPY ASSETS      *
echo *************************
echo:

if "%~1"=="" goto :END_1
if "%~2"=="" goto :END_2

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


for %%f in (%~2\Assets\Shaders\*.spv) do (

    echo Copying shaders files:
    copy %~2\Assets\Shaders\*.spv %~1\Assets\Shaders
    echo:

	goto :MODEL_FOLDER
)

:MODEL_FOLDER
for %%f in (%~2\Assets\Models\*.*) do (

    echo Copying models files:
    copy %~2\Assets\Models\*.* %~1\Assets\Models
    echo:

	goto :TEXTURE_FOLDER
)

:TEXTURE_FOLDER
for %%f in (%~2\Assets\Textures\*.*) do (

    echo Copying textures files:
    copy %~2\Assets\Textures\*.* %~1\Assets\Textures
    echo:

	goto :END_OK
)


goto :END_OK


:END_1
echo No $(ProjectDir) provided as parameter!

:END_1
echo No $(OutDir) provided as parameter!


:END_OK

::exit