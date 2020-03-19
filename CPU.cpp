#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-signed-bitwise"
#include <iostream>
#include <cstdlib>

#include "CPU.hpp"
#include "Bus.hpp"


CPU::CPU()
{
  // flag statuses are indeterminate on powerup and reset, but are set here to simplify debugging via logging
  F = 0x02;
  //F = 0x00;
  
  interrupt_enabled = false;
  interrupt_pending = false;
  interrupt_vector = 0x0008;
}

void CPU::tick(int run_cycles)
{
  running_test = bus->is_running_test();
  if (running_test) bdos_support();
  
  for (int i = 0; i < run_cycles; ++i)
  {
    if ( !running_test && interrupt_pending  && interrupt_enabled == 0)
    {
      interrupt_pending = false;
      interrupt_enabled = false;
    }
    
    opcode = read_imm_byte();
    
    switch (opcode)
    {
      /* NOP */
      case 0x00: case 0x08:  // fallthru intended
      case 0x10: case 0x18:  // fallthru intended
      case 0x20: case 0x28:  // fallthru intended
      case 0x30: case 0x38: cycles(4); break;
      /* LXI */
      case 0x01: cycles(10); BC = read_imm_word(); break;
      case 0x11: cycles(10); DE = read_imm_word(); break;
      case 0x21: cycles(10); HL = read_imm_word(); break;
      case 0x31: cycles(10); SP = read_imm_word(); break;
      /* STAX and STA */
      case 0x02: cycles(7); write_byte(BC, A); break;
      case 0x12: cycles(7); write_byte(DE, A); break;
      case 0x32: cycles(13); write_byte(read_imm_word(), A); break;
      /* LDAX and LDA */
      case 0x0A: cycles(7); A = read_byte(BC); break;
      case 0x1A: cycles(7); A = read_byte(DE); break;
      case 0x3A: cycles(13); A = read_byte(read_imm_word()); break;
      /* SHLD and LHLD */
      case 0x22: cycles(16); write_word(read_imm_word(), HL); break;
      case 0x2A: cycles(16); HL = read_word(read_imm_word()); break;
      /* INX */
      case 0x03: cycles(5); BC++; break;
      case 0x13: cycles(5); DE++; break;
      case 0x23: cycles(5); HL++; break;
      case 0x33: cycles(5); SP++; break;
      /* DAD */
      case 0x09: cycles(10); _dad(BC); break;
      case 0x19: cycles(10); _dad(DE); break;
      case 0x29: cycles(10); _dad(HL); break;
      case 0x39: cycles(10); _dad(SP); break;
      /* DCX */
      case 0x0B: cycles(5); BC--; break;
      case 0x1B: cycles(5); DE--; break;
      case 0x2B: cycles(5); HL--; break;
      case 0x3B: cycles(5); SP--; break;
      /* INR */
      case 0x04: cycles(5); B = _inr(B); break;
      case 0x14: cycles(5); D = _inr(D); break;
      case 0x24: cycles(5); H = _inr(H); break;
      case 0x34: cycles(10); write_byte(HL, _inr(read_byte(HL))); break;
      case 0x0C: cycles(5); C = _inr(C); break;
      case 0x1C: cycles(5); E = _inr(E); break;
      case 0x2C: cycles(5); L = _inr(L); break;
      case 0x3C: cycles(5); A = _inr(A); break;
      /* DCR */
      case 0x05: cycles(5); B = _dcr(B); break;
      case 0x15: cycles(5); D = _dcr(D); break;
      case 0x25: cycles(5); H = _dcr(H); break;
      case 0x35: cycles(10); write_byte(HL, _dcr(read_byte(HL))); break;
      case 0x0D: cycles(5); C = _dcr(C); break;
      case 0x1D: cycles(5); E = _dcr(E); break;
      case 0x2D: cycles(5); L = _dcr(L); break;
      case 0x3D: cycles(5); A = _dcr(A); break;
      /* MVI */
      case 0x06: cycles(7); B = read_imm_byte(); break;
      case 0x16: cycles(7); D = read_imm_byte(); break;
      case 0x26: cycles(7); H = read_imm_byte(); break;
      case 0x36: cycles(10);
      
      write_byte(HL, read_imm_byte());
      break;
      case 0x0E: cycles(7); C = read_imm_byte(); break;
      case 0x1E: cycles(7); E = read_imm_byte(); break;
      case 0x2E: cycles(7); L = read_imm_byte(); break;
      case 0x3E: cycles(7); A = read_imm_byte(); break;
      /* Rotate Accumulator */
      case 0x07: cycles(4); _rlc(A); break;  // RLC
      case 0x17: cycles(4); _ral(A); break;  // RAL
      case 0x0F: cycles(4); _rrc(A); break;  // RRC
      case 0x1F: cycles(4); _rar(A); break;  // RAR
      /* Single Register Instructions */
      case 0x27: cycles(4); _daa(A); break;  // DAA
      case 0x37: cycles(4); set_flag(CF, true); break;  // STC
      case 0x2F: cycles(4); A = ~A; break;  // CMA
      case 0x3F: cycles(4); set_flag(CF, !get_flag(CF)); break;  // CMC
      /* MOV */
      case 0x40: cycles(5); B = B; break;
      case 0x41: cycles(5); B = C; break;
      case 0x42: cycles(5); B = D; break;
      case 0x43: cycles(5); B = E; break;
      case 0x44: cycles(5); B = H; break;
      case 0x45: cycles(5); B = L; break;
      case 0x46: cycles(7); B = read_byte(HL); break;
      case 0x47: cycles(5); B = A; break;
      case 0x48: cycles(5); C = B; break;
      case 0x49: cycles(5); C = C; break;
      case 0x4A: cycles(5); C = D; break;
      case 0x4B: cycles(5); C = E; break;
      case 0x4C: cycles(5); C = H; break;
      case 0x4D: cycles(5); C = L; break;
      case 0x4E: cycles(7); C = read_byte(HL); break;
      case 0x4F: cycles(5); C = A; break;
      case 0x50: cycles(5); D = B; break;
      case 0x51: cycles(5); D = C; break;
      case 0x52: cycles(5); D = D; break;
      case 0x53: cycles(5); D = E; break;
      case 0x54: cycles(5); D = H; break;
      case 0x55: cycles(5); D = L; break;
      case 0x56: cycles(7); D = read_byte(HL); break;
      case 0x57: cycles(5); D = A; break;
      case 0x58: cycles(5); E = B; break;
      case 0x59: cycles(5); E = C; break;
      case 0x5A: cycles(5); E = D; break;
      case 0x5B: cycles(5); E = E; break;
      case 0x5C: cycles(5); E = H; break;
      case 0x5D: cycles(5); E = L; break;
      case 0x5E: cycles(7); E = read_byte(HL); break;
      case 0x5F: cycles(5); E = A; break;
      case 0x60: cycles(5); H = B; break;
      case 0x61: cycles(5); H = C; break;
      case 0x62: cycles(5); H = D; break;
      case 0x63: cycles(5); H = E; break;
      case 0x64: cycles(5); H = H; break;
      case 0x65: cycles(5); H = L; break;
      case 0x66: cycles(7); H = read_byte(HL); break;
      case 0x67: cycles(5); H = A; break;
      case 0x68: cycles(5); L = B; break;
      case 0x69: cycles(5); L = C; break;
      case 0x6A: cycles(5); L = D; break;
      case 0x6B: cycles(5); L = E; break;
      case 0x6C: cycles(5); L = H; break;
      case 0x6D: cycles(5); L = L; break;
      case 0x6E: cycles(7); L = read_byte(HL); break;
      case 0x6F: cycles(5); L = A; break;
      case 0x70: cycles(7); write_byte(HL, B); break;
      case 0x71: cycles(7); write_byte(HL, C); break;
      case 0x72: cycles(7); write_byte(HL, D); break;
      case 0x73: cycles(7); write_byte(HL, E); break;
      case 0x74: cycles(7); write_byte(HL, H); break;
      case 0x75: cycles(7); write_byte(HL, L); break;
      case 0x77: cycles(7); write_byte(HL, A); break;
      case 0x78: cycles(5); A = B; break;
      case 0x79: cycles(5); A = C; break;
      case 0x7A: cycles(5); A = D; break;
      case 0x7B: cycles(5); A = E; break;
      case 0x7C: cycles(5); A = H; break;
      case 0x7D: cycles(5); A = L; break;
      case 0x7E: cycles(7); A = read_byte(HL); break;
      case 0x7F: cycles(5); A = A; break;
      /* HALT */
      case 0x76: cycles(7); _hlt(); break;
      /* ADD */
      case 0x80: cycles(4); _add(B); break;
      case 0x81: cycles(4); _add(C); break;
      case 0x82: cycles(4); _add(D); break;
      case 0x83: cycles(4); _add(E); break;
      case 0x84: cycles(4); _add(H); break;
      case 0x85: cycles(4); _add(L); break;
      case 0x86: cycles(7); _add(read_byte(HL)); break;
      case 0x87: cycles(4); _add(A); break;
      /* ADC */
      case 0x88: cycles(4); _adc(B); break;
      case 0x89: cycles(4); _adc(C); break;
      case 0x8A: cycles(4); _adc(D); break;
      case 0x8B: cycles(4); _adc(E); break;
      case 0x8C: cycles(4); _adc(H); break;
      case 0x8D: cycles(4); _adc(L); break;
      case 0x8E: cycles(7); _adc(read_byte(HL)); break;
      case 0x8F: cycles(4); _adc(A); break;
      /* SUB */
      case 0x90: cycles(4); _sub(B); break;
      case 0x91: cycles(4); _sub(C); break;
      case 0x92: cycles(4); _sub(D); break;
      case 0x93: cycles(4); _sub(E); break;
      case 0x94: cycles(4); _sub(H); break;
      case 0x95: cycles(4); _sub(L); break;
      case 0x96: cycles(7); _sub(read_byte(HL)); break;
      case 0x97: cycles(4); _sub(A); break;
      /* SBB */
      case 0x98: cycles(4); _sbb(B); break;
      case 0x99: cycles(4); _sbb(C); break;
      case 0x9A: cycles(4); _sbb(D); break;
      case 0x9B: cycles(4); _sbb(E); break;
      case 0x9C: cycles(4); _sbb(H); break;
      case 0x9D: cycles(4); _sbb(L); break;
      case 0x9E: cycles(7); _sbb(read_byte(HL)); break;
      case 0x9F: cycles(4); _sbb(A); break;
      /* ANA */
      case 0xA0: cycles(4); _ana(B); break;
      case 0xA1: cycles(4); _ana(C); break;
      case 0xA2: cycles(4); _ana(D); break;
      case 0xA3: cycles(4); _ana(E); break;
      case 0xA4: cycles(4); _ana(H); break;
      case 0xA5: cycles(4); _ana(L); break;
      case 0xA6: cycles(7); _ana(read_byte(HL)); break;
      case 0xA7: cycles(4); _ana(A); break;
      /* XRA */
      case 0xA8: cycles(4); _xra(B); break;
      case 0xA9: cycles(4); _xra(C); break;
      case 0xAA: cycles(4); _xra(D); break;
      case 0xAB: cycles(4); _xra(E); break;
      case 0xAC: cycles(4); _xra(H); break;
      case 0xAD: cycles(4); _xra(L); break;
      case 0xAE: cycles(7); _xra(read_byte(HL)); break;
      case 0xAF: cycles(4); _xra(A); break;
      /* ORA */
      case 0xB0: cycles(4); _ora(B); break;
      case 0xB1: cycles(4); _ora(C); break;
      case 0xB2: cycles(4); _ora(D); break;
      case 0xB3: cycles(4); _ora(E); break;
      case 0xB4: cycles(4); _ora(H); break;
      case 0xB5: cycles(4); _ora(L); break;
      case 0xB6: cycles(7); _ora(read_byte(HL)); break;
      case 0xB7: cycles(4); _ora(A); break;
      /* CMP */
      case 0xB8: cycles(4); _cmp(B); break;
      case 0xB9: cycles(4); _cmp(C); break;
      case 0xBA: cycles(4); _cmp(D); break;
      case 0xBB: cycles(4); _cmp(E); break;
      case 0xBC: cycles(4); _cmp(H); break;
      case 0xBD: cycles(4); _cmp(L); break;
      case 0xBE: cycles(7); _cmp(read_byte(HL)); break;
      case 0xBF: cycles(4); _cmp(A); break;
      /* Return From Subroutine */
      case 0xC0: cycles(5); _conditional_ret(get_flag(ZF) == 0); break;  // RNZ
      case 0xC8: cycles(5); _conditional_ret(get_flag(ZF) == 1); break;  // RZ
      case 0xD0: cycles(5); _conditional_ret(get_flag(CF) == 0); break;  // RNC
      case 0xD8: cycles(5); _conditional_ret(get_flag(CF) == 1); break;  // RC
      case 0xE0: cycles(5); _conditional_ret(get_flag(PF) == 0); break;  // RPO
      case 0xE8: cycles(5); _conditional_ret(get_flag(PF) == 1); break;  // RPE
      case 0xF0: cycles(5); _conditional_ret(get_flag(SF) == 0); break;  // RP
      case 0xF8: cycles(5); _conditional_ret(get_flag(SF) == 1); break;  // RM
      case 0xC9:  // fallthrough intended
      case 0xD9: cycles(10); _ret(); break;  // RET
      /* Stack */
      case 0xC1: cycles(10); BC = pop_stack(); filter_flags(); break;  // POP
      case 0xD1: cycles(10); DE = pop_stack(); filter_flags(); break;  // POP
      case 0xE1: cycles(10); HL = pop_stack(); filter_flags(); break;  // POP
      case 0xF1: cycles(10); PSW = pop_stack(); filter_flags(); break;  // POP
      case 0xC5: cycles(11); push_stack(BC); break;  // PUSH
      case 0xD5: cycles(11); push_stack(DE); break;  // PUSH
      case 0xE5: cycles(11); push_stack(HL); break;  // PUSH
      case 0xF5: cycles(11); push_stack(PSW); break;  // PUSH
      /* Jumps */
      case 0xC2: cycles(10); _conditional_jmp(get_flag(ZF) == 0); break;  // JNZ
      case 0xCA: cycles(10); _conditional_jmp(get_flag(ZF) == 1); break;  // JZ
      case 0xD2: cycles(10); _conditional_jmp(get_flag(CF) == 0); break;  // JNC
      case 0xDA: cycles(10); _conditional_jmp(get_flag(CF) == 1); break;  // JC
      case 0xE2: cycles(10); _conditional_jmp(get_flag(PF) == 0); break;  // JPO
      case 0xEA: cycles(10); _conditional_jmp(get_flag(PF) == 1); break;  // JPE
      case 0xF2: cycles(10); _conditional_jmp(get_flag(SF) == 0); break;  // JP
      case 0xFA: cycles(10); _conditional_jmp(get_flag(SF) == 1); break;  // JM
      case 0xC3:  //fallthrough intended
      case 0xCB: cycles(10); _jmp(); break;  // JMP
      /* Calls */
      case 0xC4: cycles(11); _conditional_call(get_flag(ZF) == 0); break;  // CNZ
      case 0xCC: cycles(11); _conditional_call(get_flag(ZF) == 1); break;  // CZ
      case 0xD4: cycles(11); _conditional_call(get_flag(CF) == 0); break;  // CNC
      case 0xDC: cycles(11); _conditional_call(get_flag(CF) == 1); break;  // CC
      case 0xE4: cycles(11); _conditional_call(get_flag(PF) == 0); break;  // CPO
      case 0xEC: cycles(11); _conditional_call(get_flag(PF) == 1); break;  // CPE
      case 0xF4: cycles(11); _conditional_call(get_flag(SF) == 0); break;  // CP
      case 0xFC: cycles(11); _conditional_call(get_flag(SF) == 1); break;  // CM
      case 0xCD: case 0xDD: case 0xED:  // fallthrough intended
      case 0xFD: cycles(17); _call(read_imm_word()); break;  // CALL
      /* Immediate Instructions */
      case 0xC6: cycles(7); _add(read_imm_byte()); break;  // ADI
      case 0xCE: cycles(7); _adc(read_imm_byte()); break;  // ACI
      case 0xD6: cycles(7); _sub(read_imm_byte()); break;  // SUI
      case 0xDE: cycles(7); _sbb(read_imm_byte()); break;  // SBi
      case 0xE6: cycles(7); _ana(read_imm_byte()); break;  // ANI
      case 0xEE: cycles(7); _xra(read_imm_byte()); break;  // XRI
      case 0xF6: cycles(7); _ora(read_imm_byte()); break;  // ORI
      case 0xFE: cycles(7); _cmp(read_imm_byte()); break;  // CPI
      /* Reset */
      case 0xC7: cycles(11); _rst(0x00); break;  // RST 0
      case 0xCF: cycles(11); _rst(0x08); break;  // RST 1
      case 0xD7: cycles(11); _rst(0x10); break;  // RST 2
      case 0xDF: cycles(11); _rst(0x18); break;  // RST 3
      case 0xE7: cycles(11); _rst(0x20); break;  // RST 4
      case 0xEF: cycles(11); _rst(0x28); break;  // RST 5
      case 0xF7: cycles(11); _rst(0x30); break;  // RST 6
      case 0xFF: cycles(11); _rst(0x38); break;  // RST 7
      /* Exchange */
      case 0xE3: cycles(18); _xthl(); break;  // XTHL
      case 0xEB: cycles(5); std::swap(DE, HL); break;  // XCHG
      /* Load */
      case 0xE9: cycles(5); PC = HL; break;  // PCHL
      case 0xF9: cycles(5); SP = HL; break;  // SPHL
      /* Interrupts */
      case 0xF3: cycles(4); interrupt_enabled = false; break;  // DI
      case 0xFB: cycles(4); interrupt_enabled = true; break;  // EI
      /* IO Ports */
      case 0xD3: cycles(10); _out(read_imm_byte(), A); break;  // OUT
      case 0xDB: cycles(10); A = _in(read_imm_byte()); break;  // IN
    }
  }
}

