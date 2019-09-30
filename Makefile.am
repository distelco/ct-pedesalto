##AUTOMAKE_OPTIONS = foreign no-installinfo no-installman
INCLUDES = -I$(srcdir) -I$(srcdir)/include
LDADD = devices/libdevices.a drivers/libdrivers.a modbus/libmodbus.a \
	comunication/libcomunication.a common/libtimer.a \
	commands/libcommands.a common/libserial.a  common/libcommon.a
SUBDIRS = ct commands common comunication devices drivers modbus

bin_PROGRAMS = ctd

ctd_SOURCES = plc.c
ctd_LDADD = $(LDADD) ct/libct.a $(LDADD) ct/libct.a $(LDADD)
