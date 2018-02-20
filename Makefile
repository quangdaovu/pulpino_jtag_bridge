CXX = g++
CC = gcc
CFLAGS = -Wall -fPIC -c -I /usr/include/libftdi1/ -DADBG_OPT_HISPEED
CXXFLAGS = -Wall -g -O -D__STDC_LIMIT_MACROS -DADBG_OPT_HISPEED -lftdi1
OBJECTC = *.o
RM = rm -rf

CSOURCES = \
	errcodes.c \
	utilities.c \
	cable_ft2232.c \
	cable_common.c \
	chain_commands.c \
	adv_dbg_commands.c 

CXXSOURCES = \
	jtagif.cpp \
	debug_if.cpp \
	breakpoints.cpp \
	rsp.cpp \
	cache.cpp \
	bridge.cpp \
	main.cpp 

all: jtag_bridge

jtag_bridge:
	$(CC) $(CSOURCES) $(CFLAGS)
	$(CXX) $(OBJECTC) $(CXXSOURCES) -o jtag_bridge $(CXXFLAGS)

.PHONY: clean
clean:
	$(RM) ./*.gc??
	$(RM) ./*.o
	$(RM) ./*.so
	$(RM) ./jtag_bridge