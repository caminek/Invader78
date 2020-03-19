#ifndef SPACE_INVADERS_CONTROLS_HPP
#define SPACE_INVADERS_CONTROLS_HPP

#include <SDL.h>
#include <vector>

class Bus;

class Controls
{
 public:
  Controls();
  ~Controls();
  
  void init();
  void attach(Bus *b) { bus = b; }
  void handle_input();
  uint8_t in_port(uint8_t port, uint16_t PC, uint16_t DE);
  void out_port(uint8_t port, uint8_t data);
  static constexpr int CONTROLLER_DEAD_ZONE = 8000;
  static constexpr int GAME_RUNNING = 0x01;
  
 private:
  Bus *bus = nullptr;
  std::vector<SDL_GameController*> controllers;
  
  bool run_tests;
  uint8_t shift_offset, shift0, shift1;
  
  uint8_t read(uint16_t address);
  void write(uint16_t address, uint8_t data);
  uint8_t si_in_helper(uint8_t port);
  void cpm_in_helper(uint16_t PC, uint16_t DE);
  void init_controller(int id);

  // https://computerarcheology.com/Arcade/SpaceInvaders/Hardware.html
  union
  {
    uint16_t value;
    struct
    {
      uint8_t DIP4 : 1;   // Seems to be self-test-request read_byte at power up
      uint8_t u1 : 1;     // Always 1
      uint8_t u2 : 1;     // Always 1
      uint8_t u3 : 1;     // Always 1
      uint8_t fire : 1;   // Fire
      uint8_t left : 1;   // Left
      uint8_t right : 1;  // Right
      uint8_t unk : 1;    // ? tied to demux port 7 ?
    };
  } port_0{};
  
  union
  {
    uint16_t value;
    struct
    {
      uint8_t credit : 1;    // CREDIT (1 if deposit)
      uint8_t p2 : 1;        // 2P start (1 if pressed)
      uint8_t p1 : 1;        // 1P start (1 if pressed)
      uint8_t u3 : 1;        // Always 1
      uint8_t p1_shot : 1;   // 1P shot (1 if pressed)
      uint8_t p1_left : 1;   // 1P left (1 if pressed)
      uint8_t p1_right : 1;  // 1P right (1 if pressed)
      uint8_t u7 : 1;        // Not connected
    };
  } port_1{};
  
  union
  {
    uint16_t value;
    struct
    {
      uint8_t DIP3 : 1;      // 00 = 3 ships  10 = 5 ships (0 for 3, 1 for 5 in the emulator)
      uint8_t DIP5 : 1;      // DIP5 01 = 4 ships  11 = 6 ships (0 for 4, 1 for 6 in the emulator)
      uint8_t tilt : 1;      // Tilt
      uint8_t DIP6 : 1;      // DIP6 0 = extra ship at 1500, 1 = extra ship at 1000
      uint8_t p2_shot : 1;   // P2 shot (1 if pressed)
      uint8_t p2_left : 1;   // P2 left (1 if pressed)
      uint8_t p2_right : 1;  // P2 right (1 if pressed)
      uint8_t DIP7 : 1;      // DIP7 Coin info displayed in demo screen 0=ON
    };
  } port_2{};
  
  enum CONTROLLER_BUTTON
  {
    button_a = 0,
    button_b,
    button_x,
    button_y,
    back,
    start = 6,
    leftstick_click,
    rightstick_click,
    shoulder_left,
    shoulder_right,
    d_up,
    d_down,
    d_left,
    d_right
  };
  
  enum CONTROLLER_AXIS
  {
    leftstick_x = 0,
    leftstick_y,
    rightstick_x,
    rightstick_y,
    left_trigger,
    right_trigger
  };
};

#endif
