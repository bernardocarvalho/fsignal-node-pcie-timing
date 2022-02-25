
OBJSX=PcieTimingParameter.x \
PcieTimingNode.x 

include ../../Makefile.inc.def

all: $(OBJS) \
	build/PcieTimingNode.a

INCLUDES+=-I../
INCLUDES+=-I../utils
INCLUDES+=-I../xml
INCLUDES+=-I../orb

EPIC_BASE=/home/oper/EPICS/epics-base
INCLUDES+=-I$(EPIC_BASE)/modules/libcom/src/osi/os/posix
INCLUDES+=-I$(EPIC_BASE)/include
INCLUDES+=-I$(EPIC_BASE)/include/compiler/gcc
# INCLUDES+=-I../../../ATCA-IO-CONTROL/SoftwareBBC/Linux/include/

LIBRARIES+=-L../build/ -lfsignal 

include ../../Makefile.inc.rules
