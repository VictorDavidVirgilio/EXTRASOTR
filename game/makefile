RTOS := flynshoot
ISOIMAGE := rtos.iso
.PHONY: clean default $(RTOS).exe

CC = mgcc
AR = ar
STRIP = strip
MARTE = /home/victor/myapps/marte_2.0_22Feb2017
CFLAGS = -Wall -Wextra -I$(MARTE)/arch/drivers/svga -I ../port -I ../include
OFLAGS = -s -Os -DNDEBUG
LDFLAGS = -lsvga

default: flynshoot.exe

#flynshoot.exe:
$(RTOS).exe:
	$(CC) $(CFLAGS) $(OFLAGS) ../qep/source/*.c ../qf/source/*.c *.c $(LDFLAGS) -o $@;
	$(STRIP) --strip-all $@
bsp_testE:bsp.c
	$(CC) -E $(CFLAGS) $(OFLAGS) $< -o $@
main_testE:main.c
	$(CC) -E $(CFLAGS) $(OFLAGS) $< -o $@
clean: 
	@exec echo -e "\n>> Cleaning... ";
	@find \( -name '*.[oa]' -or -name '*~*' -or -name '*.ali' -or -name '*.log' \
		-or -name '*.exe' -or -name 'mprogram' -or -name 'a.out' \) -print -delete
	@exec echo ">> End Cleaning"