void CPU::write_byte(uint16_t address, uint8_t data)
{
  bus->write(address, data);
}

uint8_t CPU::read_byte(uint16_t address)
{
  return bus->read(address);
}

uint16_t CPU::read_word(uint16_t address)
{
  return read_byte(address + 1) << 8 | read_byte(address);
}

void CPU::write_word(uint16_t address, uint16_t value)
{
  write_byte(address, value & 0xFF);
  write_byte(address + 1, value >> 8);
}

void CPU::push_stack(uint16_t address)
{
  write_byte(SP - 1, address >> 8);
  write_byte(SP - 2, address & 0xFF);
  SP -= 2;
}

uint16_t CPU::pop_stack()
{
  uint16_t retval = read_word(SP);
  SP += 2;
  return retval;
}

void CPU::bdos_support()
{
  if (PC == P_TERMCPM)
  {
    std::cout << std::endl << "System halted. Test Complete." << std::endl;
    bus->end_test();
  }
  
  if (PC == L_WRITE)
  {
    if (C == C_WRITE)
    {
      std::cout << (char)E << std::flush;
    }
    else if (C == C_WRITESTR)
    {
      for (int i = DE; read_byte(i) != '$'; ++i)
        std::cout << (char) read_byte(i) << std::flush;
    }
  }
}

