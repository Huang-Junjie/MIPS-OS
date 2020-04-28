include ./tools/include.mk

modules := drivers init lib trap pmm vmm process schedule sync user
kernel := kernel
swapimg := swap.img
kernel_lds := tools/kernel.lds
user_lds := tools/user.lds

objects :=  init/*.o	\
			lib/*.o	 \
			trap/*.o	\
			pmm/*.o		\
			vmm/*.o		\
			process/*.o		\
			schedule/*.o		\
			sync/*.o		\
			drivers/gxconsole/*.o \
			drivers/gxclock/*.o \
			drivers/gxide/*.o 

user_objects := user/*.o \
				user/lib/*.o

user_bin	:= user/user_code.out
user_bin_name   := _binary_user_user_code_out

.PHONY: all $(modules) clean start debug

all: $(modules) $(kernel)

$(modules):
	$(MAKE) --directory=$@ "DEFS+=-DUSERSTART=$(user_bin_name)_start -DUSERSIZE=$(user_bin_name)_size"

$(kernel): $(modules)
	$(LD) -nostdlib -T $(user_lds) -o $(user_bin) $(user_objects)
	$(LD) -nostdlib -T $(kernel_lds) -o $(kernel) $(objects) -b binary $(user_bin)




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
