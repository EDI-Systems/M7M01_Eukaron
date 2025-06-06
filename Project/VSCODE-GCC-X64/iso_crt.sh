cp -f ./Object/RME ./isofiles/boot/
grub-mkrescue -o RME.iso isofiles
objdump -S ./Object/RME > RME.asm
