#include <string>
#include <cstdint>
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



bool ExecuteStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{ 
   E * ereg = (E *) pregs[EREG];
   M * mreg = (M *) pregs[MREG];
   setMInput(mreg, ereg->getstat()->getOutput(), ereg->geticode()->getOutput(), 0, 0, ereg->getvalA()->getOutput(),
   	RNONE, ereg->getdstM()->getOutput());
   return false;
}



void ExecuteStage::doClockHigh(PipeReg ** pregs)
{
   E * ereg = (E *) pregs[EREG];
   M * mreg = (M *) pregs[MREG];

   
   mreg->getstat()->normal(); 
   mreg->geticode()->normal();
   mreg->getCnd()->normal();
   mreg->getvalE()->normal();
   mreg->getvalA()->normal();
   mreg->getdstE()->normal();
   mreg->getdstM()->normal();
}


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
