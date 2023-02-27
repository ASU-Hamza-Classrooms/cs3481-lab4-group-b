CC = g++
CFLAGS = -g -c -Wall -std=c++11 -O0
OBJ = lab5.o Loader.o Memory.o Tools.o RegisterFile.o \
ConditionCodes.o 

.C.o:
	$(CC) $(CFLAGS) $< -o $@

lab5: $(OBJ)

lab5.o: Memory.h RegisterFile.h ConditionCodes.h Loader.h

Loader.o: Loader.h Memory.h

ConditionCodes.o: ConditionCodes.h Tools.h

Memory.o: Memory.h Tools.h

RegisterFile.o: RegisterFile.h Tools.h


clean:
	rm $(OBJ) lab5

run:
	make clean
	make lab5
	./run.sh

