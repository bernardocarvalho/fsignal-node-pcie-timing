
OBJSX= 

include ../../../Makefile.inc.def

all: $(OBJS) \
	build/TestNodeMain.ex

INCLUDES+=-I../
INCLUDES+=-I../../
INCLUDES+=-I../../utils
INCLUDES+=-I../../xml
INCLUDES+=-I../../orb
INCLUDES+=-I../../../../ATCA-IO-CONTROL/SoftwareBBC/Linux/include/

LIBRARIES+=-L../../build/ -lfsignal ../build/PcieAdcATCANode.a

include ../../../Makefile.inc.rules
