CC=nios2-elf-gcc
LD=nios2-elf-ld
CFLAGS=-c -nostdinc -W -Wall -G0 -O2 -mhw-div -I../bootloader -DRANGECHECK

OBJECTS    = start.o i_video.o i_timer.o am_map.o d_items.o d_iwad.o d_main.o doomdef.o doomstat.o dstrings.o f_finale.o f_wipe.o g_game.o hu_lib.o hu_stuff.o info.o i_main.o i_system.o m_argv.o m_bbox.o m_cheat.o m_config.o m_fixed.o m_menu.o m_misc.o m_random.o p_ceilng.o p_doors.o p_enemy.o p_floor.o p_inter.o p_lights.o p_map.o p_maputl.o p_mobj.o p_plats.o p_pspr.o p_saveg.o p_setup.o p_sight.o p_spec.o p_switch.o p_telept.o p_tick.o p_user.o r_bsp.o r_data.o r_draw.o r_main.o r_plane.o r_segs.o r_sky.o r_things.o sounds.o s_sound.o st_lib.o st_stuff.o tables.o v_video.o w_file.o wi_stuff.o w_merge.o w_wad.o z_zone.o
MEMORY_MAP = doom.x
ELF_OUT    = doom.elf
BINARY     = doom.bin
LD_SCRIPT  = doom.ld

.PHONY: clean

#
# Binäärin teko
#
$(BINARY): $(ELF_OUT)
	nios2-elf-objcopy -O binary $(ELF_OUT) $(BINARY)

#
# Linkkeri
#
$(ELF_OUT): $(OBJECTS) $(LD_SCRIPT)
	$(LD) -o $(ELF_OUT) $(OBJECTS) -T $(LD_SCRIPT) ../bootloader/string.o \
	../bootloader/graphic.o ../bootloader/fat32.o ../bootloader/timer.o \
	../bootloader/utils.o ../bootloader/jtag.o ../bootloader/fonts.o \
	../bootloader/sdcard.o -Map memorymap

#
# Käsitellään muistikartta cppllä
#
$(LD_SCRIPT): $(MEMORY_MAP)
	cpp $(MEMORY_MAP) >$(LD_SCRIPT)
	sed -i -e 's/^#.*$$//g' $(LD_SCRIPT)

start.o: start.S
	$(CC) $(CFLAGS) start.S

clean:
	rm -f $(ELF_OUT) $(BINARY) $(LD_SCRIPT) $(OBJECTS)
