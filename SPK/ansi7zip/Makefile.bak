SRC =  	7Decoder.cpp LzmaDecode.cpp

OBJ = $(SRC:.cpp=.o)

OUT = ../lib7.a

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

$(OUT): $(OBJ)
	ar rcs $(OUT) $(OBJ)

depend: dep

dep:
	makedepend -- $(CFLAGS) -- $(INCLUDES) $(SRC)

clean:
	rm -f $(OBJ) $(OUT) Makefile.bak 
# DO NOT DELETE

7Decoder.o: 7Decoder.h Types.h /usr/include/stdio.h /usr/include/sys/cdefs.h
7Decoder.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
7Decoder.o: /usr/include/machine/_types.h LzmaDecode.h LzmaTypes.h
7Decoder.o: /usr/include/string.h /usr/include/strings.h
7Decoder.o: /usr/include/stdlib.h
LzmaDecode.o: LzmaDecode.h LzmaTypes.h /usr/include/string.h
LzmaDecode.o: /usr/include/sys/cdefs.h /usr/include/sys/_null.h
LzmaDecode.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
LzmaDecode.o: /usr/include/strings.h /usr/include/stdio.h
LzmaDecode.o: /usr/include/stdlib.h
