class ExecuteStage: public Stage
{
   private:
      void setMInput(M * mreg, uint64_t stat, uint64_t icode, uint64_t Cnd, 
                     uint64_t valE, uint64_t valA,
                     uint64_t dstE, uint64_t dstM);
      uint64_t e_dstE;
      uint64_t e_valE;
      bool M_bubble;
   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);
      uint64_t aluA(uint64_t E_icode, uint64_t valA, uint64_t valC);
      uint64_t aluB(uint64_t E_icode, uint64_t valB);
      uint64_t aluFun(uint64_t E_ifun, uint64_t E_icode);
      bool set_cc(uint64_t E_icode, uint64_t m_stat, uint64_t W_stat);
      uint64_t eDstE(uint64_t E_icode, uint64_t E_dstE, uint64_t e_Cnd);
      void CC(uint64_t alu, uint64_t aluFun, uint64_t aluA, uint64_t aluB);
      uint64_t ALU(uint64_t aluFun, uint64_t aluA, uint64_t aluB);
      uint64_t get_edstE();
      uint64_t get_evalE();
      uint64_t cond(uint64_t icode, uint64_t ifun);
      bool calculateControlSignals(uint64_t m_stat, uint64_t W_stat);
};
