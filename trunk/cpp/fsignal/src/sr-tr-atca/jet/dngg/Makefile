
OBJSX= 

include ../../../../Makefile.inc.def

all: $(OBJS) \
	build/DNGNodeMain.ex

INCLUDES+=-I../../
INCLUDES+=-I../../../
INCLUDES+=-I../../../utils
INCLUDES+=-I../../../xml
INCLUDES+=-I../../../orb
INCLUDES+=-I/home/andre/Projects/ATCA/SR-TR-ATCA/Software/Linux/include/

LIBRARIES+=-L../../../build/ -lfsignal ../../build/SRTRATCANode.a

include ../../../../Makefile.inc.rules
