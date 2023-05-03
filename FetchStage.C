#include <string>
#include <cstdint>
#include "Instructions.h"
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "E.h"
#include "M.h"
#include "W.h"
#include "Stage.h"
#include "FetchStage.h"
#include "DecodeStage.h"
#include "ExecuteStage.h"
#include "Status.h"
#include "Debug.h"
#include "Tools.h"
#include "Memory.h"


/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool FetchStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
   F * freg = (F *) pregs[FREG];
   D * dreg = (D *) pregs[DREG];
   E * ereg = (E *) pregs[EREG];
   M * mreg = (M *) pregs[MREG];
   W * wreg = (W *) pregs[WREG];

   DecodeStage *dec = (DecodeStage *)stages[DSTAGE];
   ExecuteStage *exe = (ExecuteStage *)stages[ESTAGE];
   uint64_t f_pc = 0, icode = 0, ifun = 0, valC = 0, valP = 0;
   uint64_t rA = RNONE, rB = RNONE, stat = SAOK;
   F_stallVar = false;
   D_stallVar = false;

   //code missing here to select the value of the PC
   //and fetch the instruction from memory
   //Fetching the instruction will allow the icode, ifun,
   //rA, rB, and valC to be set.
   //The lab assignment describes what methods need to be
   //written.
   //sets f_pc

   f_pc = selectPC(freg, mreg, wreg);
   bool mem_error = false;

   //gets instruction byte from memory
   Memory * mem_instance = Memory::getInstance();
   uint8_t instByte = mem_instance->getByte(f_pc, mem_error);
   
   // Set icode to NOP if mem_error is true
   if (mem_error)
      icode = INOP;
   // else get icode from instruction byte
   else 
      icode = Tools::getBits(instByte, 4, 7);

   // Set ifun to FNONE if mem_error is true
   if (mem_error)
      ifun = FNONE;
   // else get ifun from the instruction byte
   else 
      ifun = Tools::getBits(instByte, 0, 3);

   stat = f_stat(mem_error, icode);

   //setting valP from PCIncrement
   valP = PCincrement(f_pc, needRegIds(icode), needValC(icode));

   //getting the predicted PC and storing it in the temp variable
   uint64_t tempPredPC = predictPC(icode, valC, valP);

   //The value passed to setInput below will need to be changed
   freg->getpredPC()->setInput(tempPredPC);

   // If the instruction needs register ids, this gets the register ids
   if (needRegIds(icode)) {
      instByte = mem_instance->getByte(f_pc + 1, mem_error);
      getRegIds(instByte, rA, rB, f_pc);
   }

   // If the instruction needs valC, this builds valC
   if (needValC(icode)) {
      valC = buildValC(f_pc, icode);
   }

   // 
   // comments here!!!!!!!!!!!!!!
   //

   //getting the predicted PC and storing it in the temp variable
   tempPredPC = predictPC(icode, valC, valP);

   //The value passed to setInput below will need to be changed
   freg->getpredPC()->setInput(tempPredPC);

   //provide the input values for the D register
   setDInput(dreg, stat, icode, ifun, rA, rB, valC, valP);
   
   calculateControlSignals(ereg, dreg, mreg, 
      dec->getd_srcA(), dec->getd_srcB(), exe->gete_Cnd());
   
   return false;
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register instances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void FetchStage::doClockHigh(PipeReg ** pregs)
{
   F * freg = (F *) pregs[FREG];
   D * dreg = (D *) pregs[DREG];

   if (!F_stallVar)
      freg->getpredPC()->normal();
   else
      freg->getpredPC()->stall();

   if (D_bubbleVar)
      bubbleD(dreg);
   else if (!D_stallVar)
      normalD(dreg);
   else
      stallD(dreg);

}

/* setDInput
 * provides the input to potentially be stored in the D register
 * during doClockHigh
 *
 * @param: dreg - pointer to the D register instance
 * @param: stat - value to be stored in the stat pipeline register within D
 * @param: icode - value to be stored in the icode pipeline register within D
 * @param: ifun - value to be stored in the ifun pipeline register within D
 * @param: rA - value to be stored in the rA pipeline register within D
 * @param: rB - value to be stored in the rB pipeline register within D
 * @param: valC - value to be stored in the valC pipeline register within D
 * @param: valP - value to be stored in the valP pipeline register within D
*/
void FetchStage::setDInput(D * dreg, uint64_t stat, uint64_t icode, 
                           uint64_t ifun, uint64_t rA, uint64_t rB,
                           uint64_t valC, uint64_t valP)
{
   dreg->getstat()->setInput(stat);
   dreg->geticode()->setInput(icode);
   dreg->getifun()->setInput(ifun);
   dreg->getrA()->setInput(rA);
   dreg->getrB()->setInput(rB);
   dreg->getvalC()->setInput(valC);
   dreg->getvalP()->setInput(valP);
}
     
