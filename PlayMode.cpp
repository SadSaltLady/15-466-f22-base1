#include "PlayMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

//DEBUGS:
//for glm debug printing:
#include <glm/gtx/string_cast.hpp>

//for binary debug printing:
#include <bitset>

#include <random>
#define HURT_PALETTE 4
#define GRAVITY -5.f
#define GROUND 80.f
#define BOUND 320.f
#define BORDER 40.f
//image dimensions in terms of sprites
#define IMG_WID 7
#define IMG_HEI 4
#define IMG_FRAMES 4
//offset to reach the opposite looking sprite
//currently 7 * 16 = 
#define ORI_OFFSET 112
//walk cycle anime time
#define WALK_TIME 0.1f
#define ATTACK_TIME 0.1f
#define ATTACK_COOLDOWN 1.f
#define HURT_TIME 0.3f

Load < SpriteAtlas > sprite_master = { LoadTagDefault, [&]() {
	//create new 
	SpriteAtlas *ret = new SpriteAtlas();
	//ret->pathtest = data_path("assets\\test.png");
	glm::uvec2 size = glm::uvec2(float(IMG_WID * 8), float(IMG_HEI * IMG_FRAMES * 8));
	//allocate image array
	ret->images.resize(3);
	//fill with empty 
	ret->images.assign(2, std::vector< glm::u8vec4 >());
	//load_png(ret->pathtest, &size, &(ret->images[0]), LowerLeftOrigin);
	load_png(data_path("assets\\attack_56_128.png"), &size, &(ret->images[2]), LowerLeftOrigin);
	size = glm::uvec2(8.f, 8.f);
	load_png(data_path("assets\\nothing.png"), &size, &(ret->images[0]), LowerLeftOrigin);
	load_png(data_path("assets\\floor.png"), &size, &(ret->images[1]), LowerLeftOrigin);
	return ret;
} };

