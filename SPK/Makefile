SRC = CatFile.cpp File.cpp File_IO.cpp MultiSpkFile.cpp SpkFile.cpp XFile.cpp XspFile.cpp

OBJ = $(SRC:.cpp=.o)

# dependency makefiles, generated by gcc in target $(OBJ)
DEP = $(SRC:.cpp=.d)

OUT = libspk.a

# C++ compiler flags (-g -O2 -Wall)
CXXFLAGS = -g

# compiler
CXX = g++

$(OUT): $(OBJ) lib7.a libz.a
	ar rcs $(OUT) $(OBJ)

$(OBJ): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

libz.a:
	make -C zlib -f Makefile

lib7.a:
	make -C ansi7zip -f Makefile

# use proper target even if a file named 'clean' is present
.PHONY: clean

clean:
	rm -f $(OBJ) $(DEP) $(OUT)
	make -C zlib -f Makefile clean
	make -C ansi7zip -f Makefile clean

# update dependencies if necessary, before doing anything else
-include $(DEP)
