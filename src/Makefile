CC = gcc
CFLAGS = -Werror -Wall -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lglew32 -lopengl32 -g
DISABLE_FLAGS = -Wno-missing-braces

# INCLUDE = -isystem C:/c_libs/include/ -LC:/c_libs/lib/
INCLUDE = -isystem ../libs/include -L../libs/lib/

C_SOURCES = $(wildcard *.c */*.c */*/*.c)
HEADERS = $(wildcard *.h */*.h */*/*.h)

OBJ = $(C_SOURCES:.c=.o)


../bin/foXandbox.exe: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(DISABLE_FLAGS) $(INCLUDE)
	
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(DISABLE_FLAGS) $(INCLUDE) -c $< -o $@
	
	
clean:
	rm -rf *.o
	rm -rf */*.o
	rm -rf */*/*.o
	rm log.txt
	rm ../*.exe