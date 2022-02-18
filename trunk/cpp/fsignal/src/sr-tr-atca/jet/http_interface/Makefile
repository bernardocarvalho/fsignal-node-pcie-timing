
OBJSX=HTTPInterface.x\
DevDriverHardwareInterface.x

include ../../../Makefile.inc.def

all: $(OBJS) \
	build/JETHTTPInterface.a \
	build/TestHTTPInterface.ex

INCLUDES+=-I../../
INCLUDES+=-I../../utils
INCLUDES+=-I../../xml
INCLUDES+=-I../../orb

LIBRARIES+=-L../../build/ -lfsignal libshttpd.a

include ../../../Makefile.inc.rules

