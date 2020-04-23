OUTPUT_FORMAT("elf32-tradbigmips")
OUTPUT_ARCH(mips)
ENTRY()

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