uint8_t CPU::read_imm_byte()
{
  return read_byte(PC++);
}

uint16_t CPU::read_imm_word()
{
  uint16_t retval = read_word(PC);
  PC += 2;
  return retval;
}

void CPU::cycles(uint8_t cycle_count)
{
  total_cycles += cycle_count;
}

uint8_t CPU::get_flag(CPU::FLAGS flag)
{
  return ((F & flag) > 0) ? 1 : 0;
}

void CPU::set_flag(CPU::FLAGS flag, bool value)
{
  value ? (F |= flag) : (F &= ~flag);
}

uint8_t CPU::_inr(uint8_t reg)
{
  uint8_t value = ++reg;
  
  set_flag(SF, (value & 0x80) != 0);
  set_flag(ZF, value == 0);
  set_flag(HF, (value & 0x0F) == 0);
  set_flag(PF, PARITY_LOOKUP[value]);
  
  return value;
}

uint8_t CPU::_dcr(uint8_t reg)
{
  reg--;
  
  set_flag(SF, (reg & 0x80) != 0);
  set_flag(ZF, reg == 0);
  set_flag(HF, (reg & 0x0F) != 0xF);
  set_flag(PF, PARITY_LOOKUP[reg]);
  
  return reg;
}

void CPU::_rlc(uint8_t reg)
{
  set_flag(CF, A >> 7);
  A = (A << 1) | get_flag(CF);
}

