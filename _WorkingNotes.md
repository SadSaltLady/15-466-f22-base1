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
