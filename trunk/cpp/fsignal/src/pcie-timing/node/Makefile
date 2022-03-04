
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

EPIC_BASE=/home/oper/EPICS/epics-base
INCLUDES+=-I$(EPIC_BASE)/modules/libcom/src/osi/os/posix
INCLUDES+=-I$(EPIC_BASE)/include
INCLUDES+=-I$(EPIC_BASE)/include/compiler/gcc

LIBRARIES+=-L../../build/ -lfsignal ../build/PcieTimingNode.a
LIBRARIES+=-L/home/oper/EPICS/epics-base/lib/linux-x86_64 -lca 

include ../../../Makefile.inc.rules
