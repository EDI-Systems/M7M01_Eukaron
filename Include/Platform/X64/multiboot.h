/*  multiboot.h - Multiboot2 boot information used by RME.  */
/*  Copyright (C) 1999,2003,2007,2008,2009,2010  Free Software Foundation, Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ANY
 *  DEVELOPER OR DISTRIBUTOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 *  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef MULTIBOOT_HEADER
#define MULTIBOOT_HEADER 1

#define MULTIBOOT_TAG_ALIGN             8U
#define MULTIBOOT_TAG_END               0U
#define MULTIBOOT_TAG_MMAP              6U
#define MULTIBOOT_TAG_FRAMEBUFFER       8U
#define MULTIBOOT_TAG_EFI64             12U
#define MULTIBOOT_TAG_ACPI_OLD          14U
#define MULTIBOOT_TAG_ACPI_NEW          15U

#define MULTIBOOT_MEMORY_AVAILABLE      1U

#define MULTIBOOT_FRAMEBUFFER_RGB       1U
#define MULTIBOOT_FRAMEBUFFER_EGA_TEXT  2U

struct multiboot_info
{
    unsigned int total_size;
    unsigned int reserved;
};

struct multiboot_tag
{
    unsigned int type;
    unsigned int size;
};

struct multiboot_mmap_entry
{
    unsigned long long addr;
    unsigned long long len;
    unsigned int type;
    unsigned int reserved;
} __attribute__((__packed__));

struct multiboot_tag_mmap
{
    struct multiboot_tag tag;
    unsigned int entry_size;
    unsigned int entry_version;
    struct multiboot_mmap_entry entries[];
};

struct multiboot_tag_framebuffer
{
    struct multiboot_tag tag;
    unsigned long long addr;
    unsigned int pitch;
    unsigned int width;
    unsigned int height;
    unsigned char bpp;
    unsigned char type;
    unsigned short reserved;
};

struct multiboot_framebuffer_rgb
{
    unsigned char red_position, red_size;
    unsigned char green_position, green_size;
    unsigned char blue_position, blue_size;
};

#endif
