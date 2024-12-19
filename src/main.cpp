#include <iostream>
#include <string>
#include <SDL.h>

using namespace std;


int main()
{
    if(SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		printf("SDL Audio could not initialize! SDL Error: %s\n", SDL_GetError());
		return 1;
	}


    SDL_Quit();
    return 0;
}