void CPU::_ral(uint8_t reg)
{
  uint8_t prev_CF = get_flag(CF);
  set_flag(CF, A >> 7);
  A = prev_CF | A << 1;
}

void CPU::_rrc(uint8_t reg)
{
  set_flag(CF, A & 0x01);
  A = (A >> 1) | (get_flag(CF) << 7);
}

void CPU::_rar(uint8_t reg)
{
  uint8_t temp = A;
  A = (get_flag(CF) << 7) | (temp >> 1);
  set_flag(CF, (temp & 1) == 1);
}

void CPU::_daa(uint8_t reg)
{
  auto temp = static_cast<uint16_t>(A);
  uint16_t adjust = 0;
  uint8_t value;
  
  if ((A & 0x0F) > 0x9 || get_flag(HF))
    adjust = 0x06;
  
  if (A > 0x99 || get_flag(CF))
  {
    adjust += 0x60;
    set_flag(CF, true);
  }
  
  temp += adjust;
  value = temp & 0xFF;

  set_flag(SF, value >> 7);
  set_flag(HF, ((A ^ adjust ^ value) >> 4) & 0x01);
  set_flag(PF, PARITY_LOOKUP[value]);
  set_flag(ZF, value == 0);
  
  A = value;
}

void CPU::_hlt()
{
  std::cerr << "Halt triggered." << std::endl;
  exit(-1);
}

