@echo off
echo:
echo *************************
echo *      COPY ASSETS      *
echo *************************
echo:

if "%~1"=="" goto :END_1
if "%~2"=="" goto :END_2

if not exist %~2Assets\Shaders (
    echo Folder %~2Assets\Shaders not found, creating it.
    mkdir "%~2Assets\Shaders"
)

if not exist %~2Assets\Models (
    echo Folder %~2Assets\Models not found, creating it.
    mkdir "%~2Assets\Models"
)

if not exist %~2Assets\Textures (
    echo Folder %~2Assets\Textures not found, creating it.
    mkdir "%~2Assets\Textures"
)
echo:

for %%f in (%~1Assets\Shaders\*.spv) do (

    echo Copying shaders files:
    copy %~1Assets\Shaders\*.spv %~2Assets\Shaders
    echo:

	goto :MODEL_FOLDER
)

:MODEL_FOLDER
for %%f in (%~1Assets\Models\*.*) do (

    echo Copying models files:
    copy %~1Assets\Models\*.* %~2Assets\Models
    echo:

	goto :TEXTURE_FOLDER
)

:TEXTURE_FOLDER
for %%f in (%~1Assets\Textures\*.*) do (

    echo Copying textures files:
    copy %~1Assets\Textures\*.* %~2Assets\Textures
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