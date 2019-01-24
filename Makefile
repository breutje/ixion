#
# IXION
#
DEFINES = -Wall -DLINUX -DUNIX
FLAGS = -g
CFLAGS = $(DEFINES) $(FLAGS)
INCLUDES = -I. -I/usr/local/include
OSLIBS = -lstdc++
LIBS = -lrt -lm
LDFLAGS = -I/usr/lib -I/usr/local/lib
DEBUG =

CC = gcc

ALL = monixion asmixion disixion genixion

all: $(ALL)

#
# monixion - monitor / cpu
#
monixion_objects = monmain.o monixion.o ixion.o instructions.o memory.o utility.o s19.o build.o
monixion: $(monixion_objects) build.cpp ixion.h memory.h s19.h instructions.h
	$(CC) $(LDFLAGS) -o $@ $(monixion_objects) $(LIBS)
	rm -f build.cpp

#
# asmixion - assembler
#
asmixion_objects = asmmain.o asmixion.o instructions.o memory.o utility.o s19.o build.o
asmixion: $(asmixion_objects) build.cpp ixion.h memory.h s19.h instructions.h
	$(CC) $(LDFLAGS) -o $@ $(asmixion_objects) $(LIBS)
	rm -f build.cpp

#
# disixion - disassemble ixion S1/S9 files
#
disixion_objects = dismain.o disixion.o ixion.o instructions.o memory.o utility.o s19.o build.o
disixion: $(disixion_objects) build.cpp ixion.h instructions.h memory.h
	$(CC) $(LDFLAGS) -o $@ $(disixion_objects) $(LIBS)
	rm -f build.cpp

#
# genixion - generate instructions.h from instructions.cpp
#
genixion_objects = genixion.o instructions.o utility.o build.o
genixion: $(genixion_objects)
	$(CC) $(LDFLAGS) -o $@ $(genixion_objects) $(LIBS)
	rm -f build.cpp

instructions.h: genixion
	./genixion

build.cpp:
	@expr `cat .build 2>/dev/null` + 1 >.build
	@echo -n "char *_version = (char *) \"" >build.cpp
	@echo "`cat .version`\";" >>build.cpp
	@echo -n "int _build = " >>build.cpp
	@echo "`cat .build`;" >>build.cpp
	@echo -n "char *_date = (char *) \"" >>build.cpp
	@echo "`TZ=$(APP_TZ) date`\";" >>build.cpp

clean:
	rm -fr *.o build.cpp $(ALL)

#
# default rule to compile .cpp files into .o file
#
%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $(DEBUG) $<
