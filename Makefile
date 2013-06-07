CXX=g++
CXXFLAGS = -c

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
	$(CXX) spktool.o SPK/libspk.a SPK/libz.a SPK/lib7.a -o ../spktool

spkconvert: spkconvert.o SPK/libspk.a SPK/libz.a SPK/lib7.a
	$(CXX) spkconvert.o SPK/libspk.a SPK/libz.a SPK/lib7.a -o ../spkconvert

spktool.o: SPKTool/spktool.cpp
	$(CXX) $(CXXFLAGS) SPKTool/spktool.cpp

spkconvert.o: SPKConvert/main.cpp
	$(CXX) $(CXXFLAGS) SPKConvert/main.cpp -o spkconvert.o


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
	makedepend -- $(CXXFLAGS) -- *.[ch]

# DO NOT DELETE THIS LINE -- make depend depends on it.
