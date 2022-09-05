#include "PPU466.hpp"
#include "Mode.hpp"
#include "Load.hpp"
#include "data_path.hpp"
#include "load_save_png.hpp"

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <deque>
#include <iostream>

struct SpriteAtlas {
		int sprite_count = 0;
		std::string pathtest = "";
		//array of images 
		//image = vector< glm::u8vec4 >
		std::vector< std::vector< glm::u8vec4 > > images = {};
};

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} p0_left, p0_right, p0_down, p0_up, p1_left, p1_right, p1_down, p1_up;

	//some weird background animation:
	float background_fade = 0.0f;

	//This game has two players
	//Player info tracking:
	//I could put the directions here but lazy code
	struct Player {
		//Player position
		glm::vec2 at = glm::vec2(0.0f, 0.0f);
		//Player movement direction
		glm::vec2 direction = glm::vec2(0.0f, 0.0f);
		//Pallette index
		uint8_t color_index = 0;
		//sprite index
		uint8_t sprite_idx = 0;
		//Weapon position
		glm::vec2 weapon_at = glm::vec2(0.0f, 0.0f);
		//Weapon sprite is always assumed to be sprite_index + 1
		uint8_t weapon_idx = 0;
		//Weapon color
		uint8_t weapon_color = 0;
		//status
		bool hurt = false;
	}player0, player1;

	//----- drawing handled by PPU466 -----

	PPU466 ppu;
};
