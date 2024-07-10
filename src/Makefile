CC = gcc

# DEFINES = -DNOOPENGL

ifeq ($(OS),Windows_NT)     # is Windows_NT on XP, 2000, 7, Vista, 10...
	LDFLAGS = -L../lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lglew32 -lopengl32
	CFLAGS = -Wall -g
	INCLUDE = -isystem ../include -isystem ../include/win_only
else
	LDFLAGS = -lSDL2 -lSDL2_image -lGLEW -lGL -lm -g
	CFLAGS = -Wall -g
	INCLUDE = -isystem ../include
endif

DISABLE_FLAGS = -Wno-missing-braces

C_SOURCES = $(wildcard *.c */*.c */*/*.c)

OBJ = $(C_SOURCES:.c=.o)

../bin/main.exe: $(OBJ)
	$(CC) $^ $(LDFLAGS) -o $@
	
%.o: %.c
	$(CC) $(CFLAGS) $(DISABLE_FLAGS) $(DEFINES) $(INCLUDE) -c -MMD $< -o $@
	
-include *.d */*.d */*/*.d

clean:
	rm -rf *.o
	rm -rf */*.o
	rm -rf */*/*.o
	rm -rf *.d
	rm -rf */*.d
	rm -rf */*/*.d
	rm ../*.exe