uint8_t CPU::_add(uint16_t reg)
{
  uint16_t value = static_cast<uint16_t>(A) + reg;
  
  set_flag(SF, (value & 0x80) != 0);
  set_flag(ZF, (value & 0xFF) == 0);
  set_flag(HF, (A ^ value ^ reg) & 0x10);
  set_flag(PF, PARITY_LOOKUP[value & 0xFF]);
  set_flag(CF, value >> 8);
  
  A = value & 0xFF;
  return A;
}

uint8_t CPU::_adc(uint16_t reg)
{
  uint16_t value = static_cast<uint16_t>(A) + reg + get_flag(CF);
  set_flag(SF, (value & 0x80) != 0);
  set_flag(ZF, (value & 0xFF) == 0);
  set_flag(HF, (A ^ value ^ reg) & 0x10);
  set_flag(PF, PARITY_LOOKUP[value & 0xFF]);
  set_flag(CF, value >> 8);
  
  A = value & 0xFF;
  return A;
}

uint8_t CPU::_sub(uint16_t reg)
{
  uint16_t value = static_cast<uint16_t>(A) - reg;
  set_flag(SF, (value & 0x80) != 0);
  set_flag(ZF, (value & 0xFF) == 0);
  set_flag(HF, (~(A ^ value ^ reg) >> 4) & 0x01);
  set_flag(PF, PARITY_LOOKUP[value & 0xFF]);
  set_flag(CF, value >> 8);
  
  A = value & 0xFF;
  return A;
}

