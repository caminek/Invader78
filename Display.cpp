#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-signed-bitwise"
#include <iostream>
#include <array>
#include <SDL_image.h>
#include "Bus.hpp"
#include "Display.hpp"

void Display::init()
{
  // SDL init
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0)
  {
    SDL_Log("Error: Unable to initialize SDL: %s", SDL_GetError());
    exit(1);
  }
  
  // create SDL window
  window = SDL_CreateWindow(window_title.c_str(),
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      SCREEN_WIDTH * 2,
      SCREEN_HEIGHT * 2,
      SDL_WINDOW_RESIZABLE);
  
  if (window == nullptr)
  {
    SDL_Log("unable to create window: %s", SDL_GetError());
    exit(1);
  }
  
  SDL_SetWindowMinimumSize(window, SCREEN_WIDTH, SCREEN_HEIGHT);
  SDL_ShowCursor(SDL_DISABLE);
  
  // on macOS, use metal driver if available:
  SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
  
  // create renderer
  //renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  
  if (renderer == nullptr)
  {
    SDL_Log("unable to create renderer: %s", SDL_GetError());
    exit(1);
  }
  
  SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
  
  // print info on renderer:
  SDL_RendererInfo renderer_info;
  SDL_GetRendererInfo(renderer, &renderer_info);
  SDL_Log("using renderer %s", renderer_info.name);
  
  // create texture
  texture = SDL_CreateTexture(renderer,
      SDL_PIXELFORMAT_RGBA32,
      SDL_TEXTUREACCESS_STREAMING,
      SCREEN_WIDTH,
      SCREEN_HEIGHT);
  
  if (texture == nullptr)
  {
    SDL_Log("unable to create texture: %s", SDL_GetError());
    exit(1);
  }
  
  memset(screen_buffer, 0, sizeof(screen_buffer));
}

Display::~Display()
{
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

void Display::update()
{
  for (int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH / TILE_WIDTH; i++)
  {
    int x = i * TILE_WIDTH % SCREEN_HEIGHT;
    int y = i * TILE_WIDTH / SCREEN_HEIGHT;
    
    uint8_t tile = read_vram(i);
    
    for (uint8_t bit = 0; bit < TILE_WIDTH; bit++)
    {
      pixel.x = x + bit;
      pixel.y = y;
      pixel.is_lit = (tile >> bit) & 1;
      
      enable_pixel(pixel, true);
      
      // causes the screen to be rendered 90 degrees counter-clockwise
      reposition_pixels(pixel.x, pixel.y);
      
      screen_buffer[pixel.y][pixel.x][0] = pixel.red;
      screen_buffer[pixel.y][pixel.x][1] = pixel.green;
      screen_buffer[pixel.y][pixel.x][2] = pixel.blue;
    }
  }
  
  const uint32_t pitch = sizeof(uint8_t) * 4 * SCREEN_WIDTH;
  SDL_UpdateTexture(texture, nullptr, &screen_buffer, pitch);
}

void Display::draw()
{
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, nullptr, nullptr);
  SDL_RenderPresent(renderer);
}

void Display::reposition_pixels(int &x, int &y)
{
  int temp_x = x ;
  x = y;
  y = -temp_x + SCREEN_HEIGHT - 1;
}

uint8_t Display::read_vram(uint16_t index)
{
  return bus->read(0x2400 + index);
}

void Display::enable_pixel(Pixel &pix, bool colorize)
{
  pixel.rgb = 0x00;
  
  if (!colorize && pixel.is_lit)
    pixel.rgb = 0xFFFFFF;
  
  // colorize pixels based on the ASCII map shown at:
  // http://www.emutalk.net/threads/38177-Space-Invaders
  if (colorize && pixel.is_lit)
  {
    if ((pixel.x  < 16 && (pixel.y > 16 && pixel.y < 134)) || (pixel.x >= 16 && pixel.x  < 64))
      pixel.green = 0xFF;
    else if (pixel.x  >= 192 && pixel.x  < 224)
      pixel.red = 0xFF;
    else
      pixel.rgb = 0xFFFFFF;
  }
}

#pragma clang diagnostic pop