#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
#pragma ide diagnostic ignored "hicpp-signed-bitwise"
#include <iostream>

#include "Bus.hpp"


Bus::Bus()
{
  cpu.attach(this);
  audio.attach(this);
  display.attach(this);
  controls.attach(this);

  test_complete = false;
  paused = false;
}

uint8_t Bus::read(uint16_t address)
{
  
  if (run_tests) return ram[address];
  
  switch (address)
  {
    // read_byte ROMs
    case 0x0000 ... 0x07FF: return invaders_h[address];
    case 0x0800 ... 0x0FFF: return invaders_g[address & 0x07FF];
    case 0x1000 ... 0x17FF: return invaders_f[address & 0x07FF];
    case 0x1800 ... 0x1FFF: return invaders_e[address & 0x07FF];
      
    // read_byte RAM
    case 0x2000 ... 0x23FF: return si_ram[address & 0x03FF];
      
    // read_byte VRAM
    case 0x2400 ... 0x3FFF: return vram[address - 0x2400];
      
    // read_byte RAM mirror
    case 0x4000 ... 0xFFFF: return si_ram[address & 0x03FF];
    
    default:
      std::cerr << "Error: Unexpected READ address: 0x" << std::hex << address << ". Exiting." << std::endl;
      exit(EXIT_FAILURE);
  }
}

void Bus::write(uint16_t address, uint8_t data)
{
  if (run_tests)
  {
    ram[address] = data;
    return;
  }
  
  switch (address)
  {
    // write_byte RAM
    case 0x2000 ... 0x23FF: si_ram[address & 0x03FF] = data; break;
      
    // write_byte VRAM
    case 0x2400 ... 0x3FFF: vram[address - 0x2400] = data; break;
    
    // write_byte RAM mirror
    case 0x4000 ... 0x43FF: si_ram[address & 0x03FF] = data; break;
      
    // shouldn't get here
    default:
      std::cerr << "Error: Unexpected WRITE address: 0x" << std::hex << address << ". Exiting." << std::endl;
      exit(EXIT_FAILURE);
  }
}

void Bus::load_rom(const std::string& filename)
{
  if (run_tests)
  {
    load_helper(filename.c_str(), 0x0100, 0x0100, ram);
  }
  else
  {
    load_helper(filename.c_str(), 0x0000, 0x0000, ram);
  }
}

void Bus::load_helper(const char *filename, uint16_t offset, uint16_t origin, uint8_t* storage)
{
  FILE* ROMData;
  
  ROMData = fopen(filename, "rb");
  if (ROMData == nullptr)
  {
    std::cerr << "Error. Unable to open ROM file: " << filename << std::endl << "Exiting." << std::endl;
    exit(1);
  }
  
  fseek (ROMData, 0, SEEK_END);
  uint16_t ROMSize = ftell (ROMData);
  fclose (ROMData);
  ROMData = fopen (filename, "rb");
  
  auto Buffer = (uint8_t*) malloc(ROMSize);
  
  if (fread(Buffer, 1, ROMSize, ROMData) != ROMSize) exit(11);
  
  for (int i = 0; i < ROMSize; i++)
    storage[offset + i] = Buffer[i];
  
  cpu.set_PC(origin);
  
  if (run_tests)
  {
    ram[0x0000] = 0xD3;
    ram[0x0001] = 0x00;
    ram[0x0005] = 0xDB;
    ram[0x0006] = 0x00;
    ram[0x0007] = 0xC9;
  }

  free (Buffer);
  fclose (ROMData);
  
}

void Bus::test_helper(const std::string& filename)
{
  load_rom(filename);
  test_complete = false;

  while (!test_complete)
    cpu.tick(1);
}

void Bus::test()
{
  run_tests = true;
  controls.init();

  for (const auto& test_rom : test_roms)
    test_helper(test_rom);

  std::cout << "End of testing. Exit." << std::endl;
  exit(EXIT_SUCCESS);
}

void Bus::play()
{
  run_tests = false;

  display.init();
  audio.init();
  controls.init();

  Uint64 start = 0;
  Uint64 end = 0;
  float dt = 0;

  Uint64 pause_start = 0;
  Uint64 pause_end = 0;
  float pause_dt = 0;

  load_space_invaders();

  while (true)
  {
    if (!paused)
    {
      start = SDL_GetPerformanceCounter();

      controls.handle_input();
      clock();
      display.update();
      clock();
      display.update();

      end = SDL_GetPerformanceCounter();
      dt = (end - start) / static_cast<float>(SDL_GetPerformanceFrequency()) * 1000.0f;

      // Cap to 60 FPS
      SDL_Delay(floor(std::max(0.0, (16.6667 - dt))));
    }
    else  // pause without burdening the CPU
    {
      pause_start = SDL_GetPerformanceCounter();

      controls.handle_input();

      pause_end = SDL_GetPerformanceCounter();
      pause_dt = (pause_end - pause_start) / static_cast<float>(SDL_GetPerformanceFrequency()) * 1000.0f;

      // Cap to 60 FPS
      SDL_Delay(floor(std::max(0.0, (16.6667 - pause_dt))));
    }

    display.draw();
  }
}

void Bus::clock()
{
  uint16_t interrupt_vector = cpu.get_interrupt_vector();

  cpu.tick(HALF_FRAME);
  
  cpu.set_total_cycles(cpu.get_total_cycles() - HALF_FRAME);
  cpu.trigger_interrupt(interrupt_vector);
  interrupt_vector = (interrupt_vector == 0x08) ? 0x10 : 0x08;
  cpu.set_interrupt_vector(interrupt_vector);
}

void Bus::load_space_invaders()
{
  // no offsets for individual rom "chips"
  load_helper("Resources/ROMs/SI/invaders.h", 0x0000, 0x0000, invaders_h);
  load_helper("Resources/ROMs/SI/invaders.g", 0x0000, 0x0000, invaders_g);
  load_helper("Resources/ROMs/SI/invaders.f", 0x0000, 0x0000, invaders_f);
  load_helper("Resources/ROMs/SI/invaders.e", 0x0000, 0x0000, invaders_e);
}
uint8_t Bus::in_port(uint8_t port, uint16_t PC, uint16_t DE)
{
  return controls.in_port(port, PC, DE);
}

void Bus::out_port(uint8_t port, uint8_t data)
{
  controls.out_port(port, data);
}

void Bus::port3_audio()
{
  audio.play(3);
}

void Bus::port5_audio()
{
  audio.play(5);
}


#pragma clang diagnostic pop