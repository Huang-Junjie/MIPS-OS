OUTPUT_FORMAT("elf32-tradbigmips")
OUTPUT_ARCH(mips)
ENTRY(umain)

SECTIONS
{

  . = 0x400000;
  .text : {
    *(.text)
  }

  .rodata : {
    *(.rodata .rodata.*)
  }

  .data : {
    *(.data)
  }

  .sdata : {
    *(.sdata)
  }


  .bss  : {
    *(.bss)
  }

}
