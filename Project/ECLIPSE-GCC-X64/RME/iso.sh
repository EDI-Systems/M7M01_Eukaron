cp -f RME isofiles/boot/RME
grub-mkrescue -o os.iso isofiles
objdump -S RME > RME.asm
