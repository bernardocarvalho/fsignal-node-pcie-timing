
OBJSX= 

include ../../../Makefile.inc.def

all: $(OBJS) \
	build/PcieTimingNodeMain.ex

INCLUDES+=-I../
INCLUDES+=-I../../
INCLUDES+=-I../../utils
INCLUDES+=-I../../xml
INCLUDES+=-I../../orb
INCLUDES+=-I../../include/

LIBRARIES+=-L../../build/ -lfsignal ../build/PcieTimingNode.a

include ../../../Makefile.inc.rules
