#ifndef SPACE_INVADERS_DISPLAY_HPP
#define SPACE_INVADERS_DISPLAY_HPP

#include <string>
#include <utility>
#include <SDL2/SDL.h>

class Bus;

class Display
{
 public:
  Display(std::string title) : window_title(std::move(title)) {};
  ~Display();
  
  void attach(Bus *b) { bus = b; }
  void init();
  void update();
  void draw();

 private:
  Bus *bus = nullptr;
  uint8_t read_vram(uint16_t index);
  
  // SDL
  SDL_Window* window;
  SDL_Renderer* renderer;
  SDL_Texture* texture;
  std::string window_title;

  // pixel-related
  static constexpr int SCREEN_HEIGHT = 256;
  static constexpr int SCREEN_WIDTH = 224;
  static constexpr int TILE_WIDTH = 8;
  uint8_t screen_buffer[SCREEN_HEIGHT][SCREEN_WIDTH][4];
  static void reposition_pixels(int &x, int &y);
  
  struct Pixel
  {
    int x;
    int y;
    bool is_lit;
    union
    {
      uint32_t rgb : 24;
      struct
      {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
      };
    };
  } pixel;
  
  void enable_pixel(Pixel &pix, bool colorize);
};

#endif