PlayMode::PlayMode() {
	//TODO:
	// you *must* use an asset pipeline of some sort to generate tiles.
	// don't hardcode them like this!
	// or, at least, if you do hardcode them like this,
	//  make yourself a script that spits out the code that you paste in here
	//   and check that script into your repository.

	//Also, *don't* use these tiles in your game:

	//load 8*8 file starting at sprite_master[master_idex] into ppu.tile_table[tile_index]
	auto load_single_tile = [&](uint32_t tile_index, uint32_t master_index, uint8_t wid, uint8_t hei, PPU466 &ppu) {
		//iterate over width and height tiles of the image and read it in 8*8 blocks
		uint32_t count = 0;
		for (uint32_t h = 0; h < hei; ++h) {
			for (uint32_t w = 0; w < wid ; ++w) {
				//create new tile
				PPU466::Tile tile;
				//iterate over 8*8 pixels of the tile
				for (uint32_t y = 7; y != -1 ; --y) { //NOTE:: why does this != 0 work...
					uint8_t bit0 = 0;
					uint8_t bit1 = 0;
					for (uint32_t x = 7; x != -1; --x) {
						//get the color of the pixel
						glm::u8vec4 color = sprite_master->images[master_index][(w * 8 + x) + (h * 8 + y) * wid * 8];
						//set bits according to color channels
						bit0 |= (color.r > 0) << x;
						bit1 |= (color.g > 0) << x;
					}
					tile.bit0[y] = bit0;
					tile.bit1[y] = bit1;
				}
				//set the tile in the tile table
				//ppu.tile_table[tile_index + w + h * wid] = tile;
				ppu.tile_table[tile_index + count] = tile;
				++count;
				std::cout << "--count: " << count << std::endl;

			}
		}
		//read in the same file, but reverse the x axis
		//this is to make the sprite look in the opposite direction
		//loading right side
		
		for (uint32_t h = 0; h < hei; ++h) {
			for (uint32_t w = 0; w < wid ; ++w) {
				//create new tile
				PPU466::Tile tile;
				//iterate over 8*8 pixels of the tile
				for (uint32_t y = 7; y != -1 ; --y) { //NOTE:: why does this != 0 work...
					uint8_t bit0 = 0;
					uint8_t bit1 = 0;
					for (uint32_t x = 7; x != -1; --x) {
						//get the color of the pixel
						glm::u8vec4 color = sprite_master->images[master_index][(w * 8 + x) + (h * 8 + y) * wid * 8];
						//set bits according to color channels
						//but in reverse order
						bit0 |= (color.r > 0) << (7 - x);
						bit1 |= (color.g > 0) << (7 - x);
					}
					tile.bit0[y] = bit0;
					tile.bit1[y] = bit1;
				}
				//set the tile in the tile table
				ppu.tile_table[tile_index + count] = tile;
				++count;
				std::cout << "count: " << count << std::endl;
			}
		}

	};
	load_single_tile(0, 0, 1, 1, ppu);
	load_single_tile(2, 1, 1, 1, ppu);
	load_single_tile(4, 2, IMG_WID, IMG_HEI * IMG_FRAMES, ppu);

	//makes the outside of tiles 0-16 solid:
	ppu.palette_table[0] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	};

	//makes the center of tiles 0-16 solid:
	ppu.palette_table[1] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	};

	//used for the player one sprite:
	//seagull
	ppu.palette_table[7] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0xff, 0xff, 0xff, 0xff),
		glm::u8vec4(0xA1, 0xA1, 0xA1, 0xff),
		glm::u8vec4(0xff, 0xCB, 0x0E, 0xff),
	};

	//used for the player two sprite:
	//crow
	ppu.palette_table[5] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x2b, 0x20, 0x2b, 0xff),
		glm::u8vec4(0x12, 0x0f, 0x12, 0xff),
		glm::u8vec4(0xff, 0x73, 0x00, 0xff),
	};
	

	//used for the player hurt sprite:
	ppu.palette_table[4] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0xff, 0x00, 0x00, 0xff),
		glm::u8vec4(0xff, 0x00, 0x00, 0xff),
		glm::u8vec4(0xff, 0x00, 0x00, 0xff),
	};

	//used for the misc other sprites:
	ppu.palette_table[6] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0xff, 0xff, 0xff, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	};

	//Initialize the players:
	//again, weapon_idx is player + 1
	player0.sprite_idx = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28};
	player0.weapon_idx = 57;
	player0.color_index = 7; 
	player0.weapon_color = 5;
	player0.dim = glm::vec2(7.0f, 4.0f);
	player0.on_right = false;
	//place player at left middle of screen
	player0.at = glm::vec2(ppu.ScreenWidth/2 - player0.dim.x/2, ppu.ScreenHeight/3 * 2 - player0.dim.y/2);

	player1.sprite_idx = {29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56};
	player1.weapon_idx = 58;
	player1.color_index = 5;
	player1.weapon_color = 5;
	player1.dim = glm::vec2(7.0f, 4.0f);
	player1.on_right = true;
	//place player at right middle of screen
	player1.at = glm::vec2(ppu.ScreenWidth/2 + player1.dim.x/2, ppu.ScreenHeight/3 * 2  - player1.dim.y/2);
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	//Controlls player movement, p0 = player one, p1 = player 2
	//player1 uses arrow keys to control movement
	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_j) {
			p0_left.downs += 1;
			p0_left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_l) {
			p0_right.downs += 1;
			p0_right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_i) {
			p0_up.downs += 1;
			p0_up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_b) {
			p0_atc.downs += 1;
			p0_atc.pressed = true;
			return true;
		} //player two uses WASD to control movement 
		else if (evt.key.keysym.sym == SDLK_a) {
			p1_left.downs += 1;
			p1_left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			p1_right.downs += 1;
			p1_right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			p1_up.downs += 1;
			p1_up.pressed = true;
			return true;
		} 
		else if (evt.key.keysym.sym == SDLK_c) {
			p1_atc.downs += 1;
			p1_atc.pressed = true;
			return true;
		}
		
	} else if (evt.type == SDL_KEYUP) {
		//Player one
		if (evt.key.keysym.sym == SDLK_j) {
			p0_left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_l) {
			p0_right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_i) {
			p0_up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_b) {
			p0_atc.pressed = false;
			return true;
		//Player 2
		} else if (evt.key.keysym.sym == SDLK_a) {
			p1_left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			p1_right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			p1_up.pressed = false;
			return true;
		} 
		else if (evt.key.keysym.sym == SDLK_c) {
			p1_atc.pressed = false;
			return true;
		} 
	}

	return false;
}

