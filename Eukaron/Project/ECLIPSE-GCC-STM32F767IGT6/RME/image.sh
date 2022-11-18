arm-none-eabi-objcopy -O binary RME RME.bin
arm-none-eabi-objdump -S RME > RME.asm
# Try to flash this in with st-flash
st-flash write RME.bin 0x08000000
