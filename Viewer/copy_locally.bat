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

for %%f in (%~2Assets\Shaders\*.spv) do (

    echo Copying shaders files from %~2Assets\Shaders\ to %~1Assets\Shaders\:
    copy %~2Assets\Shaders\*.spv %~1Assets\Shaders
    echo:

	goto :MODEL_FOLDER
)

:MODEL_FOLDER
for %%f in (%~2Assets\Models\*.*) do (

    echo Copying models files from %~2Assets\Models\ to %~1Assets\Models\:
    copy %~2Assets\Models\*.* %~1Assets\Models
    echo:

	goto :TEXTURE_FOLDER
)

:TEXTURE_FOLDER
for %%f in (%~2Assets\Textures\*.*) do (

    echo Copying textures files from %~2Assets\Textures\ to %~1Assets\Textures\:
    copy %~2Assets\Textures\*.* %~1Assets\Textures
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