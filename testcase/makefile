COMPILER   = g++
SOURCE	   = $(wildcard *.cpp)
OBJECT     = $(filter-out ../obj/Main.o,$(wildcard ../obj/*.o))
INCLUDE    = ../include
LIBRARY    = ../lib /usr/X11/lib
LINK	   = liquidfun GL glut glui X11 Xrandr Xxf86vm
TARGET     = $(subst .cpp,,$(SOURCE))
DEFINE	   = GL_SILENCE_DEPRECATION
CMPLOPTION = -g -std=c++20 $(addprefix -D,$(DEFINE)) $(addprefix -I,$(INCLUDE))
LINKOPTION = -g $(addprefix -L,$(LIBRARY)) $(addprefix -l,$(LINK))

all: $(TARGET)

%: %.cpp
	$(COMPILER) $(CMPLOPTION) $(LINKOPTION) -o $@ $^ $(OBJECT)

.PHONY: clean
clean:
	-rm -r $(TARGET) *.dSYM save.dat