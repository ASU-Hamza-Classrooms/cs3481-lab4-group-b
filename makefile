CC = g++
CFLAGS = -g -c -Wall -std=c++11 -O0
OBJ = yess.o Loader.o Memory.o Tools.o RegisterFile.o ConditionCodes.o PipeReg.o PipeRegField.o Simulate.o DecodeStage.o ExecuteStage.o FetchStage.o MemoryStage.o WritebackStage.o F.o D.o E.o M.o W.o

.C.o:
	$(CC) $(CFLAGS) $< -o $@

yess: $(OBJ)

yess.o: Debug.h Memory.h RegisterFile.h ConditionCodes.h Loader.h PipeReg.h Stage.h Simulate.h

PipeReg.o: PipeReg.h

PipeRegField.o: PipeRegField.h

Simulate.o: PipeRegField.h PipeReg.h F.h D.h E.h M.h W.h Stage.h ExecuteStage.h MemoryStage.h DecodeStage.h FetchStage.h WritebackStage.h Simulate.h Memory.h RegisterFile.h ConditionCodes.h

DecodeStage.o: Stage.h RegisterFile.h Instructions.h PipeRegField.h PipeReg.h F.h D.h E.h DecodeStage.h Status.h Debug.h

ExecuteStage.o: Stage.h RegisterFile.h PipeRegField.h PipeReg.h D.h E.h M.h W.h ExecuteStage.h Status.h Debug.h

FetchStage.o: Stage.h Instructions.h RegisterFile.h PipeRegField.h PipeReg.h F.h D.h M.h W.h FetchStage.h Status.h Debug.h Tools.h Memory.h

MemoryStage.o: Stage.h RegisterFile.h PipeRegField.h PipeReg.h E.h M.h W.h MemoryStage.h Status.h Debug.h

WritebackStage.o: Stage.h Instructions.h RegisterFile.h PipeRegField.h PipeReg.h M.h W.h WritebackStage.h Status.h Debug.h

Loader.o: Loader.h Memory.h

ConditionCodes.o: ConditionCodes.h Tools.h

Memory.o: Memory.h Tools.h

RegisterFile.o: RegisterFile.h Tools.h

F.o: PipeRegField.h PipeReg.h F.h

D.o: Instructions.h RegisterFile.h PipeReg.h PipeRegField.h D.h Status.h

E.o: RegisterFile.h Instructions.h PipeReg.h PipeRegField.h E.h Status.h

M.o: RegisterFile.h Instructions.h PipeReg.h PipeRegField.h M.h Status.h

W.o: RegisterFile.h Instructions.h PipeReg.h PipeRegField.h W.h Status.h

clean:
	rm $(OBJ) yess

run:
	make clean
	make yess
	./run.sh

