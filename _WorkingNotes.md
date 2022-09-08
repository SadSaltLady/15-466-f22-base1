## Game Description
A local two player fighting game, where you attach in the direction you move in.

## Parts to implement 
*Input*
- support WASD and Arrow Input, each controlling its own sprite

*Art*
- Players
- Background Tiles
- Weapon(swords)
- (Optional) Colliding swords
- (Optional) Death Sprites

*AssetPipeline* `important!!`
How do I get tiles sprites into the game and use them?

*GameLogic*
- Check for collision in input direction -> end game 
- DONE - Move in the input direction 
- move sword in the input direction

## Steps to Understand the Code
0. Skim over the rest of the chapter
1. Figure out where it is drawing the sprites
2. Figure out how does the input work

## Notes on Understanding main.cpp
 1. bunch of hardware initializations-- like GLs, platform specific thing
 2. Set up Vsync with SDL_GL_SetSwapInterval
 3. Load Assets by calling load function -- somehow??? -- ASK
 4. Create Game Mode
 5. Main Looooop
    1. process any  pending events (resizing, inputs , screenshots)
        *USEFUL* Inputs handled in GameMode, PlayMode.cpp->PlayMode::handle_event
                Buttons also defined in PlayMode.hpp
        **TODO** Make this support WASD, also add to definition in hpp
    2. Call the current Mode's update function (w elapsed time)
        *USEFUL* Update is in Playmode::update
                updates player position (per tick), background, and arrows
        **TODO** Add position update for the other player (and Weapons!)
    3. Draw! 
        [TODO]: take a closer look at the tile drawing...
        ppu structure seems important!
        PPU data structure(draw only) ->> PPU466.hpp
            - background layer(16 bit,more details see file)
            - sprite[32 bit], alawys 64 sprite on screen, move off screen to hide

## Implementing the asset pipeline 
- Use load to load the assets into variables stored in global, or in our case, 
load into PPU sprite buffers (ideally?)
- we will need a dynamic look up function to get the path for our resources
- we will need a function that is able to parse it
- we need something (Load) to put it in a usable place

## some really sketchy code as a basis
REFERENCE-----
	struct Tile {
		std::array< uint8_t, 8 > bit0; //<-- controls bit 0 of the color index
		std::array< uint8_t, 8 > bit1; //<-- controls bit 1 of the color index
	};
REFERENCE-----
struct SpriteAtlas {
    //idea: take in one big sprite file
    //constructor that reads from sprite file
    //takes in a png, and should parese into the three channels
    //store that data struct in a vector


}
Load< SpriteAtlas > mars_atlas( LoadTagDefault, [&](){
	SpriteAtlas *ret = new SpriteAtlas(data_path("some data"));
	lander_sprite = ret->lookup("lander");
	rock_sprite = ret->lookup("rock");
});

## how much sprites will I need to make guilty goose...
1. idle ((4 * 4) + 4) = 20 (optimize = 16)
2. prep frame optimized = 4
2. attack optimized = 6
3. fly optimized = 9 ~ 10

total : (16 + 4 + 6 + 10) * 2 = 36 * 2 = 72 tiles... 

## there should be a github action code that one can run

## new todo now that you're making a fighiting game...
1. a way to load in large amount of assets & index
2. new movement mechanic (ie, side ways with a floor )
3. how to do animation on a time based and not frame-based fashion
3. figure out a way to author hitbox and hurt box

