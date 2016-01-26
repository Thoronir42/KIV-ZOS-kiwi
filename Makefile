#compiler
CC=gcc
#compiler options
OPTS=-c -Wall
#source files
SOURCES=$(wildcard *.c)
#object files
OBJECTS=$(SOURCES:.c=.o)
#sdl-config or any other library here. 
#``- ensures that the command between them is executed, and the result is put into LIBS
#executable filename
EXECUTABLE=FATty
#Special symbols used:
#$^ - is all the dependencies (in this case =$(OBJECTS) )
#$@ - is the result name (in this case =$(EXECUTABLE) )

default: all clean


all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(LINK.o) $^ -pthread -o $@

clean:
	rm $(OBJECTS)