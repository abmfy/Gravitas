COMPILER   = g++
SOURCE     = $(wildcard src/*.cpp)
OBJECT     = $(subst src,obj,$(SOURCE:.cpp=.o))
DEPENDENCY = $(OBJECT:.o=.d)
INCLUDE    = include
LIBRARY    = lib /usr/X11/lib /usr/X11R6/lib
LINK	   = liquidfun GL glut glui X11 Xrandr Xxf86vm
TARGET     = build/main
DEFINE	   = GL_SILENCE_DEPRECATION
CMPLOPTION = -MMD -g -std=c++17 $(addprefix -D,$(DEFINE)) $(addprefix -I,$(INCLUDE))
LINKOPTION = -g $(addprefix -L,$(LIBRARY)) $(addprefix -l,$(LINK))

$(TARGET): $(OBJECT)
	$(COMPILER) $(LINKOPTION) -o $@ $^

obj/%.o: src/%.cpp
	$(COMPILER) $(CMPLOPTION) -c $< -o $@

-include $(DEPENDENCY)

.PHONY: clean
clean:
	-rm $(TARGET) $(OBJECT) $(DEPENDENCY)