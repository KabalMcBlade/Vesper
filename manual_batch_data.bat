@echo off

:: Debug or Release
set PLATFORM=x64
set CONFIGURATION=Debug

set SOLUTION_PATH=C:\Projects\Vesper

set PROJECT_NAME=Viewer

set OUT_DESTINATION=%SOLUTION_PATH%\%PLATFORM%\%CONFIGURATION%\	
set OUT_VIEWER_PATH=%SOLUTION_PATH%\%PROJECT_NAME%\

call %SOLUTION_PATH%\VesperEngine\compile_shaders.bat %SOLUTION_PATH%\VesperEngine\
call %SOLUTION_PATH%\VesperEngine\copy_assets.bat %SOLUTION_PATH%\VesperEngine\ %OUT_DESTINATION%
call %SOLUTION_PATH%\%PROJECT_NAME%\copy_locally.bat %OUT_VIEWER_PATH% %OUT_DESTINATION%
