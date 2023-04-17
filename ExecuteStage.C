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
#include "ConditionCodes.h"
#include "Tools.h"

// e_dstE made global in order to implement forwarding
uint64_t e_dstE;

// e_valE made global in order to implement forwarding
uint64_t e_valE;

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

   // Sets e_valE to the result of the ALU.  The value of e_valE is then stored in M_valE.
   // Get the value returned from aluFun
   uint64_t alu_fun = aluFun(ereg->getifun()->getOutput(), ereg->geticode()->getOutput());
   // Gets the value of aluA
   uint64_t alu_A = aluA(ereg->geticode()->getOutput(), ereg->getvalA()->getOutput(), 
      ereg->getvalC()->getOutput());
   // Gets the value of aluB
   uint64_t alu_B = aluB(ereg->geticode()->getOutput(), ereg->getvalB()->getOutput());
   // Sets the ALU
   e_valE = ALU(alu_fun, alu_A, alu_B);

   // Sets the Condition Codes (CC)
   if (set_cc(ereg->geticode()->getOutput()))
      CC(e_valE, alu_fun, alu_A, alu_B);

   // Sets e_Cnd
   uint64_t e_Cnd = cond(ereg->geticode()->getOutput(), ereg->getifun()->getOutput());
   
   // Set new dstE to send to M register
   e_dstE = eDstE(ereg->geticode()->getOutput(), ereg->getdstE()->getOutput(), e_Cnd);
   
   // Sets inputs for the M register
   setMInput(mreg, ereg->getstat()->getOutput(), ereg->geticode()->getOutput(), e_Cnd, e_valE, 
      ereg->getvalA()->getOutput(), e_dstE, ereg->getdstM()->getOutput());
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

/* aluA
 * determines the number used for the first operand in ALU
 * 
 * @param E_icode - icode from the ExecuteStage register
 * @param E_valA - valA from the ExecuteStage register
 * @param E_valC - valC from the ExecuteStage register
 * 
 * 
*/
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

/* aluB
 * determines the number used for the second operand in ALU
 * 
 * @param E_icode - icode from the ExecuteStage register
 * @param E_valB - valB from the ExecuteStage register
 * 
 * 
*/
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

/* aluFun
 * determines if the operation to be performed is an OPQ
 * 
 * @param E_ifun - ifun from the ExecuteStage register
 * @param E_icode - icode from the ExecuteStage register
 * 
*/
uint64_t ExecuteStage::aluFun(uint64_t E_ifun, uint64_t E_icode)
{
   if (E_icode == IOPQ)
      return E_ifun;
   else
      return ADDQ;
}

/* set_cc
 * determines if condition codes need to be set
 *
 * @param E_icode - icode from the ExecuteStage register
 * 
*/
bool ExecuteStage::set_cc(uint64_t E_icode)
{
   return E_icode == IOPQ;
}

/*
 * eDstE
 * determines whether to use the dstE from the ExecutionStage Register
 * or the dstE that is changed/calculated depending on a certain condition
 * 
 * @param E_icode - icode from the ExecuteStage register
 * @patam E_dstE - dstE from the ExecuteStage register
 * @param e_Cnd - condition from the execute stage
 * 
*/
uint64_t ExecuteStage::eDstE(uint64_t E_icode, uint64_t E_dstE, uint64_t e_Cnd)
{
   if (E_icode == IRRMOVQ && !e_Cnd)
      return RNONE;
   else
      return E_dstE;
}

/*
 * CC
 * Sets the condition code flags depending on the result from the ALU, ALU Fun, ALU A, and ALU B
 * 
 * @param alu - result from the ALU method/hardware
 * @patam aluFun - result from the aluFun method
 * @param aluA - result from the aluA method
 * @param aluB - result from the aluB method
 * 
*/
void ExecuteStage::CC(uint64_t alu, uint64_t aluFun, uint64_t aluA, uint64_t aluB) {
   // get the instance of the ConditionCodes (Singleton)
   ConditionCodes * cc = ConditionCodes::getInstance();

   //printf("===============================\n");
   //printf("ALU: %d ALU_FN: %d ALU_A: %d ALU_B: %d\n", alu,aluFun, aluA, aluB);
   //printf("===============================\n");
   
   // error variable used for method calls
   bool error = false;

   // Sets SF or ZF flag
   if (alu == 0) {
      cc->setConditionCode(1, ZF, error);
      cc->setConditionCode(0, SF, error);
   }
   else if (Tools::sign(alu) == 0) {
      //printf("**********HERE GT***********\n");
      cc->setConditionCode(0, SF, error);
      cc->setConditionCode(0, ZF, error);
   }
   else if (Tools::sign(alu) == 1) {
      //printf("**********HERE***********\n");
      cc->setConditionCode(1, SF, error);
      cc->setConditionCode(0, ZF, error);
   }

   // Sets OF flag to zero for general case
   cc->setConditionCode(0, OF, error);

   // if opq is add
   if (aluFun == ADDQ) {
      if (Tools::addOverflow(aluA, aluB))
         cc->setConditionCode(1, OF, error);
      else 
         cc->setConditionCode(0, OF, error);
   }

   // if opq is subtract
   if (aluFun == SUBQ) {
      if (Tools::subOverflow(aluA, aluB))
         cc->setConditionCode(1, OF, error);
      else 
         cc->setConditionCode(0, OF, error);
   }

   //printf("===============================\n");
   //printf("SF: %d   ZF: %d OF: %d\n", cc->getConditionCode(SF, error), cc->getConditionCode(ZF, error), cc->getConditionCode(OF, error));
   //printf("===============================\n");
}

/*
 * ALU
 *
 * Calculates the result (using aluA and aluB) of a specified operation based on
 * the result of the alu function (aluFun method)
 * 
 * @patam aluFun - result from the aluFun method
 * @param aluA - result from the aluA method
 * @param aluB - result from the aluB method
 * 
*/
uint64_t ExecuteStage::ALU(uint64_t aluFun, uint64_t aluA, uint64_t aluB) {
   // aluFun is the alu function calcuated based on the icode and ifun
   if (aluFun == ADDQ)
      return aluA + aluB;
   if (aluFun == SUBQ)
      return aluB - aluA;
   if (aluFun == ANDQ)
      return aluA & aluB;
   if (aluFun == XORQ) 
      return aluA ^ aluB;
}

uint64_t ExecuteStage::cond(uint64_t icode, uint64_t ifun) {
   if (icode == IJXX || icode == ICMOVXX) {
      // JLE / CMOVLE
      if (ifun == 1)
         return (SF ^ OF) | ZF;
      // JL / CMOVL
      if (ifun == 2)
         return (SF ^ OF);
      // JE / CMOVE
      if (ifun == 3)
         return ZF;
      // JNE / CMOVNE
      if (ifun == 4)
         return !ZF;
      // JGE / CMOVGE
      if (ifun == 5)
         return !(SF ^ OF);
      // JG / CMOVG
      if (ifun == 6)
         return !(SF ^ OF) & !ZF;
   }
   else 
      return 0;
}

/*
 * get_edstE 
 *
 * returns e_dstE
*/
uint64_t ExecuteStage::get_edstE() {
   return e_dstE;
}

/*
 * get_evalE 
 *
 * returns e_valE
*/
uint64_t ExecuteStage::get_evalE() {
   return e_valE;
}