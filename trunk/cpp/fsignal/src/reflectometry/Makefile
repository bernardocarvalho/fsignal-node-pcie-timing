
OBJSX=ReflectNode.x \
ReflectParameter.x 

include ../../Makefile.inc.def

all: $(OBJS) \
	build/ReflectNodeMain.ex

INCLUDES+=-I../utils
INCLUDES+=-I../xml
INCLUDES+=-I../orb

LIBRARIES+=-L../build/ -lfsignal -ltelnet -lomnithread

include ../../Makefile.inc.rules

