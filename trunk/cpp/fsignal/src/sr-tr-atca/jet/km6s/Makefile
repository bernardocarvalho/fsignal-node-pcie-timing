
OBJSX= 

include ../../../../Makefile.inc.def

all: $(OBJS) \
	build/KM6SNodeMain.ex

INCLUDES+=-I../../
INCLUDES+=-I../../../
INCLUDES+=-I../../../utils
INCLUDES+=-I../../../xml
INCLUDES+=-I../../../orb
INCLUDES+=-I/home/andre/Projects/ATCA/SR-TR-ATCA/Software/Linux/include/

LIBRARIES+=-L../../../build/ -lfsignal ../../build/SRTRATCANode.a -L/usr/local/lib

include ../../../../Makefile.inc.rules
