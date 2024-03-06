PREFIX         = v810
LIBERIS        = $(HOME)/devel/liberis
V810GCC        = $(HOME)/devel/pcfx/bin/v810-gcc

ASFLAGS        = -a=$*.lst
# CFLAGS        += -I$(LIBERIS)/include/ -I$(V810GCC)/include/ -I$(V810GCC)/$(PREFIX)/include/ -O2 -Wall -std=gnu99 -mv810 -msda=256 -mprolog-function
CFLAGS        += -I$(LIBERIS)/include/ -I$(V810GCC)/include/ -I$(V810GCC)/$(PREFIX)/include/ -Wall -std=gnu99 -mv810 -msda=256 -mprolog-function
CPFLAGS       += -I$(LIBERIS)/include/ -I$(V810GCC)/include/ -I$(V810GCC)/$(PREFIX)/include/ -O2 -Wall -std=gnu++11 -fno-rtti -fno-exceptions -mv810 -msda=256 -mprolog-function
LDFLAGS       += -T$(LIBERIS)/ldscripts/v810.x -L$(LIBERIS)/ -L$(V810GCC)/lib/ -L$(V810GCC)/$(PREFIX)/lib/ -L$(V810GCC)/lib/gcc/$(PREFIX)/4.7.4/ $(V810GCC)/$(PREFIX)/lib/crt0.o

LIBS           = -leris -lc -lsim -lgcc

ctrlr.cue: cdlink_ctrlr.txt ctrlr
	pcfx-cdlink cdlink_ctrlr.txt ctrlr

ctrlr.flashboot: ctrlr
	python3 mkflashboot.py ctrlr

ctrlr: ctrlr.o font.o
	v810-ld $(LDFLAGS) ctrlr.o font.o $(LIBS) --sort-common=descending -o ctrlr.linked -Map ctrlr.map
	v810-objcopy -O binary ctrlr.linked ctrlr

font.o: font.s
	v810-as $(ASFLAGS) font.s -o font.o

ctrlr.o: ctrlr.source
	v810-as $(ASFLAGS) ctrlr.source -o ctrlr.o

%.o: %.s
	v810-as $(ASFLAGS) $< -o $@

ctrlr.source: ctrlr.c
	v810-gcc $(CFLAGS) ctrlr.c -S -o ctrlr.source

%.elf: $(OBJECTS)
	v810-ld $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@ -Map $*.map

%.bin: %.elf
	v810-objcopy -O binary $< $@

lbas.h:
	bincat out.bin lbas.h $(BIN_TARGET) $(ADD_FILES)

clean:
	rm -rf ctrlr *.o *.source *.map *.lst *.linked lbas.h out.bin ctrlr.bin ctrlr.cue
