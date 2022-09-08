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
	} p0_left, p0_right, p0_up, p0_atc, p1_left, p1_right, p1_up, p1_atc;

	//some weird background animation:
	float background_fade = 0.0f;

	//This game has two players
	//Player info tracking:
	//I could put the directions here but lazy code
	struct Player {
		//Player position
		glm::vec2 at = glm::vec2(0.0f, 0.0f);
		//Player direction = acceleration
		glm::vec2 acceleration = glm::vec2(0.0f, -5.0f);
		//player velocity
		glm::vec2 velocity = glm::vec2(0.0f, 0.0f);
		//player health
		uint8_t health = 3;
		//----------------BOOLEANS----------------
		bool on_ground = true;
		bool on_right = false;
		bool hurt = false;
		bool is_attacking = false;
		bool can_attack = true;
		bool is_dead = false;
		//----------------DRAWING RELATED ---------
		//Pallette index
		uint8_t color_index = 0;
		//sprite index
		//equal to constant between 1 - 28
		std::vector< uint8_t > sprite_idx = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28};
		//dimensions of sprite
		glm::vec2 dim = glm::vec2(.0f, 0.0f);
		//Weapon position
		glm::vec2 weapon_at = glm::vec2(0.0f, 0.0f);
		//Weapon sprite is always assumed to be sprite_index + 1
		uint8_t weapon_idx = 0;
		//Weapon color
		uint8_t weapon_color = 0;
		//-----------------ANIM STATS----------------
		//0 = idle, 1 = walk, 2 = jump, 3 = attack
		uint8_t animation = 0;
		//-----------------TIMERS----------------
		float anim_timer = 0.0f;
		float attack_timer = 0.0f;
		float hurt_timer = 0.0f;
		float attack_cooldown = 0.0f;
	}player0, player1;

	//----- drawing handled by PPU466 -----

	PPU466 ppu;
};
