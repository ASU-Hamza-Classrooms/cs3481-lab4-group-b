#include <string>
#include <cstdint>
#include "Instructions.h"
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "D.h"
#include "E.h"
#include "M.h"
#include "W.h"
#include "Stage.h"
#include "ExecuteStage.h"
#include "Status.h"
#include "Debug.h"


/*
 * doClockLow:
 * Performs the Execute stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool ExecuteStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{ 
   E * ereg = (E *) pregs[EREG];
   M * mreg = (M *) pregs[MREG];

   // Sets e_valE to E_valC.  The value of e_valE is then stored in M_valE.
   uint64_t e_valE = ereg->getvalC()->getOutput();

   // Sets inputs for the M register
   setMInput(mreg, ereg->getstat()->getOutput(), ereg->geticode()->getOutput(), 0, e_valE, 
      ereg->getvalA()->getOutput(), ereg->getdstE()->getOutput(), ereg->getdstM()->getOutput());
   return false;
}


/* doClockHigh
 * applies the appropriate control signal to the M
 * instance
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void ExecuteStage::doClockHigh(PipeReg ** pregs)
{
   M * mreg = (M *) pregs[MREG];

   
   mreg->getstat()->normal(); 
   mreg->geticode()->normal();
   mreg->getCnd()->normal();
   mreg->getvalE()->normal();
   mreg->getvalA()->normal();
   mreg->getdstE()->normal();
   mreg->getdstM()->normal();
}

/* setMInput
 * provides the input to potentially be stored in the M register
 * during doClockHigh
 * 
 * @params: mreg - pointer to the memory stage register
 * @params: stat - stat from execute stage register
 * @params: icode - icode from the execute stage register
 * @params: Cnd - Cnd from the execute stage
 * @params: valE - valE from the execute stage
 * @params: valA - valA from the execute stage register
 * @params: dstE - dstE from the execute stage
 * @params: dstM - dstM from the execute stage register
*/
void ExecuteStage::setMInput(M * mreg, uint64_t stat, uint64_t icode, uint64_t Cnd, 
	uint64_t valE, uint64_t valA, uint64_t dstE, uint64_t dstM)
{
   mreg->getstat()->setInput(stat); 
   mreg->geticode()->setInput(icode);
   mreg->getCnd()->setInput(Cnd);
   mreg->getvalE()->setInput(valE);
   mreg->getvalA()->setInput(valA);
   mreg->getdstE()->setInput(dstE);
   mreg->getdstM()->setInput(dstM);
}

uint64_t ExecuteStage::aluA(uint64_t E_icode, uint64_t E_valA, uint64_t E_valC)
{
   if (E_icode == IRRMOVQ || E_icode == IOPQ)
      return E_valA;
   if (E_icode == IIRMOVQ || E_icode == IRMMOVQ || E_icode == IMRMOVQ)
      return E_valC;
   if (E_icode == ICALL || E_icode == IPUSHQ)
      return -8;
   if (E_icode == IRET || E_icode == IPOPQ)
      return 8;
   else
      return 0;
}

uint64_t ExecuteStage::aluB(uint64_t E_icode, uint64_t E_valB)
{
   if (E_icode == IRMMOVQ || E_icode == IMRMOVQ || E_icode == IOPQ || E_icode == ICALL ||
      E_icode == IPUSHQ || E_icode == IRET || E_icode == IPOPQ)
         return E_valB;
   if (E_icode == IRRMOVQ || E_icode == IIRMOVQ)
      return 0;
   else
      return 0;
}

uint64_t ExecuteStage::aluFun(uint64_t E_ifun, uint64_t E_icode)
{
   if (E_icode == IOPQ)
      return E_ifun;
   else
      return ADDQ;
}

bool ExecuteStage::set_cc(uint64_t E_icode)
{
   return E_icode == IOPQ;
}

uint64_t ExecuteStage::eDstE(uint64_t E_icode, uint64_t E_dstE, uint64_t e_Cnd)
{
   if (E_icode == IRRMOVQ && !e_Cnd)
      return RNONE;
   else
      return E_dstE;
}

