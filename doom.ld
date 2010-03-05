








OUTPUT_FORMAT( "elf32-littlenios2" )
OUTPUT_ARCH( nios2 )
ENTRY( _start )

SECTIONS
{
 .ram 0x00800000 :
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
