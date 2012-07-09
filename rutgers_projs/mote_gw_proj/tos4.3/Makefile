AS     = avr-as
LD     = avr-gcc
CC     = avr-gcc 
FULLCC = gcc
OBJCOPY= avr-objcopy
LOCAL_GROUP = 0xa
SFLAGS = -S -g -Os -I. -I./system/include -mmcu=at90s8535 -DLOCAL_GROUP=$(LOCAL_GROUP)
CFLAGS = -c -Os -Wall -I. -I./system/include -mmcu=at90s8535 -DLOCAL_GROUP=$(LOCAL_GROUP)
TOOLS  = ./tools

#------------------------------------------------------
# Set DESC to point to the structural description
# of the graph of components.  The C files associated
# with those components and the interconnections are generated
# from the .desc description
#--------------------------------------


# Simple blinking LED example
#DESC = apps/connect.desc
#DESC = apps/connect_base.desc
#DESC = apps/blink.desc
#DESC = apps/wave.desc
#DESC = apps/sens_to_leds.desc
#DESC = apps/cnt_to_leds.desc
#DESC = apps/cnt_to_rfm.desc
#DESC = apps/rfm_to_leds.desc
#DESC = apps/cnt_to_leds_and_rfm.desc
#DESC = apps/sens_to_rfm.desc
#DESC = apps/sens_to_leds_and_rfm.desc
#DESC = apps/chirp.desc
#DESC = apps/uart_chirp.desc
#DESC = apps/mags.desc
#DESC = apps/LOGGER_TEST.desc
#DESC = apps/generic_base.desc
#DESC = apps/router.desc
#DESC = apps/router_base.desc
DESC = apps/MOTE_GW.desc

#CFLAGS += -DBASE_STATION

#--------------------------------------------
# Remainder is common to all mote applications
#-------------------------------------------

LIBS   =
PROGRAMER= uisp -v=3
WIN_PROGRAMER= ./tools/mote_prog.exe
SET_ID=$(TOOLS)/munge_srec
all: Makefile.objs super.h am_dispatch.h main.exe

super.h: $(DESC)
	$(TOOLS)/mksuper_desc $(DESC) > super.h

am_dispatch.h: super.h
	$(TOOLS)/mk_amdisp > am_dispatch.h

system/AMdispatch.c: $(OBJS) system/AM.h system/AMdispatch.template
	(cpp -I. -Isystem/include system/AMdispatch.template | grep -v NULL_FUNC > system/AMdispatch.c)>& /dev/null

Makefile.objs: $(DESC)
	$(TOOLS)/desc2objs $(DESC) > Makefile.objs

All_headers.c: $(HEADER_FILES) Makefile.objs
	$(TOOLS)/desc2hdrs $(HEADER_FILES) > All_headers.c

include Makefile.objs

OBJS = system/sched.o $(CUST_OBJS) system/AMdispatch.o
OBJS = system/sched.o $(CUST_OBJS) 


%.s: %.S
	cpp $< $@ 

%.o : %.s
	$(AS) $< -o $@ 

%.s : %.c
	$(CC) $(SFLAGS) -o $@ $<

%.h : %.comp
	$(TOOLS)/mkheader $< > $@

%.o : %.c %.h
	$(CC) $(CFLAGS) -o $@ $<

%: %.o
	$(LD) $< -o $@ $(LIBS)



# -dlpt3 required for IBM thinkpad.  Remove on other platforms
install: main.srec
	$(PROGRAMER) -dapa -dno-poll --erase 
	$(PROGRAMER) -dapa -dno-poll --upload if=$<
	$(PROGRAMER) -dapa -dno-poll --verify if=$<

install.%: main.srec
	$(SET_ID) main `echo $@ |sed 's:install.::g'`
	$(PROGRAMER) -dapa -dno-poll --erase 
	$(PROGRAMER) -dapa -dno-poll --upload if=out.srec
	$(PROGRAMER) -dapa -dno-poll --verify if=out.srec


install_windows: main.srec
	$(WIN_PROGRAMER) --erase --speed 124 --upload $<

install_windows.%: main.srec
	$(SET_ID) main `echo $@ |sed 's:install_windows.::g'`
	$(WIN_PROGRAMER) --erase --speed 124 --upload out.srec

%.srec : %.exe
	$(OBJCOPY) --output-target=srec $< $@

main.exe: $(OBJS) $(HEADER_FILES) All_headers.o
	$(LD) -Xlinker -M -o $@  -m avr85xx -mmcu=at90s8535 $(OBJS) > img.file
	grep _e img.file | grep =\.

clean:
	rm -f *.o system/*.o main.exe main.srec Makefile.objs super.h *.h system/*.h img.file  system/AMdispatch.c All_headers.c

