@echo off
set UV=C:\Keil_v5\UV4\UV4.exe
set UV_PRO_PATH=D:\0.project\12.CODE_MCU_Stm32F407\stm32_code_projector_commercial\workspace\Simu_STM32F407_UDP_FAN_Temp\MDK-ARM\PJMainControlMCU.uvprojx
echo Start building ...
echo .>autobuild_log.txt
rem %UV% -j0 -b %UV_PRO_PATH% -t"Release" -o %cd%\autobuild_log.txt
%UV% -j0 %UV_PRO_PATH% -t"Release" -o %cd%\autobuild_log.txt
type autobuild_log.txt
echo Done.