
OBJSX=CentralTestNode.x \
CentralTestParameter.x

TOP=/home/oper/Apps/epics/base

include ../../Makefile.inc.def
include $(TOP)/configure/CONFIG

all: $(OBJS) \
	build/CentralTestNodeMain.ex

INCLUDES+=-I../utils
INCLUDES+=-I../xml
INCLUDES+=-I../orb

#INCLUDES+=-I$(TOP)/include
#INCLUDES+=-I$(TOP)/include/os/Linux
#INCLUDES+=-I$(TOP)/include/compiler/gcc

LIBRARIES+=-L../build/ -lfsignal

include $(TOP)/configure/RULES
include ../../Makefile.inc.rules
