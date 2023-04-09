#include <string>
#include <cstdint>
#include "Instructions.h"
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "M.h"
#include "W.h"
#include "Stage.h"
#include "WritebackStage.h"
#include "Status.h"
#include "Debug.h"

/*
 * doClockLow:
 * Performs the Writeback stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool WritebackStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
    W * wreg = (W *) pregs[WREG];
    // if icode is halt instruction then return true
    if (wreg->geticode()->getOutput() == IHALT)
        return true;
    else
        return false;
}


void WritebackStage::doClockHigh(PipeReg ** pregs)
{
    W * wreg = (W *) pregs[WREG];
    // Use the RegisterFile::writeRegister method to write the value of W_valE to R[W_dstE].
    RegisterFile * regFile = RegisterFile::getInstance();
    // Temporary boolean variable for errors
    bool error = false;
    
    regFile->writeRegister(wreg->getvalE()->getOutput(), wreg->getdstE()->getOutput(), error);
}
