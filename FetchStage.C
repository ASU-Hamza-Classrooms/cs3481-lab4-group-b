#include <string>
#include <cstdint>
#include "Instructions.h"
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "M.h"
#include "W.h"
#include "Stage.h"
#include "FetchStage.h"
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
   M * mreg = (M *) pregs[MREG];
   W * wreg = (W *) pregs[WREG];
   uint64_t f_pc = 0, icode = 0, ifun = 0, valC = 0, valP = 0;
   uint64_t rA = RNONE, rB = RNONE, stat = SAOK;

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
   //declares a tempPredPC to hold the values of predictPC
   uint64_t tempPredPC = 1;
   //as long as there is no mem_error
   if (!mem_error)
   {
      //getting the icode from instruction byte
      icode = Tools::getBits(instByte, 4, 7);
      //getting the ifun from the instruction byte
      ifun = Tools::getBits(instByte, 0, 3);
      //setting valP from PCIncrement
      valP = PCincrement(f_pc, needRegIds(icode), needValC(icode));
      //getting the predicted PC and storing it in the temp variable
      uint64_t tempPredPC = predictPC(icode, valC, valP);
   }
   //The value passed to setInput below will need to be changed
   freg->getpredPC()->setInput(tempPredPC);

   //provide the input values for the D register
   setDInput(dreg, stat, icode, ifun, rA, rB, valC, valP);
   return false;
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void FetchStage::doClockHigh(PipeReg ** pregs)
{
   F * freg = (F *) pregs[FREG];
   D * dreg = (D *) pregs[DREG];

   freg->getpredPC()->normal();
   dreg->getstat()->normal();
   dreg->geticode()->normal();
   dreg->getifun()->normal();
   dreg->getrA()->normal();
   dreg->getrB()->normal();
   dreg->getvalC()->normal();
   dreg->getvalP()->normal();
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

uint64_t FetchStage::PCincrement(uint64_t f_pc, bool needRegIds, bool needValC)
{

}

