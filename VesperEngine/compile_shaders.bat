@echo off
echo:
echo *****************************
echo *   COMPILE SPIR-V SHADER   *
echo *****************************
echo:

if "%~1"=="" goto :END_1

::-Select the Vulkan platform is Bin32 (for 32 bit) or Bin (for 64 bit) only!
::-But we are using 64 bit on this engine, so go for Bin!
set VK_PLATFORM=Bin

echo Compiling vertex shader files:
for %%f in (%~1Assets\Shaders\*.vert) do (
	%VULKAN_SDK%\%VK_PLATFORM%\glslangValidator.exe -V -o %%f.spv %%f
)
echo:

echo Compiling fragment shader files:
for %%f in (%~1Assets\Shaders\*.frag) do (
	%VULKAN_SDK%\%VK_PLATFORM%\glslangValidator.exe -V -o %%f.spv %%f"
)

goto :END_OK


:END_1
echo No $(ProjectDir) provided as parameter!

:END_OK

::exit