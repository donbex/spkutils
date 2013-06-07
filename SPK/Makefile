SRC =  	CatFile.cpp File.cpp File_IO.cpp MultiSpkFile.cpp SpkFile.cpp XFile.cpp XspFile.cpp

OBJ = $(SRC:.cpp=.o)

OUT = libspk.a

# include directories
INCLUDES = -I. -I../include/ -I/usr/local/include

# C++ compiler flags (-g -O2 -Wall)
CCFLAGS = -g

# compiler
CCC = cc

# library paths
LIBS = -L../ -L/usr/local/lib -lm

# compile flags
LDFLAGS = -g

.SUFFIXES: .cpp

default: dep $(OUT)

.cpp.o:
	$(CCC) $(INCLUDES) $(CCFLAGS) -c $< -o $@

libz.a:
	make -C zlib -f Makefile

lib7.a:
	make -C ansi7zip -f Makefile

$(OUT): $(OBJ) lib7.a libz.a
	ar rcs $(OUT) $(OBJ) lib7.a libz.a

depend: dep

dep:
	makedepend -- $(CFLAGS) -- $(INCLUDES) $(SRC)

clean:
	rm -f $(OBJ) $(OUT) Makefile.bak 
# DO NOT DELETE

CatFile.o: CatFile.h File.h String.h zlib/zlib.h zlib/zconf.h
CatFile.o: ansi7zip/7Decoder.h ansi7zip/Types.h /usr/include/stdio.h
CatFile.o: /usr/include/sys/cdefs.h /usr/include/sys/_null.h
CatFile.o: /usr/include/sys/_types.h /usr/include/machine/_types.h File_IO.h
CatFile.o: lists.h /usr/include/stdlib.h /usr/include/time.h
CatFile.o: /usr/include/sys/timespec.h
File.o: File.h String.h zlib/zlib.h zlib/zconf.h ansi7zip/7Decoder.h
File.o: ansi7zip/Types.h /usr/include/stdio.h /usr/include/sys/cdefs.h
File.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
File.o: /usr/include/machine/_types.h /usr/include/time.h
File.o: /usr/include/sys/timespec.h /usr/include/sys/stat.h
File.o: /usr/include/sys/time.h /usr/include/sys/_timeval.h
File.o: /usr/include/sys/types.h /usr/include/machine/endian.h
File.o: /usr/include/sys/_pthreadtypes.h /usr/include/sys/select.h
File.o: /usr/include/sys/_sigset.h /usr/include/unistd.h
File.o: /usr/include/sys/unistd.h SpkFile.h lists.h /usr/include/stdlib.h
File.o: CatFile.h File_IO.h
File_IO.o: File_IO.h String.h
MultiSpkFile.o: MultiSpkFile.h String.h SpkFile.h lists.h
MultiSpkFile.o: /usr/include/stdlib.h /usr/include/sys/cdefs.h
MultiSpkFile.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
MultiSpkFile.o: /usr/include/machine/_types.h File.h zlib/zlib.h zlib/zconf.h
MultiSpkFile.o: ansi7zip/7Decoder.h ansi7zip/Types.h /usr/include/stdio.h
SpkFile.o: SpkFile.h lists.h /usr/include/stdlib.h /usr/include/sys/cdefs.h
SpkFile.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
SpkFile.o: /usr/include/machine/_types.h File.h String.h zlib/zlib.h
SpkFile.o: zlib/zconf.h ansi7zip/7Decoder.h ansi7zip/Types.h
SpkFile.o: /usr/include/stdio.h
XFile.o: XFile.h File.h String.h zlib/zlib.h zlib/zconf.h ansi7zip/7Decoder.h
XFile.o: ansi7zip/Types.h /usr/include/stdio.h /usr/include/sys/cdefs.h
XFile.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
XFile.o: /usr/include/machine/_types.h
XspFile.o: XspFile.h SpkFile.h lists.h /usr/include/stdlib.h
XspFile.o: /usr/include/sys/cdefs.h /usr/include/sys/_null.h
XspFile.o: /usr/include/sys/_types.h /usr/include/machine/_types.h File.h
XspFile.o: String.h zlib/zlib.h zlib/zconf.h ansi7zip/7Decoder.h
XspFile.o: ansi7zip/Types.h /usr/include/stdio.h StringList.h
