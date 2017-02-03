OBJ = main.o fun.o

all: hello

hello: $(OBJ)
	gcc $(OBJ) -o shell

$(OBJ): fun.h

.PHONY: clean
clean:
	rm *.o
