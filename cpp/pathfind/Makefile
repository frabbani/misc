CXX ?= g++
PKG_CONFIG ?= pkg-config

TARGET := main.exe
SOURCES := main.cpp mysdl2.cpp
OBJECTS := $(SOURCES:.cpp=.o)
SDL_PACKAGES := sdl2 SDL2_image SDL2_ttf SDL2_net

CPPFLAGS += -I. $(shell $(PKG_CONFIG) --cflags $(SDL_PACKAGES))
CXXFLAGS ?= -std=c++17 -O2
LDLIBS += $(shell $(PKG_CONFIG) --libs $(SDL_PACKAGES))

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@ $(LDLIBS)

%.o: %.cpp mysdl2.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(TARGET) $(OBJECTS)
