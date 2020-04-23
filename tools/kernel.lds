OUTPUT_FORMAT("elf32-tradbigmips")
ENTRY(_start)

SECTIONS
{
  . = 0x80000000;
  .text.exc_tlb : {
    *(.text.exc_tlb)
  }

  . = 0x80000180;
  .text.exc_vec : {
    *(.text.exc_vec)
  }

  . = 0x80010000;
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

  . = 0x80400000;

  end = . ;
}
