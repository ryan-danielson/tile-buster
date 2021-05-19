/* Using SDL and standard IO */
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <math.h>

/* screen dimension constants */
const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 768;
const int CEILING       = 1;
const int WALL          = 2;
const int GAME_OVER     = -1;

/* surface images */
char* BACKGROUND= "Tiles/background.png";
char* TILE1	= "Tiles/TILE1.png";
char* TILE2     = "Tiles/TILE2.png";
char* TILE3     = "Tiles/TILE3.png";
char* TILE4     = "Tiles/TILE4.png";
char* PLAYER	= "Tiles/LAZER.png";
char* BALL      = "Tiles/BALL.png";
char* WIN       = "Tiles/WIN.png";
char* LOSS      = "Tiles/LOSS.png";
int TOTAL_TILES = 5;

enum KeyPressSurfaces
{
	KEY_PRESS_SURFACE_PLAYER,
        KEY_PRESS_SURFACE_BALL,
	KEY_PRESS_SURFACE_TOTAL		// 2
};

struct MapTile
{
    SDL_Rect rect;
    SDL_Texture *texture;
    int alive;
};
typedef struct MapTile MapTile;

struct MapSize
{
	int x;
	int y;
};
typedef struct MapSize MapSize;

struct Direction
{
        float x;
        float y;
};
typedef struct Direction Direction;

struct Ball
{
        int alive;
        Direction direction;
        SDL_Rect ballRect;
};
typedef struct Ball Ball;

SDL_Window* init(SDL_Window* window, SDL_Surface* screenSurface, SDL_Renderer** gRenderer);
_Bool loadMedia(SDL_Texture** gKeyPressSurfaces, SDL_Renderer** gRenderer, SDL_Texture** mapTiles, Mix_Chunk **effects);
SDL_Texture* loadTexture(char* somePath, SDL_Renderer** gRenderer);
void updatePlayer(SDL_Window** window, SDL_Surface* screenSurface, SDL_Surface* gCurrentSurface, SDL_Rect playerRect);
_Bool mapDraw(SDL_Renderer* gRenderer, SDL_Rect* tileRect, SDL_Texture** mapTiles, MapSize dimensions, int map[][dimensions.x], MapTile mapArray[][dimensions.x]);
int boundsCheck(MapSize dimensions, MapTile mapArray[][dimensions.x], Ball ball, SDL_Rect playerRect);
MapSize mapDimensions(FILE *p);
_Bool mapReader(FILE *p, MapSize dimensions, int map[][dimensions.x], MapTile mapArray[][dimensions.x], SDL_Texture **mapTiles);
Ball ballLaunch(SDL_Rect playerRect);
SDL_Rect ballUpdate(Ball ball);
Direction newAngle(Direction angle, int surface);
void end(SDL_Window* window, SDL_Surface* screenSurface, SDL_Texture* gCurrentSurface, SDL_Texture** gKeyPressSurfaces, SDL_Texture** mapTiles);


