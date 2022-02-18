
OBJSX= 

include ../../../Makefile.inc.def

all: $(OBJS) \
	build/PciEpnNodeMain.ex

INCLUDES+=-I../
INCLUDES+=-I../../
INCLUDES+=-I../../utils
INCLUDES+=-I../../xml
INCLUDES+=-I../../orb
INCLUDES+=-I../../include/

LIBRARIES+=-L../../build/ -lfsignal ../build/PciEpnNode.a

include ../../../Makefile.inc.rules
