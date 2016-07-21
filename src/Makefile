#
# Makefile for glSkel C++ 
#
#---------------------------------------------------------------------

PROG = main

BASE_DIR = ${PWD}/../
BUILD_DIR = $(BASE_DIR)build/

# -g tells compiler to generate debug info 
MYCPPFLAGS = -g
                           
# -------------------- set system-dependent variables ----------------
CC = clang
CCC = clang++
LINKER = clang++

GL_LIBS     = -L$(BASE_DIR)lib/OSX -lglfw3 -lGLEW -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo
CPPFLAGS    = -Wno-overloaded-virtual -Wno-deprecated-declarations

# -------------- Common specifications ---------------------------------

MAKEFILE    = Makefile

# -------------- Compiler and Linker flags ---------------
CPPFLAGS    += -Wall -std=c++14 $(MYCPPFLAGS) -I$(BASE_DIR)include

#---------- Application info ------------------------------------------

SRCS = $(wildcard *cpp)

# for every .cpp input, need to produce a .o
OBJS = $(BUILD_DIR)$(SRCS:.cpp=.o) 

#------------------- dependencies/actions ------------------------
# dependency: need1 need2 ...  
#         action(s)
#
default: build

build: makebuilddir compile $(PROG)

makebuilddir:
	mkdir -p $(BASE_DIR)build

compile: $(OBJS) 

clean:	clear build
all: build run

$(BUILD_DIR)%.o : %.cpp %.h
	$(CCC) -c $(CPPFLAGS) $*.cpp -o $(BUILD_DIR)$*.o

$(BUILD_DIR)%.o : %.cpp
	$(CCC) -c $(CPPFLAGS) $*.cpp -o $(BUILD_DIR)$*.o

$(PROG): $(OBJS)
	$(LINKER) $(OBJS) $(GL_LIBS) -o $(BUILD_DIR)$(PROG)

run: build
	$(BUILD_DIR)$(PROG)

clear:
	rm -f $(BUILD_DIR)*.o $(BUILD_DIR)$(PROG) $(PWD)/*.o
