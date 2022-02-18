
OBJSX=TestNode.x \
TestParameter.x 

include ../../Makefile.inc.def

all: $(OBJS) \
	build/TestNodeMain.ex

INCLUDES+=-I../utils
INCLUDES+=-I../xml
INCLUDES+=-I../orb

LIBRARIES+=-L../build/ -lfsignal 

include ../../Makefile.inc.rules

