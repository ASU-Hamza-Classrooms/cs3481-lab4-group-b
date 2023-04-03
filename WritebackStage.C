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

}
