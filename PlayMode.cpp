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

Load < SpriteAtlas > sprite_master = { LoadTagDefault, [&]() {
	//create new 
	SpriteAtlas *ret = new SpriteAtlas();
	//ret->pathtest = data_path("assets\\test.png");
	glm::uvec2 size = glm::uvec2(8.0f, 8.0f);
	//allocate image array
	ret->images.resize(8);
	//fill with empty 
	ret->images.assign(8, std::vector< glm::u8vec4 >());
	//load_png(ret->pathtest, &size, &(ret->images[0]), LowerLeftOrigin);
	size = glm::uvec2(56.0f, 32.0f);
	load_png(data_path("assets\\attack_56_32.png"), &size, &(ret->images[1]), LowerLeftOrigin);
	std::cout << "size of attack_56_32.png: " << ret->images[1].size() << std::endl;
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

	{ //use tiles 0-16 as some weird dot pattern thing:
		std::array< uint8_t, 8*8 > distance;
		for (uint32_t y = 0; y < 8; ++y) {
			for (uint32_t x = 0; x < 8; ++x) {
				float d = glm::length(glm::vec2((x + 0.5f) - 4.0f, (y + 0.5f) - 4.0f));
				d /= glm::length(glm::vec2(4.0f, 4.0f));
				distance[x+8*y] = uint8_t(std::max(0,std::min(255,int32_t( 255.0f * d ))));
			}
		}
		for (uint32_t index = 0; index < 16; ++index) {
			PPU466::Tile tile;
			uint8_t t = uint8_t((255 * index) / 16);
			for (uint32_t y = 0; y < 8; ++y) {
				uint8_t bit0 = 0;
				uint8_t bit1 = 0;
				for (uint32_t x = 0; x < 8; ++x) {
					uint8_t d = distance[x+8*y];
					if (d > t) {
						bit0 |= (1 << x);
					} else {
						bit1 |= (1 << x);
					}
				}
				tile.bit0[y] = bit0;
				tile.bit1[y] = bit1;
			}
			ppu.tile_table[index] = tile;
		}
	}
	//load 8*8 file starting at sprite_master[master_idex] into ppu.tile_table[tile_index]
	auto load_single_tile = [&](uint32_t tile_index, uint32_t master_index, uint8_t wid, uint8_t hei, PPU466 &ppu) {
		//iterate over width and height tiles of the image and read it in 8*8 blocks
		uint32_t count = 0;
		for (uint32_t h = hei -1; h != -1; --h) {
			for (uint32_t w = 0; w < wid ; ++w) {
				//create new tile
				PPU466::Tile tile;
				//iterate over 8*8 pixels of the tile
				for (uint32_t y = 7; y != -1 ; --y) {
					uint8_t bit0 = 0;
					uint8_t bit1 = 0;
					for (uint32_t x = 7; x != -1; --x) {
						//get the color of the pixel
						glm::u8vec4 color = sprite_master->images[master_index][(w * 8 + x) + (h * 8 + y) * 16];
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
				std::cout << "count: " << tile_index + count << std::endl;
			}
		}
	};
	//load_single_tile(33, 0, 1, 1, ppu);
	load_single_tile(33, 1, 7, 4, ppu);
	//use sprite 32 as a "player":
	ppu.tile_table[32].bit0 = {
		0b01111110,
		0b11111111,	
		0b11111111,
		0b11111111,
		0b11111111,
		0b11111111,
		0b11111111,
		0b01111110,
	};
	ppu.tile_table[32].bit1 = {
		0b00000000,
		0b00000000,
		0b00111100,
		0b00000000,
		0b00000000,
		0b00100100,
		0b00000000,
		0b00000000,
	};

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
	ppu.palette_table[7] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0xff, 0xff, 0x00, 0xff),
		glm::u8vec4(0xff, 0x00, 0x00, 0xff),
		glm::u8vec4(0xff, 0x00, 0x00, 0xff),
	};

	//used for the player two sprite:
	ppu.palette_table[5] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0xff, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	};

	//used for the player hurt sprite:
	ppu.palette_table[4] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0xff, 0x00, 0x00, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	};

	//used for the misc other sprites:
	ppu.palette_table[6] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x88, 0x88, 0xff, 0xff),
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

	player1.sprite_idx = {29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56};
	player1.weapon_idx = 58;
	player1.color_index = 7;
	player1.weapon_color = 5;
	player1.dim = glm::vec2(7.0f, 4.0f);
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	//Controlls player movement, p0 = player one, p1 = player 2
	//player1 uses arrow keys to control movement
	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			p0_left.downs += 1;
			p0_left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			p0_right.downs += 1;
			p0_right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			p0_up.downs += 1;
			p0_up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			p0_down.downs += 1;
			p0_down.pressed = true;
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
		} else if (evt.key.keysym.sym == SDLK_s) {
			p1_down.downs += 1;
			p1_down.pressed = true;
			return true;
		}
		
	} else if (evt.type == SDL_KEYUP) {
		//Player one
		if (evt.key.keysym.sym == SDLK_LEFT) {
			p0_left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			p0_right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			p0_up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			p0_down.pressed = false;
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
		} else if (evt.key.keysym.sym == SDLK_s) {
			p1_down.pressed = false;
			return true;
		} //DEBUG functionalities 

	}

	return false;
}

