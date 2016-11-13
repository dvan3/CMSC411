# Include path (prob unused...)
INCLUDES= -I/usr/local/include

CC= g++
CFLAGS= $(DEFINES) $(INCLUDES)
RM= rm -f
EXECNAME= simulator

# Alternative target for compiling on Mac OS X:
all:
	$(CC) organization-template-impl.cpp simulator.cpp -o $(EXECNAME) $<

clean:
	$(RM) *.o *.so *.obj *.lib *.exp *.dll *.manifest simulator

