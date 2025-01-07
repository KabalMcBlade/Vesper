@echo off
echo:
echo *************************
echo *      COPY ASSETS      *
echo *************************
echo:

if "%~1"=="" goto :END_1
if "%~2"=="" goto :END_2

set SOURCE=%~1
set DEST=%~2

if not exist "%DEST%Assets\Shaders" (
    echo Folder "%DEST%Assets\Shaders" not found, creating it.
    mkdir "%DEST%Assets\Shaders"
)

if not exist "%DEST%Assets\Models" (
    echo Folder "%DEST%Assets\Models" not found, creating it.
    mkdir "%DEST%Assets\Models"
)

if not exist "%DEST%Assets\Textures" (
    echo Folder "%DEST%Assets\Textures" not found, creating it.
    mkdir "%DEST%Assets\Textures"
)

echo:
echo Copying shader files from "%SOURCE%Assets\Shaders" to "%DEST%Assets\Shaders":
copy "%SOURCE%Assets\Shaders\*.spv" "%DEST%Assets\Shaders\"

echo:
echo Copying model files from "%SOURCE%Assets\Models" to "%DEST%Assets\Models":
copy "%SOURCE%Assets\Models\*.*" "%DEST%Assets\Models\"

echo:
echo Copying texture files from "%SOURCE%Assets\Textures" to "%DEST%Assets\Textures":
copy "%SOURCE%Assets\Textures\*.*" "%DEST%Assets\Textures\"

goto :END_OK

:END_1
echo Error: No source directory provided as a parameter!
goto :END

:END_2
echo Error: No destination directory provided as a parameter!
goto :END

:END_OK
echo Copy operation completed successfully!
goto :END

:END
