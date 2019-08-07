P=tertis
#OBJECTS=tertis.c
CFLAGS=`pkg-config --cflags sdl2 SDL2_image` -Wall -O3 -std=c99
LDLIBS=`pkg-config --libs sdl2 SDL2_image`
CC=clang

$(P): $(OBJECTS)