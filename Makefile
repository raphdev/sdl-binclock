CC=g++
CFLAGS=-ggdb -c -Wall
LDFLAGS=-lSDL
SDL_SRC=src/binclock_sdl.cpp
NC_SRC=src/binclock_ncurses.c
SDL_OBJ=$(SDL_SRC:.cpp=.o)
NC_OBJ=$(NC_SRC:.c=.o)
SDL_TARGET=binclock_sdl
NC_TARGET=binclock_ncurses

all: $(SDL_SRC) $(SDL_TARGET)

$(SDL_TARGET): $(SDL_OBJ) $(NC_TARGET)
	$(CC) $(LDFLAGS) $(SDL_OBJ) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

$(NC_TARGET): $(NC_OBJ)
	$(CC) $(LDFLAGS) $(NC_OBJ) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(NC_OBJ) $(SDL_OBJ)

distclean: clean
	rm -f $(NC_TARGET) $(SDL_TARGET)