int main( int argc, char* args[] )
{
	
        SDL_Window* window = NULL;
        SDL_Surface* screenSurface = NULL;
        SDL_Texture* gKeyPressSurfaces[ KEY_PRESS_SURFACE_TOTAL ];
        SDL_Texture* gCurrentTexture = NULL;
        SDL_Surface* mapBackground;
        SDL_Texture* mapTiles[TOTAL_TILES]; 
        SDL_Surface* screenClear;
        screenClear = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,0,0,0,0);
  
        SDL_FillRect(screenClear, NULL, SDL_MapRGB(screenClear->format, 0,0,0));
        
        Mix_Chunk *effects[2];
  
        SDL_Renderer* gRenderer = NULL;
        SDL_Texture* gTexture = NULL;


        _Bool quit = 0;
        SDL_Event e;	
        gCurrentTexture = gKeyPressSurfaces[ KEY_PRESS_SURFACE_PLAYER ];

        SDL_Rect* playerRect = NULL;
        playerRect = malloc(sizeof(SDL_Rect));
        playerRect->x = 512;
        playerRect->y = 640;
        playerRect->h = 8;
        playerRect->w = 96;

        SDL_Rect* tileRect = NULL;
        tileRect = malloc(sizeof(SDL_Rect));
        tileRect->x = 0;
        tileRect->y = 0;
        tileRect->h = 32;
        tileRect->w = 64;

        SDL_Rect *backRect = NULL;
        backRect = malloc(sizeof(SDL_Rect));
        SDL_Texture *backTexture;
        backRect->w = SCREEN_WIDTH;
        backRect->h = SCREEN_HEIGHT;
        backRect->x = 0;
        backRect->y = 0;

        SDL_Rect Loss;
        SDL_Texture *lossTexture; 
        Loss.w = 512;
        Loss.h = 128;
        Loss.x = 256;
        Loss.y = 320;
       
        /* create window */
        window = init(window, screenSurface, &gRenderer);
        screenSurface = SDL_GetWindowSurface(window);

        backTexture = loadTexture(BACKGROUND, &gRenderer);
        lossTexture = loadTexture(LOSS, &gRenderer);

        
        /* load media to window */
    
        puts("loading media");
        if(!loadMedia(&gKeyPressSurfaces[0], &gRenderer, &mapTiles[0], &effects[0])) {
	        puts("Failed to load media.");
	        return 1;
        }
        
        /* open and close tile map */

        FILE *p = fopen("tilemap.txt", "rw+");
        if (p == NULL)
	        puts("Error opening tilemap");    
    
        MapSize dimensions = mapDimensions(p);
        if (&dimensions.x == NULL || &dimensions.y == NULL) {
	        puts("Failed to load map dimensions");
	        return 1;
        }

        printf("Map dimensions: %dx%d\n", dimensions.x, dimensions.y);
        int map[dimensions.y][dimensions.x];
        MapTile mapArray[dimensions.y][dimensions.x];
    
        if(!mapReader(p, dimensions, map, &mapArray[0], &mapTiles[0])) {
	        puts("error reading map");
	        return 1;
        }
        fclose(p);