/* selectPC
 * selects the proper PC.
 *
 * @param: fregs - pointer to the F register. 
 * @param: mregs - pointer to the M register. 
 * @param: wregs - pointer to the W register. 
 */
uint64_t FetchStage::selectPC(F * freg, M * mreg, W * wreg)
{
   // returns M_valA if there is a jump and M condition code is not set.
   if (mreg->geticode()->getOutput() == IJXX && !mreg->getCnd()->getOutput())
      return mreg->getvalA()->getOutput();
   // returns W_valM if icode is IRET 
   if (wreg->geticode()->getOutput() == IRET)
      return wreg->getvalM()->getOutput();
   // else returns predPC 
   else
      return freg->getpredPC()->getOutput();
}

/* needRegIds
 *
 * @param: f_icode - icode from the F register. 
 */
bool FetchStage::needRegIds(uint64_t f_icode)
{
   if (f_icode == IRRMOVQ || f_icode == IOPQ || f_icode == IPUSHQ || f_icode == IPOPQ
      || f_icode == IIRMOVQ || f_icode == IRMMOVQ || f_icode == IMRMOVQ) {
      return true;
   }
   return false;
}

/* needValC
 *
 * @param: f_icode - icode from the F register. 
 */
bool FetchStage::needValC(uint64_t f_icode)
{
   if (f_icode == IIRMOVQ || f_icode == IRMMOVQ || f_icode == IMRMOVQ
      || f_icode == IJXX || f_icode == ICALL)  {
      return true;
   }
   return false;
}

/* predictPC 
 *
 * @param: f_icode - icode from the F register. 
 * @param: f_valC - valC from the F register. 
 * @param: f_valP - valP from the F register. 
 */
uint64_t FetchStage::predictPC(uint64_t f_icode, uint64_t f_valC, uint64_t f_valP)
{
   if (f_icode == IJXX || f_icode == ICALL)
      return f_valC;
   return f_valP;
}

/* PCIncrement
 * 
 * @param: f_pc - pc from the F register
 * @param: needRegIds - bool signifying whether register IDs are needed
 * @param: needValC - bool signifying whether valC is needed
*/
uint64_t FetchStage::PCincrement(uint64_t f_pc, bool needRegIds, bool needValC)
{
   // always increment f_pc by one because every instruction has at least one byte.
   f_pc++;

   // instruction is 10 bytes total.
   if (needRegIds) 
   	f_pc++;

   // instruction is 9 bytes total.
   if (needValC) 
   	f_pc += 8;

   return f_pc;
}

/* getRegIds 
 *
 * @param: instByte - byte containing the register ids. 
 * @param: &rA - pointer to the rA register. 
 * @param: &rB - pointer to the rB register. 
 * @param: f_pc - value of the PC.
 */
void FetchStage::getRegIds(uint8_t instByte, uint64_t &rA, uint64_t &rB, uint64_t f_pc)
{
   // Set the rA register by extracting the respective bits from the instruction
   rA = Tools::getBits(instByte, 4, 7);
   // Set the rB register by extracting the respective bits from the instruction
   rB = Tools::getBits(instByte, 0, 3);
}

/* buildValC 
 *
 * @param: f_pc - value of the PC. 
 */
uint64_t FetchStage::buildValC(uint64_t f_pc, uint64_t f_icode)
{
   // Get the instance of Memroy(Singleton) and assign to mem_instance
   Memory * mem_instance = Memory::getInstance();
   // Array to store the bytes of valC from the instruction
   uint8_t byte[8];

   int start;
   int end;
   int i;

   // printf("===============================================\n");
   // printf("f_icode: %x\n", f_icode);

   // if icode is JXX or CALL
   if (f_icode == IJXX || f_icode == ICALL) {
      i = 1;
      start = 1;
      end = 9;
   }
   else {
      i = 2;
      start = 2;
      end = 10;
   }

   // Loops through valC and stores each byte in the array
   for (start; start < end; start++)
   {
      bool getError = false;
      uint8_t numByte = mem_instance->getByte(f_pc + start, getError);
      byte[start - i] = numByte;
   }

   // Buils valC from the byte array
   return Tools::buildLong(byte);
}

