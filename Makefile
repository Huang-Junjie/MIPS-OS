include ./tools/include.mk

modules := drivers init lib trap pmm vmm
kernel := kernel
swapimg := swap.img
kernel_lds := tools/kernel.lds

objects :=  init/*.o	\
			lib/*.o	 \
			trap/*.o	\
			pmm/*.o		\
			vmm/*.o		\
			drivers/gxconsole/*.o \
			drivers/gxclock/*.o \
			drivers/gxide/*.o \



.PHONY: all $(modules) clean start debug

all: $(modules) $(kernel)

$(modules):
	$(MAKE) --directory=$@

$(kernel): $(modules)
	$(LD) $(objects) -T $(kernel_lds) -o $(kernel)


clean:
	for d in $(modules);	  \
		do					\
			$(MAKE) --directory=$$d clean; \
		done; \
	rm -rf $(kernel)

start: $(kernel)
	gxemul -E oldtestmips -C 4Kc -M 64 -d 1:$(swapimg) $(kernel)

debug: $(kernel)
	gxemul -E oldtestmips -C 4Kc -M 64 -d 1:$(swapimg) $(kernel) -V
