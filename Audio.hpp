#ifndef SPACE_INVADERS_AUDIO_HPP
#define SPACE_INVADERS_AUDIO_HPP

#include <string>
#include "SDL.h"
#include "SDL_mixer.h"

class Bus;

class Audio
{
 public:
  Audio() = default;
  ~Audio();
  
  void attach(Bus *b) { bus = b; }
  void init();
  void play(uint8_t port);
  void mute_audio();
  void paused();

  static constexpr int UFO_INACTIVE = 0x00;
  static constexpr int UFO_HIT = 0x01;
  static constexpr int AUDIO_CHANNELS = 4;
  static constexpr int MAX_VOLUME = 128;
  static constexpr int MIN_VOLUME = 0;

 private:
  Bus *bus = nullptr;

  int sfx_volume = 50;
  int volume_when_muted = 50;
  bool mute{};
  bool game_paused{};
  bool sounds_folder_exists{};
  Mix_Chunk* sfx[2][5]{};
  uint8_t port_3_last_played{};
  uint8_t port_5_last_played{};
  
  void play_helper(uint8_t A, uint8_t port, uint8_t last_played);
  void check_audio();
  uint8_t read(uint16_t address);

  const char* sound_file[2][5] =
  {
      // Bank 0 are Port 3 sounds
      {
          "Resources/Sounds/00-ufo.wav",
          "Resources/Sounds/01-player_shoot.wav",
          "Resources/Sounds/02-player_hit.wav",
          "Resources/Sounds/03-invader_hit.wav",
          "Resources/Sounds/09-extra_life.wav"
      },
      // Bank 1 are Port 5 sounds
      {
          "Resources/Sounds/04-invader.wav",
          "Resources/Sounds/05-invader.wav",
          "Resources/Sounds/06-invader.wav",
          "Resources/Sounds/07-invader.wav",
          "Resources/Sounds/08-ufo_hit.wav"
      }
  };
};

#endif
