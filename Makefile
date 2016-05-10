MAIN = standaloneExample

LIBDIRS = 
INCDIRS = 

CC = g++
CFLAGS = -v 
LIBS = -lglfw -lGLEW

OS := $(shell uname)
ifeq ($(OS),Darwin)
	CFLAGS += -framework OpenGL 
else
	LIBDIRS += -L/usr/X11R6/lib -L/usr/X11R6/lib64 -L/usr/local/lib
	INCDIRS += -I/usr/include -I/usr/local/include -I/usr/include/GL
	CFLAGS += $(INCDIRS)	
	LIBS += -lGL	
endif


all:
	$(CC) $(CFLAGS) -o $(MAIN) $(LIBDIRS) $(MAIN).cpp helper.cpp $(LIBS)
clean:
	rm -f *.o
