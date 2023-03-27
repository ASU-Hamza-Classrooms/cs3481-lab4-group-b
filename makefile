CC = g++
CFLAGS = -g -c -Wall -std=c++11 -O0
OBJ = yess.o Loader.o Memory.o Tools.o RegisterFile.o ConditionCodes.o PipeReg.o PipeRegField.o Simulate.o DecodeStage.o ExecuteStage.o FetchStage.o MemoryStage.o WritebackStage.o

.C.o:
	$(CC) $(CFLAGS) $< -o $@

yess: $(OBJ)

yess.o: Debug.h Memory.h RegisterFile.h ConditionCodes.h Loader.h PipeReg.h Stage.h Simulate.h

PipeReg.o: PipeReg.h

PipeRegField.o: PipeRegField.h

Simulate.o: PipeRegField.h PipeReg.h F.h D.h E.h M.h W.h Stage.h ExecuteStage.h MemoryStage.h DecodeStage.h FetchStage.h WritebackStage.h Simulate.h Memory.h RegisterFile.h ConditionCodes.h

DecodeStage.o: Stage.h

ExecuteStage.o: Stage.h

FetchStage.o: Stage.h

MemoryStage.o: Stage.h

WritebackStage.o: Stage.h

Loader.o: Loader.h Memory.h

ConditionCodes.o: ConditionCodes.h Tools.h

Memory.o: Memory.h Tools.h

RegisterFile.o: RegisterFile.h Tools.h


clean:
	rm $(OBJ) yess

run:
	make clean
	make yess
	./run.sh

