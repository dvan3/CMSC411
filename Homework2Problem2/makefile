#makefile
#Dave Van
#Spring12
##


CC = /usr/bin/gcc
CFLAGS = -Wall -g -std=c99
LDFLAGS = -lm

# name of the executable to be created
PROJECT = hw2prob2

# list of all .c files
SRCS = hw2prob2.c 

#list of all .h files
HDRS = fractions.h mixFraction.h

# list of .o files
OBJS = $(SRCS: .c = .o)

# the first rule is the default rule
# build the executable from the .o files
PROJECT : $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(PROJECT)

# some handy commands that you can "make"
clean:
	rm -f core* $(PROJECT) $(OBJS)

cleaner: clean
	rm -f #* *~
