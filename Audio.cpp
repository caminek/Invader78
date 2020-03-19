#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-signed-bitwise"
#include <stdexcept>
#include <iostream>
#include <filesystem>
#include "Audio.hpp"
#include "Bus.hpp"

Audio::~Audio()
{
  if (sounds_folder_exists)
  {
    for (auto& i : sfx)
      for (auto& j : i)
        Mix_FreeChunk(j);

    Mix_CloseAudio();
  }
}

void Audio::init()
{
  // SDL init
  if (SDL_Init(SDL_INIT_AUDIO) != 0)
  {
    SDL_Log("Error: Unable to initialize SDL Audio: %s", SDL_GetError());
    exit(1);
  }

  mute = false;
  game_paused = false;
  check_audio();

  if (sounds_folder_exists)
  {
    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, AUDIO_CHANNELS, 4096) == -1)
      throw std::runtime_error(Mix_GetError());

    for (int i = 0; i < 2; ++i)
      for (int j = 0; j < 5; ++j)
        if (std::filesystem::exists(sound_file[i][j]))
          sfx[i][j] = Mix_LoadWAV(sound_file[i][j]);
        else
          sfx[i][j] = nullptr;

    port_3_last_played = 0;
    port_5_last_played = 0;

    // channels are reserved starting from channel 0
    Mix_ReserveChannels(1);

    Mix_Volume(-1, sfx_volume);
  }
}

uint8_t Audio::read(uint16_t address)
{
  return bus->read(address);
}

void Audio::play(uint8_t port)
{
  if (!sounds_folder_exists) return;

  uint8_t A = bus->cpu.get_A();
  
  if (read(0x2085) == UFO_HIT || read(0x2084) == UFO_INACTIVE)
  {
    Mix_HaltChannel(0);
  }
  
  // Bits in order: All 4 walk sounds (bits 0 - 3) and UFO hit sound (bit 4)
  if (port == 3 && A != port_3_last_played)
  {
    play_helper(A, port, port_3_last_played);
    port_3_last_played = A;
  }
  // Bits in order: UFO, player shoot, player hit, invader hit, extra life, begin play
  else if (port == 5 && A != port_5_last_played)
  {

    play_helper(A, port, port_5_last_played);
    port_5_last_played = A;
  }
}

void Audio::play_helper(uint8_t A, uint8_t port, uint8_t last_played)
{
  for (uint8_t bit = 0x01, idx = 0; bit <= 0x10; bit <<= 1, ++idx)
  {
    if ((A & bit) && !(last_played & bit))
    {
      if (port == 3)
      {
        if (sfx[0][idx] == nullptr) return;  // Sound is missing

        // UFO Flying is the only sound that needs to be looped
        if (idx == 0)
          Mix_PlayChannel(0, sfx[0][idx], -1);
        else
          Mix_PlayChannel(1, sfx[0][idx], 0);
      }
      else
      {
        if (sfx[1][idx] == nullptr) return;  // Sound is missing

        // UFO shot sound needs its own channel to be played at full length
        if (idx == 4)
          Mix_PlayChannel(4, sfx[1][idx], 0);
        else
          Mix_PlayChannel(2, sfx[1][idx], 0);
      }
    }
  }
}

void Audio::check_audio()
{
  if (std::filesystem::exists("Resources/Sounds"))
  {
    sounds_folder_exists = true;
  }
  else
  {
    sounds_folder_exists = false;
    std::cerr << "Warning: Sounds folder does not exists. No audio will be played." << std::endl;
  }
}


// Mutes the UFO since it will loop during pause.
// Do not allow mute status to change if the game is paused
void Audio::mute_audio()
{
  if (!mute && !game_paused)
  {
    volume_when_muted = sfx_volume;
    sfx_volume = 0;
    mute = !mute;

    Mix_Volume(-1, 0);
  }
  else if (mute && !game_paused)
  {
    sfx_volume = volume_when_muted;
    mute = !mute;
    Mix_Volume(-1, sfx_volume);

    // if the UFO spawns while muted and then we unmute, it will not play
    // this will start the sound
    if (read(0x2084))
      Mix_PlayChannel(0, sfx[0][0], -1);
  }
}

// "disable" audio only if mute hasn't been set
// this keeps UFO sound from looping on pause
void Audio::paused()
{
  if (!mute && game_paused)
  {
    sfx_volume = volume_when_muted;
  }
  else if (!mute && !game_paused)
  {
    volume_when_muted = sfx_volume;
    sfx_volume = 0;
  }

  Mix_Volume(-1, sfx_volume);
  game_paused = !game_paused;
}

#pragma clang diagnostic pop