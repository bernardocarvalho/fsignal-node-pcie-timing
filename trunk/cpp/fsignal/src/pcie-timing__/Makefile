
OBJSX=PcieTimingParameter.x \
PcieTimingNode.x 

include ../../Makefile.inc.def

all: $(OBJS) \
	build/PcieTimingNode.a

INCLUDES+=-I../
INCLUDES+=-I../utils
INCLUDES+=-I../xml
INCLUDES+=-I../orb
# INCLUDES+=-I../../../ATCA-IO-CONTROL/SoftwareBBC/Linux/include/

LIBRARIES+=-L../build/ -lfsignal 

include ../../Makefile.inc.rules
