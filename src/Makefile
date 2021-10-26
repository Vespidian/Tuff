CC = gcc
CFLAGS = -Werror -Wall -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lglew32 -lopengl32 -llibtcc -g
DISABLE_FLAGS = -Wno-missing-braces

INCLUDE = -isystem ../include -L../lib

C_SOURCES = $(wildcard *.c */*.c */*/*.c)

OBJ = $(C_SOURCES:.c=.o)

../bin/main.exe: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(DISABLE_FLAGS) $(INCLUDE)
	
%.o: %.c
	$(CC) $(CFLAGS) $(DISABLE_FLAGS) $(INCLUDE) -c -MMD $< -o $@
	
-include *.d */*.d */*/*.d

clean:
	rm -rf *.o
	rm -rf */*.o
	rm -rf */*/*.o
	rm -rf *.d
	rm -rf */*.d
	rm -rf */*/*.d
	rm ../*.exe