#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "Instructions.h"
#include "Memory.h"
#include "PipeReg.h"
#include "E.h"
#include "M.h"
#include "W.h"
#include "Stage.h"
#include "MemoryStage.h"
#include "Status.h"
#include "Debug.h"


/*
 * doClockLow:
 * Performs the Memory stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool MemoryStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
   M * mreg = (M *) pregs[MREG];
   W * wreg = (W *) pregs[WREG];

   Memory * mem_instance = Memory::getInstance();

   uint64_t addr = mem_addr(mreg);

   m_valM = 0;
   if (mem_read(mreg))
   {
      bool mem_error = false;
      m_valM = mem_instance->getLong(addr, mem_error);
   }

   if (mem_write(mreg))
   {
      bool write_error = false;
      mem_instance->putLong(mreg->getvalA()->getOutput(), addr, write_error);
      
   }

   // sets input of W register
   setWInput(wreg, mreg->getstat()->getOutput(), mreg->geticode()->getOutput(), mreg->getvalE()->getOutput(),
   	m_valM, mreg->getdstE()->getOutput(), mreg->getdstM()->getOutput());
   return false;
}


/* doClockHigh
 * applies the appropriate control signal to the W
 * register instance
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void MemoryStage::doClockHigh(PipeReg ** pregs)
{
   W * wreg = (W *) pregs[WREG];

   wreg->getstat()->normal();
   wreg->geticode()->normal();
   wreg->getvalE()->normal();
   wreg->getvalM()->normal();
   wreg->getdstE()->normal();
   wreg->getdstM()->normal();

}

/* setWInput
 * provides the input to potentially be stored in the W register
 * during doClockHigh
 * 
 * @params: wreg - pointer to the writeback stage register
 * @params: stat - stat from the memory stage
 * @params: icode - icode from the memory stage register
 * @params: valE - valE from the memory stage register
 * @params: valM - valE from the memory stage register
 * @params: dstE - dstE from the memory stage register
 * @params: dstM - dstM from the memory stage register
*/
void MemoryStage::setWInput(W * wreg, uint64_t stat, uint64_t icode, uint64_t valE, uint64_t valM,
	uint64_t dstE, uint64_t dstM)
{
   wreg->getstat()->setInput(stat);
   wreg->geticode()->setInput(icode);
   wreg->getvalE()->setInput(valE);
   wreg->getvalM()->setInput(valM);
   wreg->getdstE()->setInput(dstE);
   wreg->getdstM()->setInput(dstM);
}

uint64_t MemoryStage::mem_addr(M * mreg)
{
   uint64_t M_icode = mreg->geticode()->getOutput();

   if (M_icode == IRMMOVQ || M_icode == IPUSHQ || M_icode == ICALL 
      || M_icode == IMRMOVQ)
      return mreg->getvalE()->getOutput();
   if (M_icode == IPOPQ || M_icode == IRET)
      return mreg->getvalA()->getOutput();
   else
      return 0;
}

bool MemoryStage::mem_read(M * mreg)
{
   uint64_t M_icode = mreg->geticode()->getOutput();

   return M_icode == IMRMOVQ || M_icode == IPOPQ || M_icode == IRET;

}

bool MemoryStage::mem_write(M * mreg)
{
   uint64_t M_icode = mreg->geticode()->getOutput();

   return M_icode == IRMMOVQ || M_icode == IPUSHQ || M_icode == ICALL;

}

uint64_t MemoryStage::getm_valM()
{
   return m_valM;
}