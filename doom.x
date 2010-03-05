/*
 * Muistikartta
 */
#include "../bootloader/system.h"
OUTPUT_FORMAT( "elf32-littlenios2" )
OUTPUT_ARCH( nios2 )
ENTRY( _start )

SECTIONS
{
	.ram RAM :
	{
		*(.text)
		*(.rodata)
		*(.rodata.*)
		*(.data)
	}

	.bss : {
		*(.bss)
		*(COMMON)
	}

	/DISCARD/ : {
		*(.comment)
	}
}
