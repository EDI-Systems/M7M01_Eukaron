#!/bin/bash
set -e

kernel="${RME_KERNEL:-./Object/RME}"
pc_dir=/usr/lib/grub/i386-pc
efi_dir="${GRUB_EFI_DIR:-/usr/lib/grub/x86_64-efi}"
grub_args=()

case "${1:-hybrid}" in
    hybrid)
        iso=RME.iso
        asm=RME.asm
        if [[ ! -f "$pc_dir/multiboot2.mod" || ! -f /usr/lib/grub/x86_64-efi/multiboot2.mod ]] ||
           ! command -v mformat >/dev/null; then
            printf '%s\n' 'Dual-mode ISO requires grub-pc-bin, grub-efi-amd64-bin and mtools.' >&2
            exit 1
        fi
        # With no -d, grub-mkrescue includes both installed platforms.
        ;;
    bios)
        iso=RME.iso
        asm=RME.asm
        grub_args=(-d "$pc_dir")
        ;;
    uefi)
        iso=RME-uefi.iso
        asm=
        if [[ ! -f "$efi_dir/modinfo.sh" ]] || ! command -v mformat >/dev/null; then
            printf '%s\n' 'UEFI ISO requires grub-efi-amd64-bin and mtools.' >&2
            exit 1
        fi
        grub_args=(-d "$efi_dir")
        ;;
    *)
        printf 'Usage: %s [hybrid|bios|uefi]\n' "$0" >&2
        exit 2
        ;;
esac

if ! grub-file --is-x86-multiboot2 "$kernel"; then
    printf 'Invalid Multiboot2 kernel: %s\n' "$kernel" >&2
    exit 1
fi

# Stage the current kernel and config without touching any previous build files.
grub-mkrescue "${grub_args[@]}" -o "${RME_ISO:-$iso}" -graft-points \
    "/boot/RME=$kernel" "/boot/grub/grub.cfg=isofiles/boot/grub/grub.cfg"
if [[ -n "$asm" ]]; then
    objdump -S "$kernel" > "${RME_ASM:-$asm}"
fi
