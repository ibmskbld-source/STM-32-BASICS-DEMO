# PowerShell script to build and flash Single Digit 7-Segment Demo to STM32G431 board via ST-LINK
$ST_IDE = "C:\ST\STM32CubeIDE_2.2.0\STM32CubeIDE\plugins"
$GCC = "$ST_IDE\com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.14.3.rel1.win32_1.0.100.202602081740\tools\bin\arm-none-eabi-gcc.exe"
$OBJCOPY = "$ST_IDE\com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.14.3.rel1.win32_1.0.100.202602081740\tools\bin\arm-none-eabi-objcopy.exe"
$CLI = "$ST_IDE\com.st.stm32cube.ide.mcu.externaltools.cubeprogrammer.win32_2.2.500.202603051304\tools\bin\STM32_Programmer_CLI.exe"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

Write-Host "[1/3] Compiling Single Digit 7-Segment firmware..." -ForegroundColor Cyan
& $GCC -mcpu=cortex-m4 -mthumb -mfloat-abi=soft -O1 -T "$ScriptDir\linker.ld" --specs=nano.specs --specs=nosys.specs -nostartfiles -o "$ScriptDir\seven_seg.elf" "$ScriptDir\startup.c" "$ScriptDir\main_direct.c"

if ($LASTEXITCODE -ne 0) {
    Write-Host "Compilation failed!" -ForegroundColor Red
    exit 1
}

Write-Host "[2/3] Generating raw binary..." -ForegroundColor Cyan
& $OBJCOPY -O binary "$ScriptDir\seven_seg.elf" "$ScriptDir\seven_seg.bin"

Write-Host "[3/3] Flashing STM32G431 via ST-LINK..." -ForegroundColor Cyan
& $CLI -c port=SWD freq=4000 mode=HOTPLUG -e all -w "$ScriptDir\seven_seg.bin" 0x08000000 -v -rst

if ($LASTEXITCODE -eq 0) {
    Write-Host "`nSUCCESS: Single Digit 7-Segment Counter flashed and running!" -ForegroundColor Green
} else {
    Write-Host "`nERROR: Flashing failed!" -ForegroundColor Red
}
