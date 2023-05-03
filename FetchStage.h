//class to perform the combinational logic of
//the Fetch stage
class FetchStage: public Stage
{
   private:
      void setDInput(D * dreg, uint64_t stat, uint64_t icode, uint64_t ifun, 
                     uint64_t rA, uint64_t rB,
                     uint64_t valC, uint64_t valP);
      bool F_stallVar;
      bool D_stallVar;
      bool D_bubbleVar;
   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);
      uint64_t selectPC(F * freg, M * mreg, W * wreg);
      bool needRegIds(uint64_t f_icode);
      bool needValC(uint64_t f_icode);
      uint64_t predictPC(uint64_t f_icode, uint64_t f_valC, uint64_t f_valP);
      uint64_t PCincrement(uint64_t f_pc, bool needRegIds, bool needValC);
      void getRegIds(uint8_t instByte, uint64_t &rA, uint64_t &rB, uint64_t f_pc);
      uint64_t buildValC(uint64_t f_pc, uint64_t f_icode);
      bool instr_valid(uint64_t f_icode);
      uint64_t f_stat(bool mem_error, uint64_t f_icode);
      bool F_stall(uint64_t E_icode, uint64_t E_dstM, uint64_t d_srcA, uint64_t d_srcB);
      bool D_stall(uint64_t E_icode, uint64_t E_dstM, uint64_t d_srcA, uint64_t d_srcB);
      void calculateControlSignals(uint64_t E_icode, uint64_t E_dstM, uint64_t d_srcA, uint64_t d_srcB, uint64_t e_Cnd);
      bool D_bubble(uint64_t E_icode, uint64_t e_Cnd);
      void normalD(D * dreg);
      void bubbleD(D * dreg);
      void stallD(D * dreg);
};