//        puts("Media loaded");
    
        /* print map and player starting position */
        if(!mapDraw(gRenderer, tileRect, mapTiles, dimensions, map, &mapArray[0])) {
	        puts("Tile are empty in mapMaker()");
	        return 1;
        }	
        gCurrentTexture = gKeyPressSurfaces[ KEY_PRESS_SURFACE_PLAYER ];

        /* use for bounds checking */ 
        SDL_Rect playerPositionTracker;
        playerPositionTracker.w = 64;
        playerPositionTracker.h = 64;
        playerPositionTracker.x = playerRect->x;
        playerPositionTracker.y = playerRect->y;
    
        Ball ball;
        ball.alive = 0;

        while (!quit) {       
	        while (SDL_PollEvent(&e) != 0) {
		        if (e.type == SDL_QUIT)
			        quit = 1;
	        }

	        const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
	        if (currentKeyStates[SDL_SCANCODE_LEFT]) {
		        gCurrentTexture = gKeyPressSurfaces[KEY_PRESS_SURFACE_PLAYER];
                        if (!ball.alive) {
                                if (playerRect->x - 16 >= 256) 
                                        playerRect->x -= 32;
                        } else if (playerRect->x-16 >= 0)
		                playerRect->x -= 32;
	        } else if (currentKeyStates[SDL_SCANCODE_RIGHT]) {
		        gCurrentTexture = gKeyPressSurfaces[KEY_PRESS_SURFACE_PLAYER];
                        if (!ball.alive) {
                                if (playerRect->x+103 <= 768)
                                        playerRect->x += 32;
		        } else if (playerRect->x+104 <= 1024)
                                playerRect->x += 32;
	        } else if (currentKeyStates[SDL_SCANCODE_SPACE]) {
		        if (!ball.alive) {
                                puts("New ball created");
                                ball = ballLaunch(*playerRect);
                        } 
                    
	        } else
		    //gCurrentTexture = gKeyPressSurfaces[KEY_PRESS_SURFACE_DEFAULT];	
	
        SDL_RenderClear(gRenderer); 
        SDL_RenderCopy(gRenderer, backTexture, NULL, backRect);
	mapDraw(gRenderer, tileRect, mapTiles, dimensions, map, mapArray);
	SDL_RenderCopy(gRenderer, gCurrentTexture, NULL, playerRect);
	if (ball.alive) {
                ball.ballRect = ballUpdate(ball);

                switch (boundsCheck(dimensions, mapArray, ball, *playerRect)) {
                case 0:
                        ball.ballRect = ballUpdate(ball);
                        SDL_RenderCopy(gRenderer, gKeyPressSurfaces[KEY_PRESS_SURFACE_BALL], NULL, &ball.ballRect);
                        break;
                case 1:
                        ball.direction = newAngle(ball.direction, CEILING);
                        ball.ballRect  = ballUpdate(ball);
                        Mix_PlayChannel(-1, effects[0], 0);
                        SDL_RenderCopy(gRenderer, gKeyPressSurfaces[KEY_PRESS_SURFACE_BALL], NULL, &ball.ballRect);
                        break;
                case 2:
                        ball.direction = newAngle(ball.direction, WALL);
                        ball.ballRect  = ballUpdate(ball);
                        Mix_PlayChannel(-1, effects[0], 0);
                        SDL_RenderCopy(gRenderer, gKeyPressSurfaces[KEY_PRESS_SURFACE_BALL], NULL, &ball.ballRect);
                        break;
                case -1:                 
                        SDL_RenderCopy(gRenderer, lossTexture, NULL, &Loss);
                        SDL_RenderPresent(gRenderer); 
                        SDL_Delay(500);
                        quit = 1;
                        break;
                default:
                //        ball.ballRect = ballUpdate(ball);
                        break;
                }
           }
        SDL_RenderPresent(gRenderer);
        SDL_Delay(40);
    }
    

    /* cleanup and exit */
    SDL_DestroyRenderer(gRenderer);
    Mix_FreeChunk(effects[0]);
    Mix_FreeChunk(effects[1]);
    effects[0] = NULL;
    effects[1] = NULL;
    free(tileRect);
    free(playerRect);
    free(backRect);
    SDL_DestroyTexture(backTexture);
    Mix_Quit();
    IMG_Quit();
    end(window, screenSurface, gCurrentTexture, &gKeyPressSurfaces[0], &mapTiles[0]);
    return 0;
}

