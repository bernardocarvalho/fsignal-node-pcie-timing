
OBJSX=SRTRATCANode.x \
SRTRATCAParameter.x 

include ../../Makefile.inc.def

all: $(OBJS) \
	build/SRTRATCANode.a

INCLUDES+=-I../
INCLUDES+=-I../utils
INCLUDES+=-I../xml
INCLUDES+=-I../orb
INCLUDES+=-I../../../../llevel/Linux/include/
INCLUDES+=-I../../../../../../ATCA/SR-TR-ATCA/Software/Linux/include/

LIBRARIES+=-L../build/ -lfsignal 

include ../../Makefile.inc.rules
