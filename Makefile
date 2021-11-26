CC = cc
CFLAGS = `sdl2-config --cflags --libs` -Wall -Wextra
ODIR=obj
DEPS = i8080.h i8080Type.h opcode.h sdlFunctions.h invaders.h
_OBJ = main.o i8080.o opcode.o sdlFunctions.o invaders.o
OBJ  = $(patsubst %,$(ODIR)/%,$(_OBJ))
default: invaders

$(OBJ): $(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
invaders: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
clean:
	rm -rf $(ODIR)/*
	rm ./invaders
