#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-signed-bitwise"
#ifndef SPACE_INVADERS_CPU_HPP
#define SPACE_INVADERS_CPU_HPP

#include <cstdint>

class Bus;

class CPU
{
 public:
  CPU();
  ~CPU() = default;
  
  void attach(Bus *b) { bus = b; }
  void tick(int run_cycles);
  void set_PC(uint16_t address) { PC = address; }
  void trigger_interrupt (uint16_t address);
  auto get_total_cycles() { return total_cycles; }
  void set_total_cycles(uint32_t set_cycles) { total_cycles = set_cycles; }
  auto get_interrupt_vector() { return interrupt_vector; }
  void set_interrupt_vector(uint32_t set_vector) { interrupt_vector = set_vector; }
  uint8_t get_A() { return A; }
  
 private:
  Bus *bus = nullptr;
  uint8_t opcode;
  bool running_test;
  bool interrupt_enabled;
  bool interrupt_pending;
  
  uint8_t interrupt_vector;
  uint16_t PC;
  uint16_t SP;
  uint32_t total_cycles;
  
  
  uint8_t read_byte(uint16_t address);
  void write_byte(uint16_t address, uint8_t data);
  
  uint16_t read_word(uint16_t address);
  void write_word(uint16_t address, uint16_t value);
  
  uint8_t read_imm_byte();
  uint16_t read_imm_word();
  
  uint16_t pop_stack();
  void push_stack(uint16_t address);
  
  void bdos_support();
  void cycles(uint8_t cycle_count);
  
 private:  // opcode functions
  uint8_t _inr(uint8_t reg);
  uint8_t _dcr(uint8_t reg);
  void _rlc(uint8_t reg);
  void _ral(uint8_t reg);
  void _rrc(uint8_t reg);
  void _rar(uint8_t reg);
  void _daa(uint8_t reg);
  void _hlt();
  void _dad(uint16_t reg);
  uint8_t _add(uint16_t reg);
  uint8_t _adc(uint16_t reg);
  uint8_t _sub(uint16_t reg);
  uint8_t _sbb(uint16_t reg);
  uint8_t _ana(uint16_t reg);
  uint8_t _xra(uint16_t reg);
  uint8_t _ora(uint16_t reg);
  uint8_t _cmp(uint16_t reg);  // subtraction without changing A
  void _ret();
  void _conditional_ret(bool condition);
  void _jmp();
  void _conditional_jmp(bool condition);
  void _call(uint16_t address);
  void _conditional_call(bool condition);
  void _xthl();
  void _rst(uint8_t opcode);
  
  // Implementation specific
  uint8_t _in(uint8_t port);
  void _out(uint8_t port, uint8_t reg);
  void filter_flags();
  
 private:  // CP/M functionality used in test ROMs
  // https://seasip.info/Cpm/bdos.html
  static constexpr uint16_t P_TERMCPM = 0x0000;
  static constexpr uint16_t C_WRITE = 0x0002;
  static constexpr uint16_t L_WRITE = 0x0005;
  static constexpr uint16_t C_WRITESTR = 0x0009;
 
 private:  // flags
  enum FLAGS
  {
    SF  = (1 << 7),  // Sign Flag
    ZF  = (1 << 6),  // Zero Flag
    u6  = (1 << 5),  // Unused, always zero
    HF  = (1 << 4),  // Auxiliary Carry or Half Carry
    u3  = (1 << 3),  // Unused, always zero
    PF  = (1 << 2),  // Parity Flag
    u1  = (1 << 1),  // Unused, always one
    CF  = (1 << 0),  // Carry Flag
  };
  
  uint8_t get_flag(FLAGS flag);
  void set_flag(FLAGS flag, bool value);
 

 private:  // registers
  union
  {
    uint16_t PSW;
    struct
    {
      uint8_t F;
      uint8_t A;
    };
  };
  union
  {
    uint16_t BC;
    struct
    {
      uint8_t C;
      uint8_t B;
    };
  };
  union
  {
    uint16_t DE;
    struct
    {
      uint8_t E;
      uint8_t D;
    };
  };
  union
  {
    uint16_t HL;
    struct
    {
      uint8_t L;
      uint8_t H;
    };
  };
  
  const bool PARITY_LOOKUP[0x100] =
    {
      true, false, false, true, false, true, true, false, false, true, true, false, true, false, false, true,
      false, true, true, false, true, false, false, true, true, false, false, true, false, true, true, false,
      false, true, true, false, true, false, false, true, true, false, false, true, false, true, true, false,
      true, false, false, true, false, true, true, false, false, true, true, false, true, false, false, true,
      false, true, true, false, true, false, false, true, true, false, false, true, false, true, true, false,
      true, false, false, true, false, true, true, false, false, true, true, false, true, false, false, true,
      true, false, false, true, false, true, true, false, false, true, true, false, true, false, false, true,
      false, true, true, false, true, false, false, true, true, false, false, true, false, true, true, false,
      false, true, true, false, true, false, false, true, true, false, false, true, false, true, true, false,
      true, false, false, true, false, true, true, false, false, true, true, false, true, false, false, true,
      true, false, false, true, false, true, true, false, false, true, true, false, true, false, false, true,
      false, true, true, false, true, false, false, true, true, false, false, true, false, true, true, false,
      true, false, false, true, false, true, true, false, false, true, true, false, true, false, false, true,
      false, true, true, false, true, false, false, true, true, false, false, true, false, true, true, false,
      false, true, true, false, true, false, false, true, true, false, false, true, false, true, true, false,
      true, false, false, true, false, true, true, false, false, true, true, false, true, false, false, true
    };
};

#endif

#pragma clang diagnostic pop