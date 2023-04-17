class MemoryStage: public Stage
{
   private:
      void setWInput(W * wreg, uint64_t stat, uint64_t icode, uint64_t valE, 
                     uint64_t valM, uint64_t dstE,
                     uint64_t dstM);
   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);
      uint64_t mem_addr(M * mreg);
      bool mem_read(M * mreg);
      bool mem_write(M * mreg);
      uint64_t getm_valM();
};
