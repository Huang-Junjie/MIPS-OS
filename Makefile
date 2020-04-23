include ./tools/include.mk

modules := drivers init lib trap pmm vmm process
kernel := kernel
swapimg := swap.img
kernel_lds := tools/kernel.lds

objects :=  init/*.o	\
			lib/*.o	 \
			trap/*.o	\
			pmm/*.o		\
			vmm/*.o		\
			process/*.o		\
			drivers/gxconsole/*.o \
			drivers/gxclock/*.o \
			drivers/gxide/*.o \



.PHONY: all $(modules) clean start debug

all: $(modules) $(kernel)

$(modules):
	$(MAKE) --directory=$@

$(kernel): $(modules)
	# $(LD) -nostdlib -T $(user_lds) -o hello.out user/hello.o
	# $(LD) -nostdlib -T $(kernel_lds) -o $(kernel) $(objects) -b binary hello.out
	$(LD) -nostdlib -T $(kernel_lds) -o $(kernel) $(objects)


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