/* instructionValid
 * checks to make sure the icode is valid. 
 *
 * @param: f_icode - icode from the F register.
 */
bool FetchStage::instr_valid(uint64_t f_icode)
{
   return f_icode == INOP || f_icode == IHALT || f_icode == IRRMOVQ || f_icode == IIRMOVQ
      || f_icode ==  IRMMOVQ || f_icode == IMRMOVQ || f_icode == IOPQ || f_icode ==IJXX
      || f_icode == ICALL || f_icode == IRET || f_icode == IPUSHQ || f_icode ==IPOPQ;
}

/* f_stat
 * checks to make sure the icode is valid. 
 *
 * @param: f_icode - icode from the F register.
 * @param: mem_error - memory error variable.
 */
uint64_t FetchStage::f_stat(bool mem_error, uint64_t f_icode)
{
   if (mem_error)
      return SADR;
   if (!instr_valid(f_icode))
      return SINS;
   if (f_icode == IHALT)
      return SHLT;
   return SAOK;
}

bool FetchStage::F_stall(uint64_t E_icode, uint64_t E_dstM, uint64_t D_icode, uint64_t M_icode,
   uint64_t d_srcA, uint64_t d_srcB) 
{
   return (E_icode == IMRMOVQ || E_icode == IPOPQ) 
   && (E_dstM == d_srcA || E_dstM == d_srcB)
   || (IRET == D_icode || IRET == E_icode || IRET == M_icode);
}

bool FetchStage::D_stall(uint64_t E_icode, uint64_t E_dstM, uint64_t d_srcA, uint64_t d_srcB) {
   return (E_icode == IMRMOVQ || E_icode == IPOPQ) && (E_dstM == d_srcA || E_dstM == d_srcB);
}

void FetchStage::calculateControlSignals(E * ereg, D * dreg, M * mreg, uint64_t d_srcA, uint64_t d_srcB, uint64_t e_Cnd) {
   uint64_t E_icode = ereg->geticode()->getOutput();
   uint64_t E_dstM = ereg->getdstM()->getOutput();
   uint64_t D_icode = dreg->geticode()->getOutput();
   uint64_t M_icode = mreg->geticode()->getOutput();
   
   F_stallVar = F_stall(E_icode, E_dstM, D_icode, M_icode, d_srcA, d_srcB);
   D_stallVar = D_stall(E_icode, E_dstM, d_srcA, d_srcB);
   D_bubbleVar = D_bubble(E_icode, e_Cnd, E_dstM, D_icode, M_icode, d_srcA, d_srcB);
}

bool FetchStage::D_bubble(uint64_t E_icode, uint64_t e_Cnd, uint64_t E_dstM, uint64_t D_icode, 
   uint64_t M_icode, uint64_t d_srcA, uint64_t d_srcB) 
{
   D_bubbleVar = (E_icode == IJXX && !e_Cnd)
   || !(E_icode == IRMMOVQ || E_icode == IPOPQ)
   && (E_dstM == d_srcA || E_dstM == d_srcB)
   && (IRET == D_icode || IRET == E_icode || IRET == M_icode);
}

void FetchStage::normalD(D* dreg) {
   dreg->getstat()->normal();
   dreg->geticode()->normal();
   dreg->getifun()->normal();
   dreg->getrA()->normal();
   dreg->getrB()->normal();
   dreg->getvalC()->normal();
   dreg->getvalP()->normal();
}

void FetchStage::bubbleD(D * dreg) {
   dreg->getstat()->bubble(SAOK);
   dreg->geticode()->bubble(INOP);
   dreg->getifun()->bubble();
   dreg->getrA()->bubble(RNONE);
   dreg->getrB()->bubble(RNONE);
   dreg->getvalC()->bubble();
   dreg->getvalP()->bubble();
}

void FetchStage::stallD(D * dreg) {
   dreg->getstat()->stall();
   dreg->geticode()->stall();
   dreg->getifun()->stall();
   dreg->getrA()->stall();
   dreg->getrB()->stall();
   dreg->getvalC()->stall();
   dreg->getvalP()->stall();
}
