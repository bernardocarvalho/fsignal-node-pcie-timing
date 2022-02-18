
OBJSX=PciEpnParameter.x \
PciEpnNode.x 

include ../../Makefile.inc.def

all: $(OBJS) \
	build/PciEpnNode.a

INCLUDES+=-I../
INCLUDES+=-I../utils
INCLUDES+=-I../xml
INCLUDES+=-I../orb
INCLUDES+=-I./include

LIBRARIES+=-L../build/ -lfsignal 

include ../../Makefile.inc.rules
