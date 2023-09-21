@echo off
echo:
echo *****************************
echo *   COMPILE SPIR-V SHADER   *
echo *****************************
echo:

::-Select the Vulkan platform is Bin32 (for 32 bit) or Bin (for 64 bit) only!
::-But we are using 64 bit on this engine, so go for Bin!
set VK_PLATFORM=Bin

echo Compiling vertex shader files:
for %%f in (Assets\Shaders\*.vert) do (
	%VK_SDK_PATH%\%VK_PLATFORM%\glslangValidator.exe -V -o %%f.spv %%f
)
echo:

echo Compiling fragment shader files:
for %%f in (Assets\Shaders\*.frag) do (
	%VK_SDK_PATH%\%VK_PLATFORM%\glslangValidator.exe -V -o %%f.spv %%f"
)

::exit