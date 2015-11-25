###OSNAME###
OSNAME = 
ifeq ($(OS),Windows_NT)
    OSNAME = WIN32
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        OSNAME =  LINUX
    endif
    ifeq ($(UNAME_S),Darwin)
        OSNAME = OSX
    endif
endif

###INPUT###
SOURCE = $(wildcard src/*.cpp)
HEADERS = $(wildcard src/*.h)
INTERM = $(patsubst %.cpp,%.o,$(SOURCE))
OBJECTS = $(patsubst src/%,nbobj/%,$(INTERM))

###OUTPUT###
OUTNAME = gstd
OUTFILE = 
ifeq ($(OSNAME),LINUX)
    OUTFILE = lib$(OUTNAME).so
else ifeq ($(OSNAME),OSX)
    OUTFILE = lib$(OUTNAME).dylib
endif

###COMPILER###
CC = 
ifeq ($(OSNAME),LINUX)
    CC = g++
else ifeq ($(OSNAME),OSX)
    CC = clang++ -stdlib=libc++
endif

###FLAGS###
FLAGS = -fPIC -g -c -Wall -Wextra -Werror -pedantic -O3
LIBFLAGS = 
ifeq ($(OSNAME),LINUX)
    LIBFLAGS = -shared
else ifeq ($(OSNAME),OSX)
    LIBFLAGS = -dynamiclib -install_name '$(CURDIR)/../lib$(OUTNAME).dylib'
endif
ifeq ($(OSNAME),LINUX)
    FLAGS += -std=c++0x
else ifeq ($(OSNAME),OSX)
    FLAGS += -std=c++11
endif

###DEPS###
INCLUDE =
LINK =

###TARGETS###
$(OUTFILE): $(OBJECTS)
	$(CC) $(LIBFLAGS) -shared $(OBJECTS) -o $(OUTFILE) $(LINK)
	mv $(OUTFILE) ../

nbobj/%.o: src/%.cpp $(HEADERS)
	$(CC) $(FLAGS) $(INCLUDE) -o $@ -c $<
	
clean:
	rm -rf nbobj/* ../$(OUTFILE)
