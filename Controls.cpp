#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-signed-bitwise"
#include <iostream>
#include "Controls.hpp"
#include "Bus.hpp"

//Keyboard
#define QUIT_GAME SDL_SCANCODE_ESCAPE
#define PAUSE_GAME SDL_SCANCODE_PAUSE
#define MUTE_AUDIO SDL_SCANCODE_M
#define INSERT_COIN SDL_SCANCODE_RETURN
#define P2_START SDL_SCANCODE_2
#define P1_START SDL_SCANCODE_1
#define TILT SDL_SCANCODE_END
#define PLAYER_SHOOT SDL_SCANCODE_SPACE
#define PLAYER_MOVE_LEFT SDL_SCANCODE_A
#define PLAYER_MOVE_RIGHT SDL_SCANCODE_D

//Cheats
#define CHEAT_SPAWN_UFO SDL_SCANCODE_F12
#define CHEAT_UFO_HIT SDL_SCANCODE_F11

Controls::Controls()
{
  shift_offset = 0x00;
  shift0 = shift1 = 0x00;
  
  port_0.value = 0x70;
  port_1.value = 0x08;
  port_2.value = 0x00;
}

Controls::~Controls()
{
  for (auto& controller : controllers)
    SDL_GameControllerClose(controller);
}


void Controls::init()
{
  if (bus->is_running_test())
  {
    run_tests = true;
    return;
  }

  if(SDL_Init(SDL_INIT_GAMECONTROLLER) < 0)
  {
    std::cout << "Error: Could not initialize SDL Game Controller.  Exiting. " << std::endl << SDL_GetError() << std::endl;
    exit(1);
  }
}

void Controls::handle_input()
{
  SDL_Event event;
  
  while (SDL_PollEvent(&event) != 0)
  {
    switch (event.type)
    {
      case SDL_QUIT: exit(0);
      case SDL_KEYDOWN:
        switch(event.key.keysym.scancode)
        {
          case QUIT_GAME:
            SDL_Event quit_event;
            quit_event.type = SDL_QUIT;
            SDL_PushEvent(&quit_event);
            break;
          case INSERT_COIN: port_1.credit = 1; break;
          case P2_START:
            port_1.p2 = 1;
            break;
          case P1_START:
            port_1.p1 = 1;
            break;
          case TILT: port_2.tilt = 1; break;
          case PLAYER_SHOOT:
            port_1.p1_shot = 1;
            port_2.p2_shot = 1;
            break;
          case PLAYER_MOVE_LEFT:  // fallthrough intended
          case SDL_SCANCODE_LEFT:
            port_1.p1_left = 1;
            port_2.p2_left = 1;
            break;
          case PLAYER_MOVE_RIGHT:  // fallthrough intended
          case SDL_SCANCODE_RIGHT:
            port_1.p1_right = 1;
            port_2.p2_right = 1;
            break;
          default: break;
        } break;
      case SDL_KEYUP:
        switch(event.key.keysym.scancode)
        {
          case INSERT_COIN: port_1.credit = 0; break;
          case P2_START: port_1.p2 = 0; break;
          case P1_START: port_1.p1 = 0; break;
          case TILT: port_2.tilt = 0; break;
          case MUTE_AUDIO:  bus->audio.mute_audio(); break;
          case PAUSE_GAME:  // fallthrough intended
          case SDL_SCANCODE_P:
            bus->toggle_pause();
            bus->audio.paused();
            break;
          case PLAYER_SHOOT:
            port_1.p1_shot = 0;
            port_2.p2_shot = 0;
            break;
          case PLAYER_MOVE_LEFT:  // fallthrough intended
          case SDL_SCANCODE_LEFT:
            port_1.p1_left = 0;
            port_2.p2_left = 0;
            break;
          case PLAYER_MOVE_RIGHT:  // fallthrough intended
          case SDL_SCANCODE_RIGHT:
            port_1.p1_right = 0;
            port_2.p2_right = 0;
            break;
          case CHEAT_SPAWN_UFO:
            std::cout << "Cheat: Spawning UFO." << std::endl;
            write(0x2091, 0x00);
            write(0x2092, 0x00);
            break;
          case CHEAT_UFO_HIT:
            if (read(0x2084) == 0x00)
            {
              // Setting the UFO to be hit w/o a UFO spawned will immediately kill the next UFO that spawns
              std::cout << "Cheat: No UFO has spawned. Doing nothing." << std::endl;
            }
            else
            {
              std::cout << "Cheat: Hit UFO." << std::endl;
              write(0x2085, 0x01);
            }
            break;
          default: break;
        } break;
      case SDL_CONTROLLERBUTTONDOWN:
      {
        switch (event.cbutton.button)
        {
          case button_a: case button_b: case button_x: case button_y:  // fallthrough intended
          case leftstick_click: case rightstick_click:
            port_1.p1_shot = 1;
            port_2.p2_shot = 1;
            break;
          case d_left:
            port_1.p1_left = 1;
            port_2.p2_left = 1;
            break;
          case d_right:
            port_1.p1_right = 1;
            port_2.p2_right = 1;
            break;
          case shoulder_left:
            if (read(0x20EF) == GAME_RUNNING)
            {
              port_1.p1_shot = 1;
              port_2.p2_shot = 1;
            }
            else
            {
              port_1.p1 = 1;
            }
            break;
          case shoulder_right:
            if (read(0x20EF) == GAME_RUNNING)
            {
              port_1.p1_shot = 1;
              port_2.p2_shot = 1;
            }
            else
            {
              port_1.p2 = 1;
            }
            break;
          case start:
            if (read(0x20EF) != GAME_RUNNING)
              port_1.credit = 1;

            break;
          default:break;
        } break;
      }
      case SDL_CONTROLLERBUTTONUP:
      {
        switch (event.cbutton.button)
        {
          case button_a: case button_b: case button_x: case button_y:  // fallthrough intended
          case leftstick_click: case rightstick_click:
            port_1.p1_shot = 0;
            port_2.p2_shot = 0;
            break;
          case d_left:
            port_1.p1_left = 0;
            port_2.p2_left = 0;
            break;
          case d_right:
            port_1.p1_right = 0;
            port_2.p2_right = 0;
            break;
          case shoulder_left:
            if (read(0x20EF) == GAME_RUNNING)
            {
              port_1.p1_shot = 0;
              port_2.p2_shot = 0;
            }
            else
            {
              port_1.p1 = 1;
            }
            break;
          case shoulder_right:
            if (read(0x20EF) == GAME_RUNNING)
            {
              port_1.p1_shot = 0;
              port_2.p2_shot = 0;
            }
            else
            {
              port_1.p2 = 0;
            }
            break;
          case start:
            if (read(0x20EF) == GAME_RUNNING)
            {
              bus->toggle_pause();
              bus->audio.paused();
            }
            else
            {
              port_1.credit = 0;
            }
            break;
          default:break;
        } break;
      }
      case SDL_CONTROLLERAXISMOTION:
      {
        switch (event.caxis.axis)
        {
          case leftstick_x:
            if (event.caxis.value < -CONTROLLER_DEAD_ZONE)
            {
              port_1.p1_left = 1;
              port_2.p2_left = 1;
            }
            else if (event.caxis.value > CONTROLLER_DEAD_ZONE)
            {
              port_1.p1_right = 1;
              port_2.p2_right = 1;
            }
            else
            {
              port_1.p1_left = 0;
              port_2.p2_left = 0;
              port_1.p1_right = 0;
              port_2.p2_right = 0;
            }
            break;
          case left_trigger: case right_trigger:
            if (event.caxis.value > CONTROLLER_DEAD_ZONE)
            {
              port_1.p1_shot = 1;
              port_2.p2_shot = 1;
            }
            else
            {
              port_1.p1_shot = 0;
              port_2.p2_shot = 0;
            }
            break;
        } break;
      }
      case SDL_CONTROLLERDEVICEADDED:
      {
        init_controller(event.cdevice.which);
        break;
      }
      
      case SDL_CONTROLLERDEVICEREMOVED:
      {
        if (SDL_NumJoysticks() > 1) break;
        
        for (auto it = controllers.begin(); it != controllers.end(); ++it)
        {
          if (SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(*it)) == event.cdevice.which)
          {
            std::cout << "Controller removed: " << SDL_GameControllerName(*it) << std::endl;
            SDL_GameControllerClose(*it);
            controllers.erase(it);
            break;
          }
        }
        break;
      }
      default:
        break;
    }
  }
}

