#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define INPUT_MAX 500

typedef struct File_Header{
    uint16_t width;
    uint16_t height;
    uint8_t audio_khz;
    uint16_t chunks_per_second; // chunk is 10x10 pixels
}File_Header;

int width = 0;
int height = 0;
unsigned char *video_data;
unsigned char *audio_data;
File_Header specs;

bool strComp(char *input, char *comp) // compares strings
{
    bool same = true;
    for (int x = 0; x < strlen(comp); x++)
    {
        if (input[x] != comp[x])
        {
            same = false;
        }
    }
    return same;
}

void help()
{
    printf(" exit\n");
    printf(" convert (convert png image sequence and wav audio to character video)\n");
    printf(" play (play video currently in RAM)\n");
    printf(" load (arg: filename  load existing charvid file)\n");
    printf(" save (arg: filename  save charvid video currently in RAM)\n");

    printf("\n Image should be no bigger than 120x120. (expects image sequence ex: 0000.png, 0001.png)\n (expects audio.wav)\n");
}

void convert()
{
    char filename[INPUT_MAX];
    unsigned long int count = 0;
    unsigned char *img, pixel;
    int img_w, img_h, channels, quality, img_fps, audio_khz, chunks_per_second;

    // UI chunks_per_second
    printf(" Quality (1-10): ");
    scanf("%d", &quality);
    if (quality > 10){quality = 10;}
    else if (quality < 1){quality = 1;}

    // UI source frame rate (fps)
    printf(" Original Framerate (fps): ");
    scanf("%d", &img_fps);

    // UI chunks_per_second
    printf(" Audio kHz (4-22): ");
    scanf("%d", &audio_khz);
    if (audio_khz > 22){audio_khz = 22;}
    else if (audio_khz < 4){audio_khz = 4;}

    int prev_chunk_buff[20][20]; // chunk averages stored here for comparison
    int curr_chunk_buff[20][20];
    for(int y = 0; y < 20; y++) // zero out buff
    {
        for(int x = 0; x < 20; x++)
        {
            prev_chunk_buff[y][x] = 0;
            curr_chunk_buff[y][x] = 0;
        }
    }

    bool once = true;
    bool skip = false;
    int avr, avr_count;
    while(1)
    {
        memset(filename, 0, strlen(filename));
        sprintf(filename, "input/%04d.png", count);
        img = stbi_load(filename, &img_w, &img_h, &channels, 1);
        if (img == NULL)
        {
            printf(" Couldn't load image: %s\n", filename);
            printf(" continue? (y/n): ");
            char prmt;
            scanf("%s", &prmt);
            if (prmt != 'y')
            {
                printf(" Stopped.\n");
                break;
            }
            skip = true;
        }

        if (!skip)
        {
            if (once) // do the following only once
            {
                printf(" Image size = %d x %d (120x120 or smaller recommended)\n", img_w, img_h);
                specs.width = (uint16_t)img_w;
                specs.height = (uint16_t)img_h;
                chunks_per_second = (img_w * img_h) / 10; // number of chunks in entire frame
                chunks_per_second = (double)chunks_per_second * ((double)quality * 0.1); // chunks_per_second based off 1-10 quality
                specs.chunks_per_second = (uint16_t)chunks_per_second;
                specs.audio_khz = (uint8_t)audio_khz;
                once = false;
            }

            // find current chunk averages and store in curr_chunk_buff
            avr = 0; avr_count = 0;
            for (int chunk_y = 0; chunk_y < (img_h / 10); chunk_y++)
            {
                for (int chunk_x = 0; chunk_x < (img_w / 10); chunk_x++)
                {
                    for (int y = 0; y < 10; y++)
                    {
                        for (int x = 0; x < 10; x++)
                        {
                            pixel = img[(y+(chunk_y*10))*img_w+(x+(chunk_x*10))];
                            avr += pixel;
                            avr_count++;
                        }
                    }
                    curr_chunk_buff[chunk_y][chunk_x] = avr / avr_count;
                    avr_count = 0;
                    avr = 0;
                }
            }

        } else {
            skip = false;
        }
        count++;
        if (img != NULL)
        {
            free(img);
        }
    }
}

int main()
{
    if(SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		printf("SDL Audio could not initialize! SDL Error: %s\n", SDL_GetError());
		return 1;
	}

    bool run = true;
    char input[INPUT_MAX];
    char command[INPUT_MAX];
    char arg[INPUT_MAX];
    char *token;
    while (run)
    {
        while(1)
        {
            printf("command: ");
            fgets(input, INPUT_MAX, stdin);
            if (input[0] == '\r' || input[0] == '\n')
            {
                break;
            }
            command[0] = '\0';
            arg[0] = '\0';
            token = strtok(input, " \n\t\r");
            strcpy(command, token);
            token = strtok(NULL, " \n\t\r");
            if (token != NULL)
            {
                strcpy(arg, token);
            }

            if (strComp(command, "exit\0"))
            {
                run = false;
            } else if (strComp(command, "help\0"))
            {
                help();
            } else if (strComp(command, "convert\0"))
            {
                convert();
            }

            if (token != NULL)
            {
                free(token);
            }
            break;
        }
    }

    SDL_Quit();
    return 0;
}
