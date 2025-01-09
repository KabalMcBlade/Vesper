@echo off
echo:
echo *****************************
echo *   COMPILE SPIR-V SHADER   *
echo *****************************
echo:

if "%~1"=="" goto :END_1
if "%~2"=="" goto :END_2

::- Select the Vulkan platform: Bin32 (32-bit) or Bin (64-bit). 
::- We are using 64-bit on this engine, so go for Bin!
set VK_PLATFORM=Bin

:: Set the path to the flag configuration file
set CONFIG_FILE=%~1compile_shaders_config.txt

:: Check if the configuration file exists
if not exist "%CONFIG_FILE%" (
    echo Configuration file not found: %CONFIG_FILE%
    goto :END_1
)

echo Compiling vertex shader files:
for %%f in ("%~1Assets\Shaders\*.vert") do (
    call :COMPILE_SHADER "%%f" "vertex"
)
echo:

echo Compiling fragment shader files:
for %%f in ("%~1Assets\Shaders\*.frag") do (
    call :COMPILE_SHADER "%%f" "fragment"
)

:: Create the timestamp file here if everything is successful
echo. > "%~2compile_shaders_build.timestamp"

goto :END_OK

:COMPILE_SHADER
setlocal enabledelayedexpansion
set SHADER_FILE=%~1
set SHADER_TYPE=%~2

:: Extract matching configuration for the current shader
set FLAGS=
set VALUES=

for /f "tokens=1,* delims= " %%a in ('findstr /i /r "^%~nx1 " "%CONFIG_FILE%"') do (
    set "LINE=%%b"
    for /f "tokens=1 delims= " %%x in ("!LINE!") do set "FLAGS=%%x" & set "LINE=!LINE:%%x=!"
    set "VALUES=!LINE!"
)

:: If no flags are defined, compile normally
if "%FLAGS%"=="" (
    echo Compiling %SHADER_TYPE% shader: "%SHADER_FILE%" with no flags
    "%VULKAN_SDK%\%VK_PLATFORM%\glslangValidator.exe" -V -o "%~dpnx1.spv" "%SHADER_FILE%"
) else (
    echo DEBUG: Flags for %~nx1: %FLAGS%
    echo DEBUG: Values for %~nx1: %VALUES%
    for %%f in (%FLAGS%) do (
        call :TOLOWER %%f lowercaseFlag
        if defined lowercaseFlag (
            echo Debug: Original=%%f, Lowercase=!lowercaseFlag!
            for %%v in (!VALUES!) do (
                set OUTPUT_FILE=%~dpn1_!lowercaseFlag!%%v%~x1.spv
                echo Compiling %SHADER_TYPE% shader: "%SHADER_FILE%" with flag: %%f=%%v
                "%VULKAN_SDK%\%VK_PLATFORM%\glslangValidator.exe" --D %%f=%%v -V -o "!OUTPUT_FILE!" "%SHADER_FILE%"
            )
        ) else (
            echo ERROR: Failed to process flag %%f for %~nx1
        )
    )
)

endlocal
exit /b

:: Subroutine for converting strings to lowercase
:TOLOWER
@echo off
setlocal enabledelayedexpansion
set "lowercaseFlag="
for /f "delims=" %%a in ('powershell -Command "'%~1'.ToLower()" 2^>nul') do set "lowercaseFlag=%%a"
endlocal & set "%~2=%lowercaseFlag%"
exit /b

:END_1
echo No $(ProjectDir) provided as parameter!
goto :END

:END_2
echo No $(IntDirFullPath) provided as parameter!
goto :END

:END_OK
echo Shader compilation complete.

:END