uint8_t Controls::in_port(uint8_t port, uint16_t PC, uint16_t DE)
{
  uint8_t retval = 0x00;
  
  if (run_tests)
    cpm_in_helper(PC, DE);
  else
    retval = si_in_helper(port);
  
  return retval;
}

uint8_t Controls::si_in_helper(uint8_t port)
{
  uint16_t value = 0x00;
  switch (port)
  {
    case 0:
      return port_0.value;
    case 1:
      return port_1.value;
    case 2:
      return port_2.value;
    case 3:
      value = (shift1 << 8) | shift0;
      value = (value >> (8 - shift_offset)) & 0xFF;
      return value;
    default:
      std::cerr << "Warning: Attempted to read from port " << static_cast<int>(port) << "." << std::endl;
  }
  
  return value;
}

void Controls::cpm_in_helper(uint16_t PC, uint16_t DE)
{
  uint8_t operation = PC;
  
  // print a character stored in E
  if (operation == 2)
    std::cout << static_cast<char>(DE & 0x0F);

  // print from memory at (DE) until '$' char
  else if (operation == 9)
  {
    uint16_t addr = DE;
    do
    {
      std::cout << static_cast<char>(read(addr++));
    } while(read(addr) != '$');
  }
}

void Controls::out_port(uint8_t port, uint8_t data)
{
  if (run_tests) return;
  
  switch (port)
  {
    case 2:
      shift_offset = data & 0x07;
      break;
    case 3:  // Bits in order: UFO, player shoot, player hit, invader hit, extra life, begin play
      bus->port3_audio();
      break;
    case 4:
      shift0 = shift1;
      shift1 = data;
      break;
    case 5:  // All 4 walk sounds (bits 0 - 3) and UFO hit sound (bit 4)
      bus->port5_audio();
      break;
    case 6:  // debug port?
      break;
    default:
      std::cerr << "Warning: Attempted to write " << std::hex << static_cast<int>(data)
                << " to port " << static_cast<int>(port) << "." << std::endl;
  }
}

uint8_t Controls::read(uint16_t address)
{
  return bus->read(address);
}

void Controls::write(uint16_t address, uint8_t data)
{
  bus->write(address, data);
}

void Controls::init_controller(int id)
{
  //One shared controller makes more sense that trying to support two
  if (SDL_NumJoysticks() > 1) return;
  
  SDL_GameController *controller = SDL_GameControllerOpen(id);
  
  if (controller)
  {
    if (SDL_GameControllerGetAttached(controller))
    {
      controllers.push_back(controller);
      std::cout << "Controller found: " << SDL_GameControllerName(controller) << std::endl;
    }
    else
    {
      SDL_GameControllerClose(controller);
    }
  }
}

#pragma clang diagnostic pop