void PlayMode::update(float elapsed) {

	//slowly rotates through [0,1):
	// (will be used to set background color)
	background_fade += elapsed / 10.0f;
	background_fade -= std::floor(background_fade);
	//reset directions 
	player0.direction.x = 0;
	player0.direction.y = 0;
	player1.direction.x = 0;
	player1.direction.y = 0;


	constexpr float PlayerSpeed = 30.0f;

	auto movement_handler = [&PlayerSpeed] (Player &player, Button &left, Button &right, Button &up, Button &down, float elapsed) {
		//handle movement
		if (left.pressed) {
			player.direction.x -= 1.0f;
		}
		if (right.pressed) {
			player.direction.x += 1.0f;
		}
		if (up.pressed) {
			player.direction.y += 1.0f;
		}
		if (down.pressed) {
			player.direction.y -= 1.0f;
		}
		//normalize direction
		if (glm::length(player.direction) > 0.0f) {
			player.direction = glm::normalize(player.direction);
		}
		//move player
		player.at += player.direction * PlayerSpeed * elapsed;
		//move weapon 
		if ( left.pressed || right.pressed || up.pressed || down.pressed) {
			player.weapon_at = player.at + 8.0f * player.direction;
		}
		//don't let player move off screen
		/*
		player.position.x = std::max(player.position.x, 0.0f);
		player.position.x = std::min(player.position.x, float(CourtSize.x));
		player.position.y = std::max(player.position.y, 0.0f);
		player.position.y = std::min(player.position.y, float(CourtSize.y));
		*/
	};
	movement_handler(player0, p0_left, p0_right, p0_up, p0_down, elapsed);
	movement_handler(player1, p1_left, p1_right, p1_up, p1_down, elapsed);

	//check for collisions
	auto collision_handler = [&](Player &attack, Player &defend) {
		//check if attack is in range of defend
		if (glm::distance(attack.weapon_at, defend.at) < 8.0f) {
			defend.hurt = true;
		} else {
			defend.hurt = false;
		}
	};
	collision_handler(player0, player1);
	collision_handler(player1, player0);

	//reset button press counters:
	p0_left.downs = 0;
	p0_right.downs = 0;
	p0_up.downs = 0;
	p0_down.downs = 0;

	p1_left.downs = 0;
	p1_right.downs = 0;
	p1_up.downs = 0;
	p1_down.downs = 0;
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
			ppu.background[x+PPU466::BackgroundWidth*y] = (1);
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
		uint8_t color = 0;
		if (player.hurt) 
		{
			color = HURT_PALETTE;
		} else {
			color = player.color_index;
		}

		//draw player block
		std::vector < uint8_t > spri_idx = player.sprite_idx;
		//NOTE: this loop needs to go from negative to positive
		ppu.sprites[spri_idx[0]].x = uint8_t(player.at.x - 4);
		ppu.sprites[spri_idx[0]].y = uint8_t(player.at.y + 4);
		//Set the sprite in the tile table, might make it a varaible
		ppu.sprites[spri_idx[0]].index = 47;
		ppu.sprites[spri_idx[0]].attributes = color;

		ppu.sprites[spri_idx[1]].x = uint8_t(player.at.x + 4);
		ppu.sprites[spri_idx[1]].y = uint8_t(player.at.y + 4);
		ppu.sprites[spri_idx[1]].index = 48;
		ppu.sprites[spri_idx[1]].attributes = color;

		ppu.sprites[spri_idx[2]].x = uint8_t(player.at.x - 4);
		ppu.sprites[spri_idx[2]].y = uint8_t(player.at.y - 4);
		ppu.sprites[spri_idx[2]].index = 54;
		ppu.sprites[spri_idx[2]].attributes = color;

		ppu.sprites[spri_idx[3]].x = uint8_t(player.at.x + 4);
		ppu.sprites[spri_idx[3]].y = uint8_t(player.at.y - 4);
		ppu.sprites[spri_idx[3]].index = 55;
		ppu.sprites[spri_idx[3]].attributes = color;

		/*
		uint8_t count = 0;

		for (uint8_t h = 0; h < player.dim.y; h++) {
			for (uint8_t w = 0; w < player.dim.x; w++) {
				//create offset
				float offset_x = w - player.dim.x / 2.0f;
				float offset_y = h - player.dim.y / 2.0f;
				ppu.sprites[spri_idx[count]].x = uint8_t(player.at.x + offset_x*8);
				ppu.sprites[spri_idx[count]].y = uint8_t(player.at.y + offset_y*8);
				ppu.sprites[spri_idx[count]].index = 33 + count;
				ppu.sprites[spri_idx[count]].attributes = color;
				//std::cout << "x: " << unsigned(ppu.sprites[spri_idx[count]].x) << "   y: " << unsigned(ppu.sprites[spri_idx[count]].y) << "   count:"  << unsigned(33+ count)<< std::endl;

				count++;
			}
			//std::cout << "---------------rows: " << unsigned(h) << std::endl;
		}
		*/


		//draw weapon
		ppu.sprites[player.weapon_idx].x = int8_t(player.at.x);
		ppu.sprites[player.weapon_idx].y = int8_t(player.at.y);
		ppu.sprites[player.weapon_idx].index = 32;
		ppu.sprites[player.weapon_idx].attributes = player.weapon_color;
	};

	player_draw(player0, ppu);
	//player_draw(player1, ppu);

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