uint8_t CPU::_sbb(uint16_t reg)
{
  uint16_t value = static_cast<uint16_t>(A) - reg - get_flag(CF);
  set_flag(SF, (value & 0x80) != 0);
  set_flag(ZF, (value & 0xFF) == 0);
  set_flag(HF, (~(A ^ value ^ reg) >> 4) & 0x01);
  set_flag(PF, PARITY_LOOKUP[value & 0xFF]);
  set_flag(CF, value >> 8);
  
  A = value & 0xFF;
  return A;
}

uint8_t CPU::_ana(uint16_t reg)
{
  uint16_t value = A & reg;
  
  set_flag(SF, value >> 7);
  set_flag(ZF, (value & 0xFF) == 0);
  set_flag(HF,((A | reg) >> 3) & 0x01);
  set_flag(PF, PARITY_LOOKUP[value & 0xFF]);
  set_flag(CF, false);
  
  A = value;
  return A;
}

uint8_t CPU::_xra(uint16_t reg)
{
  A ^= reg;
  set_flag(SF, (A & 0x80) != 0);
  set_flag(ZF, A == 0);
  set_flag(HF, false);
  set_flag(PF, PARITY_LOOKUP[A]);
  set_flag(CF, false);
  
  return A;
}

uint8_t CPU::_ora(uint16_t reg)
{
  A |= reg;
  set_flag(SF, A >> 7);
  set_flag(ZF, A == 0);
  set_flag(HF, false);
  set_flag(PF, PARITY_LOOKUP[A]);
  set_flag(CF, false);
  
  return A;
}