void PlayMode::update(float elapsed) {

	//slowly rotates through [0,1):
	// (will be used to set background color)
	background_fade += elapsed / 10.0f;
	background_fade -= std::floor(background_fade);

	constexpr float PlayerSpeed = 80.0f;
	//movement and attack handler
	auto movement_handler = [&PlayerSpeed] (Player &player, Button &left, Button &right, Button &up, Button &attac, float elapsed) {
		//resolve acceleration
		if (left.pressed) {
			player.acceleration.x -= 5.0f;
		}
		if (right.pressed) {
			player.acceleration.x += 5.0f;
		}
		if (up.pressed && player.on_ground) {
			player.acceleration.y += 60.0f;
		}
		//resolve velcity 
		player.velocity += player.acceleration * elapsed * PlayerSpeed;
		//clamp velocity
		player.velocity.x = glm::clamp(player.velocity.x, -PlayerSpeed, PlayerSpeed);
		player.velocity.y = glm::clamp(player.velocity.y, -PlayerSpeed * 3.f, PlayerSpeed*3.f);
		//resolve position
		player.at += player.velocity * elapsed;
		//dampen horizontal movement
		player.acceleration.x *= 0.5f;		
		//add gravity
		player.acceleration.y += GRAVITY;
		//don't let player fall 
		if (player.at.y <= GROUND) {
			player.on_ground = true;
			player.at.y = GROUND;
			player.velocity.y = 0.0f;
			player.acceleration.y = 0.0f;
		}
		else {
			player.on_ground = false;
		}
		//don't let player move off screen
		//TODO:
		player.at.x = std::max(player.at.x, BORDER);
		player.at.x = std::min(player.at.x, BOUND - BORDER*2 - 8.0f);

		//attack handler

		if (player.can_attack && !player.is_attacking) {
			if (attac.pressed) {
				player.is_attacking = true;
				player.attack_timer = 0.0f;
				player.can_attack = false;
				player.attack_cooldown = ATTACK_COOLDOWN;
			}
		}
		else {
			if (!player.can_attack) {
				player.attack_cooldown -= elapsed;
				if (player.attack_cooldown <= 0.0f) {
					player.can_attack = true;
				}
			}
			if (player.is_attacking) {
				player.attack_timer += elapsed;
				if (player.attack_timer >= ATTACK_TIME) {
					player.is_attacking = false;
				}
			}
		}
		};
	movement_handler(player0, p0_left, p0_right, p0_up, p0_atc, elapsed);
	movement_handler(player1, p1_left, p1_right, p1_up, p1_atc, elapsed);

	auto orientation_handler = [&] (Player &p0, Player &p1) {
		if (p0.at.x < p1.at.x) {
			p0.on_right = false;
			p1.on_right = true;
		} else {
			p0.on_right = true;
			p1.on_right = false;
		}
	};
	orientation_handler(player0, player1);

	//animation setter
	auto animation_handler = [&](Player &player) {
		player.anim_timer += elapsed;
		if (player.on_ground) {
			if (player.velocity.x == 0.0f) {
				player.animation = 0;
			} else {
				if (player.anim_timer > WALK_TIME / (std::abs(player.velocity.x) /30.f)) {
					player.animation = (player.animation + 1) % 2;
					player.anim_timer = 0.0f;
				}
			}
		} else {
			player.animation = 2;
		}

		if (player.is_attacking) {
			player.animation = 3;
		}
	};
	animation_handler(player0);
	animation_handler(player1);

	//check for collisions
	auto collision_handler = [&](Player &attack, Player &defend) {
		//check if attack is in range of defend
		float dir = attack.on_right ? 1.0f : -1.0f;
		float diff = attack.at.x - defend.at.x; //how many blocks are attacking
		//if in range and if y is not too far apart
		if (defend.hurt) {
			defend.hurt_timer += elapsed;
			if (defend.hurt_timer >= HURT_TIME) {
				defend.hurt = false;
			}
		}
		if (attack.is_attacking && !defend.hurt && std::abs(diff) <= (3 * 8 + 4) && 
			(diff/std::abs(diff)) == dir && std::abs(attack.at.y - defend.at.y) <= 8) {
			//check if attack is in range of defend
			defend.hurt = true;
			defend.hurt_timer = 0.0f;
			defend.health -= 1;
			if (defend.health <= 0) {
				defend.health = 0;
				defend.is_dead = true;
			}
		} 
	};	
	collision_handler(player0, player1);
	collision_handler(player1, player0);

	//reset button press counters:
	p0_left.downs = 0;
	p0_right.downs = 0;
	p0_up.downs = 0;
	p0_atc.downs = 0;

	p1_left.downs = 0;
	p1_right.downs = 0;
	p1_up.downs = 0;
	p1_atc.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---

	//background color will be some hsv-like fade:
	ppu.background_color = glm::u8vec4(
		std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI * (background_fade + 0.0f / 3.0f) ) ) ))),
		std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI * (background_fade + 1.0f / 3.0f) ) ) ))),
		std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI * (background_fade + 2.0f / 3.0f) ) ) ))),
		0xff
	);

	//tilemap gets recomputed every frame as some weird plasma thing:
	//NOTE: don't do this in your game! actually make a map or something :-)
	for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
		for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
			//TODO: make weird plasma thing
			ppu.background[x+PPU466::BackgroundWidth*y] = 0;
			if ( y == GROUND) {
				ppu.background[x+PPU466::BackgroundWidth*y] = 12;
				
			}
		}
	}


	//background scroll: 
	//TODO: not sure how to handle this yet....
	ppu.background_position.x = int32_t(-0.5f * player1.at.x);
	ppu.background_position.y = int32_t(-0.5f * player1.at.y);
	//uncessary but I wanted to write a lambda
	auto player_draw = [](Player &player,	PPU466& ppu) {
		//draw player (four sprites in a square)
		//contains sprites used for the player
		uint8_t color = player.color_index;
		uint8_t offset = player.animation * IMG_WID * IMG_HEI;

		if (player.hurt) 
		{
			color = HURT_PALETTE;
		}

		//draw player block
		std::vector < uint8_t > spri_idx = player.sprite_idx;
		uint8_t count = 0;
		if (!player.on_right) {
			for (uint8_t h = 0; h < player.dim.y; h++) {
				for (uint8_t w = 0; w < player.dim.x; w++) {
					//create offset
					float offset_x = w - player.dim.x / 2.0f;
					float offset_y = h - player.dim.y / 2.0f;
					ppu.sprites[spri_idx[count]].x = uint8_t(player.at.x + offset_x*8);
					ppu.sprites[spri_idx[count]].y = uint8_t(player.at.y + offset_y*8);
					assert(4 + offset + count < 256);
					ppu.sprites[spri_idx[count]].index = 4 + offset + count;
					ppu.sprites[spri_idx[count]].attributes = color;
					//std::cout << "x: " << unsigned(ppu.sprites[spri_idx[count]].x) << "   y: " << unsigned(ppu.sprites[spri_idx[count]].y) << "   count:"  << unsigned(33+ count)<< std::endl;

					count++;
				}
			}
		} else {
			offset += ORI_OFFSET;
			for (uint8_t h = 0; h < player.dim.y; h++) {
				for (int8_t w = int8_t(player.dim.x) - 1; w != -1 ; --w) {
					//create offset
					float offset_x = w - player.dim.x / 2.0f;
					float offset_y = h - player.dim.y / 2.0f;
					ppu.sprites[spri_idx[count]].x = uint8_t(player.at.x + offset_x*8);
					ppu.sprites[spri_idx[count]].y = uint8_t(player.at.y + offset_y*8);
					//std::cout << "test:" << unsigned(33 + offset + count) << std::endl;
					//std::cout << "offset:" << unsigned(offset) << "  count:" << unsigned(count) << std::endl;
					assert(4 + offset + count < 256);
					ppu.sprites[spri_idx[count]].index = 4 + offset + count;
					ppu.sprites[spri_idx[count]].attributes = color;
					//std::cout << "x: " << unsigned(ppu.sprites[spri_idx[count]].x) << "   y: " << unsigned(ppu.sprites[spri_idx[count]].y) << "   count:"  << unsigned(33+ count)<< std::endl;

					count++;
				}
			}
		
		}
	};

	player_draw(player0, ppu);
	player_draw(player1, ppu);

	//some other misc sprites:
	for (uint32_t i = 59; i < 63; ++i) {
		/* 
		* Old Code:
		float amt = (i + 2.0f * background_fade) / 62.0f;
		ppu.sprites[i].x = int8_t(0.5f * PPU466::ScreenWidth + std::cos( 2.0f * M_PI * amt * 5.0f + 0.01f * player_at.x) * 0.4f * PPU466::ScreenWidth);
		ppu.sprites[i].y = int8_t(0.5f * PPU466::ScreenHeight + std::sin( 2.0f * M_PI * amt * 3.0f + 0.01f * player_at.y) * 0.4f * PPU466::ScreenWidth);
		*/
		ppu.sprites[i].x = 225;
		ppu.sprites[i].y = 225;
		ppu.sprites[i].index = 32;
		ppu.sprites[i].attributes = 6;
		if (i % 2) ppu.sprites[i].attributes |= 0x80; //'behind' bit
	}

	//--- actually draw ---
	ppu.draw(drawable_size);
}
