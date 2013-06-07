CPP=g++
CC=cc
CFLAGS = -c

all: SPK/libz.a SPK/lib7.a SPK/libspk.a spktool spkconvert

### - Compile the libraries

SPK/libspk.a:
	make -C SPK -f Makefile	

SPK/libz.a:
	make -C SPK/zlib -f Makefile

SPK/lib7.a:
	make -C SPK/ansi7zip -f Makefile

### - Binary Compile

spktool: spktool.o SPK/libspk.a SPK/libz.a SPK/lib7.a
	$(CPP) spktool.o SPK/libspk.a SPK/libz.a SPK/lib7.a -o ../spktool

spkconvert: spkconvert.o SPK/libspk.a SPK/libz.a SPK/lib7.a
	$(CPP) spkconvert.o SPK/libspk.a SPK/libz.a SPK/lib7.a -o ../spkconvert

spktool.o: SPKTool/spktool.cpp
	$(CPP) $(CFLAGS) SPKTool/spktool.cpp

spkconvert.o: SPKConvert/main.cpp
	$(CPP) $(CFLAGS) SPKConvert/main.cpp -o spkconvert.o


clean:
	rm -rf *.o
	rm -rf SPK/*.o
	rm -rf SPK/zlib/*.o
	rm -rf SPK/ansi7zip/*.o

distclean:
	rm -rf *.o
	rm -rf SPK/*.o
	rm -rf SPK/*.a
	rm -rf SPK/zlib/*.o
	rm -rf SPK/ansi7zip/*.o

depend:
	makedepend -- $(CFLAGS) -- *.[ch]

# DO NOT DELETE THIS LINE -- make depend depends on it.
