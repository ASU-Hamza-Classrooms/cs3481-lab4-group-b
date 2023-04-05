#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "Instructions.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "E.h"
#include "Stage.h"
#include "DecodeStage.h"
#include "Status.h"
#include "Debug.h"

/* doClockLow
 * 
 * Performs the Decode stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @params: pregs - pipe register array pointer
 * @params: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
*/
bool DecodeStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
   D * dreg = (D *) pregs[DREG];
   E * ereg = (E *) pregs[EREG];

   RegisterFile * regFile = RegisterFile::getInstance();
   //setting icode, srcA, srcB
   uint64_t icode = dreg->geticode()->getOutput();
   uint64_t srcA = getsrcA(icode, dreg->getrA()->getOutput());
   uint64_t srcB = getsrcB(icode, dreg->getrB()->getOutput());
   //setting dstE, dstM
   uint64_t dstE = getdstE(icode, dreg->getrB()->getOutput());
   uint64_t dstM = getdstM(icode, dreg->getrA()->getOutput());


   bool error = false;
   uint64_t d_rvalA = regFile->readRegister(srcA, error);
   uint64_t d_rvalB = regFile ->readRegister(srcB, error);

   selFwdA(srcA, d_rvalA);
   fwdB(srcB, d_rvalB);

   setEInput(ereg, dreg->getstat()->getOutput(), dreg->geticode()->getOutput(), dreg->getifun()->getOutput(), 
   	dreg->getvalC()->getOutput(), d_rvalA , d_rvalB, dstE, dstM, srcA, srcB);
   return false;
}


/* doClockHigh
 * applies the appropriate control signal to the E
 * instances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void DecodeStage::doClockHigh(PipeReg ** pregs)
{
   E * ereg = (E *) pregs[EREG];

   ereg->getstat()->normal();
   ereg->geticode()->normal();
   ereg->getifun()->normal();
   ereg->getvalC()->normal();
   ereg->getvalA()->normal();
   ereg->getvalB()->normal();
   ereg->getdstE()->normal();
   ereg->getdstM()->normal();
   ereg->getsrcA()->normal();
   ereg->getsrcB()->normal();
}


/* setEInput
 * provides the input to potentially be stored in the E register
 * during doClockHigh
 *
 * @param: ereg - pointer to the E register instance
 * @param: stat - value to be stored in the stat pipeline register within E
 * @param: icode - value to be stored in the icode pipeline register within E
 * @param: ifun - value to be stored in the ifun pipeline register within E
 * @param: valC - value to be stored in the valC pipeline register within E
 * @param: valA - value to be stored in the valA pipeline register within E
 * @param: valB - value to be stored in the valB pipeline register within E
 * @param: dstE - value to be stored in the dstE pipeline register within E
 * @param: dstM - value to be stored in the dstM pipeline register within E
 * @param: srcA - value to be stored in the srcA pipeline register within E
 * @param: srcB - value to be stored in the srcB pipeline register within E
*/
void DecodeStage::setEInput(E * ereg, uint64_t stat, uint64_t icode, 
                           uint64_t ifun, uint64_t valC, uint64_t valA,
                           uint64_t valB, uint64_t dstE, uint64_t dstM, 
			   uint64_t srcA, uint64_t srcB)
{
   ereg->getstat()->setInput(stat);
   ereg->geticode()->setInput(icode);
   ereg->getifun()->setInput(ifun);
   ereg->getvalC()->setInput(valC);
   ereg->getvalA()->setInput(valA);
   ereg->getvalB()->setInput(valB);
   ereg->getdstE()->setInput(dstE);
   ereg->getdstM()->setInput(dstM);
   ereg->getsrcA()->setInput(srcA);
   ereg->getsrcB()->setInput(srcB);
}

uint64_t DecodeStage::getsrcA(uint64_t D_icode, uint64_t D_rA)
{
   if (D_icode == IRRMOVQ || D_icode == IRMMOVQ || D_icode == IOPQ || D_icode == IPUSHQ)
      return D_rA;
   if (D_icode == IPOPQ || D_icode == IRET)
      return RSP;
   else
      return RNONE;
}

uint64_t DecodeStage::getsrcB(uint64_t D_icode, uint64_t D_rB)
{
   if (D_icode == IOPQ || D_icode == IRMMOVQ || D_icode == IMRMOVQ )
      return D_rB;
   if (D_icode == IPUSHQ ||D_icode == IPOPQ || D_icode == ICALL || D_icode == IRET)
      return RSP;
   else
      return RNONE;
}

uint64_t DecodeStage::getdstE(uint64_t D_icode, uint64_t D_rB)
{
   if (D_icode == IRRMOVQ || D_icode == IIRMOVQ || D_icode == IOPQ )
      return D_rB;
   if (D_icode == IPUSHQ ||D_icode == IPOPQ || D_icode == ICALL || D_icode == IRET)
      return RSP;
   else
      return RNONE;
}

uint64_t DecodeStage::getdstM(uint64_t D_icode, uint64_t D_rA)
{
   if (D_icode == IMRMOVQ || D_icode == IPOPQ)
      return D_rA;
   else
      return RNONE;
}

uint64_t DecodeStage::selFwdA(uint64_t d_srcA, uint64_t d_rvalA)
{
   return d_rvalA;
}

uint64_t DecodeStage::fwdB(uint64_t d_srcB, uint64_t d_rvalB)
{
   return d_rvalB;
}