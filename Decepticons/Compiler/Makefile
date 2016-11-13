#makefile
#Sean Cosentino
#Spring 12
##


CC = /usr/bin/g++
CFLAGS = -Wall -g
LDFLAGS = -lm

# name of the executable to be created
PROJECT = compile

# list of all .cpp files
SRCS = compiler.cpp

#list of all .h files
HDRS = 

# list of .o files
OBJS = $(SRCS: .c = .o)

# the first rule is the default rule
# build the executable from the .o files
PROJECT : $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(PROJECT)

# a separate rule to compile each .cpp file

compiler.o : compiler.cpp

