OBJSX=FSHardware.x \
FSNode.x \
FSParameter.x \
Properties.x

STATIC_LIBS=build/FSCore.a \
orb/build/FSOrb.a \
xml/build/FSXML.a \
utils/build/FSUtils.a

include ../Makefile.inc.def

all:	$(OBJS) \
	build/FSCore.a
	make -s -C orb -f Makefile
	make -s -C xml -f Makefile
	make -s -C utils -f Makefile 
	make -s dll
test:
	make -s -C test -f Makefile

dll:	build/fsignal.so
	

cleanall:clean
	make -s -C utils -f Makefile clean
	make -s -C orb -f Makefile clean
	make -s -C xml -f Makefile clean

include ../Makefile.inc.rules
