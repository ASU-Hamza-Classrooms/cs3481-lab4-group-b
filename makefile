CC = g++
CFLAGS = -g -c -Wall -std=c++11 -O0
OBJ = lab4.o MemoryTester.o Memory.o Tools.o RegisterFile.o \
RegisterFileTester.o ConditionCodes.o ConditionCodesTester.o

.C.o:
	$(CC) $(CFLAGS) $< -o $@

lab4: $(OBJ)

ConditionCodes.o: ConditionCodes.h Tools.h

Memory.o: Memory.h Tools.h

MemoryTester.o: Memory.h MemoryTester.h

RegisterFile.o: RegisterFile.h Tools.h

RegisterFileTester.o: RegisterFile.h RegisterFileTester.h

clean:
	rm $(OBJ) lab4

run:
	make clean
	make lab4
	./run.sh