SDL_Window*
init(SDL_Window* window, SDL_Surface* screenSurface, SDL_Renderer** gRenderer)
{
    /* Initialize SDL */
    if (SDL_Init( SDL_INIT_VIDEO || SDL_INIT_AUDIO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    	return 0;
    } else {
        /*  Create window  */
        window = SDL_CreateWindow( "SDL Isometric", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        if (window == NULL) {
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
            return 0;
	} else {
		*gRenderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED);
		if (gRenderer == NULL)
			printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
	        else { 
			SDL_SetRenderDrawColor(*gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

	       		 int imgFlags = IMG_INIT_PNG;
	        	if (!(IMG_Init(imgFlags) & imgFlags)) {
		                 printf("SDL_image could not initilize! SDL_image error: %s\n", IMG_GetError());
	    	        	 return 0;
		        }

                        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
                                printf("SDL_mixer could not initialize! SDL_mixer error: %s\n", Mix_GetError());
                                return 0;
                        }
      	        }
		puts("exit init");
       		return window;
                	    
        }
    }
}

_Bool
loadMedia(SDL_Texture** gKeyPressSurfaces, SDL_Renderer** gRenderer, SDL_Texture** mapTiles, Mix_Chunk **effects)
{

	gKeyPressSurfaces[ KEY_PRESS_SURFACE_PLAYER ] = loadTexture(PLAYER, gRenderer);
	gKeyPressSurfaces[ KEY_PRESS_SURFACE_BALL ] = loadTexture(BALL, gRenderer);

	mapTiles[0] = loadTexture(TILE1, gRenderer);
	mapTiles[1] = loadTexture(TILE2, gRenderer);
	mapTiles[2] = loadTexture(TILE3, gRenderer);
        mapTiles[3] = loadTexture(TILE4, gRenderer);
	
        effects[0] = Mix_LoadWAV("audio/surface.wav");
        effects[1] = Mix_LoadWAV("audio/destroy.wav");
	/* check for NULL key press surfaces */

	for (int i = 0; i < KEY_PRESS_SURFACE_TOTAL; ++i) {
		if (&gKeyPressSurfaces[i] == NULL) {
			printf("Failed to load keypress surface %d\n", i);
			return 0;
		}
	}

	/* check for NULL tiles */
	for (int i = 0; i < TOTAL_TILES; ++i) {
		if (&mapTiles[i] == NULL) {
			printf("Failed to load TILE%d\n", i);
			return 0;
		}
	}
	puts("exit loadMedia");
	return 1;
}

SDL_Texture*
loadTexture(char* somePath, SDL_Renderer** gRenderer)
{
	SDL_Texture* newTexture = NULL;
	SDL_Surface* loadedSurface = IMG_Load(somePath);
	if (loadedSurface == NULL)
		puts("Unable to load image");

	newTexture = SDL_CreateTextureFromSurface(*gRenderer, loadedSurface);
	if (newTexture == NULL)
		printf("Unable to create texture! SDL Error: %s\n", SDL_GetError());

	/* free old surface */

	SDL_FreeSurface(loadedSurface);
	puts("exit loadTexture");
	return newTexture;
}

MapSize 
mapDimensions(FILE* p)
{
	MapSize dimension;

	fscanf(p, "%d %d", &dimension.y, &dimension.x);
	printf("MapSize: %d x %d\n", dimension.x, dimension.y);
	puts("exit mapDimensions");
	return dimension;
}

_Bool
mapReader(FILE* p, MapSize dimensions, int map[][dimensions.x], MapTile mapArray[][dimensions.x], SDL_Texture **mapTiles)
{
        int x = 0;
        int y = 0;
        for (int i = 0; i < dimensions.y; ++i) {
		for (int k = 0; k < dimensions.x; ++k) {
                         
			fscanf(p, "%d", &map[i][k]);
                        if (map[i][k] != 0) {
                                mapArray[i][k].rect.w = 62;
                                mapArray[i][k].rect.h = 30;
                                mapArray[i][k].rect.x = x;
                                mapArray[i][k].rect.y = y;
                                mapArray[i][k].alive = 1;
                                switch (map[i][k]) {
                                case 1:
                                        mapArray[i][k].texture = mapTiles[1];
                                        break;
                                case 2:
                                        mapArray[i][k].texture = mapTiles[2];
                                        break;
                                case 3:
                                        mapArray[i][k].texture = mapTiles[3];
                                        break;
                                default:
                                        break;
                                } 
                        } else if (map[i][k] == 0) {
                                mapArray[i][k].rect.w = 62;
                                mapArray[i][k].rect.h = 30;
                                mapArray[i][k].rect.x = x;
                                mapArray[i][k].rect.y = y;
                                mapArray[i][k].alive = 0;
                                mapArray[i][k].texture = mapTiles[0];
                        }
                        x += 64;
                        
			// printf("%d ", map[i][k]);
		}
		// puts(" ");
                x = 0;
                y += 32;
	}
	
	return 1;
}

int
boundsCheck(MapSize dimensions, MapTile mapArray[][dimensions.x], Ball ball, SDL_Rect playerRect)
{
        SDL_Rect temp;
        
        if (ball.ballRect.x >= 960 || ball.ballRect.x <= 0) {
                return WALL;
        } else if (ball.ballRect.y <= 0) {
                return CEILING; 
        } else if (ball.ballRect.y > 702) {
                return GAME_OVER;
        } else if (SDL_IntersectRect(&playerRect, &ball.ballRect, &temp) && ball.direction.y > 0) {
                return CEILING;
        } else  if (ball.ballRect.y <= 192) {
                for (int i = 0; i < dimensions.y; ++i) {
                        for (int j = 0; j < dimensions.x; ++j) {
                                if (SDL_IntersectRect(&ball.ballRect, &mapArray[i][j].rect, &temp) && mapArray[i][j].alive == 1) { 
                                        mapArray[i][j].alive = 0;                             
                                        return CEILING;
                                }
                        }
                }
        }
	return 0;
}

_Bool
mapDraw(SDL_Renderer* gRenderer, SDL_Rect* tileRect, SDL_Texture** mapTiles, MapSize dimensions, int map[][dimensions.x], MapTile mapArray[][dimensions.x])
{
        puts("mapDraw()");
	if(&mapTiles[0] == NULL || &mapTiles[1] == NULL || &mapTiles[2] == NULL) {
		puts("mapTiles has no content in mapMaker");
		return 0;
	}
	
	for (int i = 0; i < dimensions.y; ++i) {
		for (int k = 0; k < dimensions.x; ++k) {       
                        if (mapArray[i][k].alive == 1)
                                 SDL_RenderCopy(gRenderer, mapArray[i][k].texture, NULL, &mapArray[i][k].rect);
                }
	}

	return 1;
}

Ball
ballLaunch(SDL_Rect playerRect)
{
        Ball ballStart;
        ballStart.ballRect.x = playerRect.x+32;
        ballStart.ballRect.y = playerRect.y-64;
        ballStart.ballRect.w = 24;
        ballStart.ballRect.h = 24;
        if (playerRect.x > 0 && playerRect.x < 360) {
                ballStart.direction.x = cos(5*M_PI/3);
                ballStart.direction.y = sin(5*M_PI/3);
                printf("Ball direction: (%f, %f)", ballStart.direction.x, ballStart.direction.y);
        } else if (playerRect.x >= 360 && playerRect.x < 768) {
                ballStart.direction.x = cos(5*M_PI/3);
                ballStart.direction.y = sin(5*M_PI/3);
                printf("Ball direction: (%f, %f)", ballStart.direction.x, ballStart.direction.y);
        }
        ballStart.alive = 1;
        return ballStart;
}

SDL_Rect
ballUpdate(Ball ball)
{
        puts("ballUpdate()");
        SDL_Rect ballUpdate;
        ballUpdate.x = ball.ballRect.x;
        ballUpdate.y = ball.ballRect.y;
        ballUpdate.x += (ball.direction.x)*16;
        ballUpdate.y += (ball.direction.y)*16;
        ballUpdate.w = ball.ballRect.w;
        ballUpdate.h = ball.ballRect.h;
        // printf("Ball: (%d, %d)\n", ballUpdate.x, ballUpdate.y);
        return ballUpdate; 
}

Direction
newAngle(Direction angle, int surface)
{
        Direction newAngle;
        if (surface == WALL) {
                newAngle.x = -(angle.x);
                newAngle.y = (angle.y);
        }
        if (surface == CEILING) {
                newAngle.x = angle.x;
                newAngle.y = -(angle.y);
        }
        return newAngle; 
}

void
end(SDL_Window* window, SDL_Surface* screenSurface, SDL_Texture* gCurrentSurface, SDL_Texture** gKeyPressSurfaces, SDL_Texture** mapTiles)
{
        SDL_DestroyTexture(gCurrentSurface);
	SDL_FreeSurface(screenSurface);
	SDL_DestroyTexture(*gKeyPressSurfaces);
	SDL_DestroyTexture(*mapTiles);
	screenSurface = NULL;
	gCurrentSurface = NULL;
	gKeyPressSurfaces = NULL;
	mapTiles = NULL;

	/* destroy window */
	SDL_DestroyWindow( window );

	/* quite SDL subsystem */
	SDL_Quit();
}