void CPU::_dad(uint16_t reg)
{
  uint32_t result = HL + reg;
  HL = result & 0xFFFF;
  
  set_flag(CF, (result & 0x10000) != 0);
}

uint8_t CPU::_cmp(uint16_t reg)
{
  uint16_t tmp = static_cast<uint16_t>(A) - static_cast<uint16_t>(reg);
  uint8_t value = tmp & 0xFF;
  
  set_flag(SF, value >> 7);
  set_flag(ZF, (value & 0xFF) == 0);
  set_flag(HF, (~(A ^ tmp ^ reg) >> 4) & 0x01);
  set_flag(PF, PARITY_LOOKUP[value]);
  set_flag(CF, tmp >> 8);
  
  return value;
}

void CPU::_conditional_ret(bool condition)
{
  if (condition)
  {
    total_cycles += 6;
    _ret();
  }
}

void CPU::_ret()
{
  PC = pop_stack();
}

void CPU::_jmp()
{
  PC = read_imm_word();
}

void CPU::_conditional_jmp(bool condition)
{
  uint16_t address = read_imm_word();
  if (condition) PC = address;
}

void CPU::_call(uint16_t address)
{
  push_stack(PC);
  PC = address;
}

void CPU::_conditional_call(bool condition)
{
  uint16_t address = read_imm_word();
  if (condition)
  {
    total_cycles += 6;
    _call(address);
  }
}

void CPU::_xthl()
{
  uint16_t tmp_hl = HL;
  
  HL = pop_stack();
  push_stack(tmp_hl);
}

uint8_t CPU::_in(uint8_t port)
{
  return bus->in_port(port, PC, DE);
}

void CPU::_out(uint8_t port, uint8_t reg)
{
    bus->out_port(port, reg);
}

void CPU::filter_flags()
{
  F &= 0xD7;
  F |= 0x02;
}

void CPU::trigger_interrupt(uint16_t address)
{
  if (interrupt_enabled)
  {
    push_stack(PC);
    PC = address & 0xFFFF;
  }
}

void CPU::_rst(uint8_t opcode)
{
  push_stack(PC);
  PC = ((opcode >> 3) & 0x7) << 3;
}

#pragma clang diagnostic pop