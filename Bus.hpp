#ifndef SPACE_INVADERS_BUS_HPP
#define SPACE_INVADERS_BUS_HPP

#include <cstdint>
#include <SDL.h>
#include "CPU.hpp"
#include "Display.hpp"
#include "Audio.hpp"
#include "Controls.hpp"

static constexpr int FPS = 60;
static constexpr int FRAME_DELAY = 1000 / FPS;
static constexpr int CYCLES_PER_MS = 2000;
static constexpr int CYCLES_PER_TIC = CYCLES_PER_MS * FRAME_DELAY;
static constexpr int HALF_FRAME = CYCLES_PER_TIC / 2;

class Bus
{
 public:
  Bus();
  ~Bus() = default;
  
 public:  // Attached devices
  CPU cpu;
  Audio audio;
  Display display = Display("Space Invaders");
  Controls controls;
  
 public:  // IO
  uint8_t read(uint16_t address);
  void write(uint16_t address, uint8_t data);
  void load_rom(const std::string& filename);
  void load_space_invaders();
  uint8_t in_port(uint8_t port, uint16_t PC, uint16_t DE);
  void out_port(uint8_t port, uint8_t data);
  bool is_running_test() { return run_tests; }
  void play();
  void test();
  void test_helper(const std::string& filename);
  void port3_audio();
  void port5_audio();
  void end_test() { test_complete = true; }
  void toggle_pause() { paused = !paused; }

 private:
  bool run_tests{};
  bool test_complete{};
  bool paused;

 private:  // RAM 'n ROM
  uint8_t invaders_h[0x0800]{};  // $0000-$07ff
  uint8_t invaders_g[0x0800]{};  // $0800-$0fff
  uint8_t invaders_f[0x0800]{};  // $1000-$17ff
  uint8_t invaders_e[0x0800]{};  // $1800-$1fff
  
  uint8_t si_ram[0x0400]{};  //$2000-$23FF, mirrored from $4000
  uint8_t vram[0x1C00]{};    //$2400-$3FFF
  
  uint8_t ram[0x10000]{};    // 64K used for test roms
  
 private:  // helper functions
  void load_helper(const char *filename, uint16_t offset = 0x0000, uint16_t origin = 0x0000, uint8_t* storage = nullptr);
  void clock();

  const std::string test_roms[5] =
      {
          "Resources/ROMs/Test/8080PRE.COM",
          "Resources/ROMs/Test/cpudiag.bin",
          "Resources/ROMs/Test/TST8080.COM",
          "Resources/ROMs/Test/CPUTEST.COM",
          "Resources/ROMs/Test/8080EXM.COM"
      };
};

#endif
