#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define INPUT_MAX 500
const int chunk_size = 12;

const unsigned char std_chunk_id = 10; // ID for standard 10x10 chunk
const unsigned char key_chunk_id = 255; // ID for keyframe chunk
const unsigned char stop_id = 244; // ID to stop video. end of datastream.

typedef struct File_Header{
    uint16_t width;
    uint16_t height;
    uint8_t fps;
    uint32_t chunks_per_second; // chunk is 10x10 pixels
}File_Header;

int width = 0;
int height = 0;
unsigned char *video_data;
unsigned long int video_data_size = 0;
unsigned long int video_data_pos;
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
    printf(" convert (convert png image sequence to character video)\n");
    printf(" play (play video currently in RAM)\n");
    printf(" load (arg: filename  load existing charvid file)\n");
    printf(" save (arg: filename  save charvid video currently in RAM)\n");

    printf("\n Image should be no bigger than 120x120. (expects image sequence ex: 0000.png, 0001.png)\n");
}

void video_mem_allocation()
{
    if (video_data == NULL)
    {
        video_data_size = 10000;
        video_data_pos = 0;
        video_data = malloc(video_data_size);
        memset(video_data, 0, video_data_size);
    } else if (video_data_pos > video_data_size - 5000)
    {
        video_data_size += 10000;
        video_data = (unsigned char*)realloc(video_data, video_data_size);
        memset(&video_data[video_data_pos], 0, video_data_size-video_data_pos);
    }
}

void convert()
{

    if (video_data != NULL)
    {
        free(video_data);
        video_data = NULL;
        video_data_size = 0;
    }

    video_mem_allocation();

    char filename[INPUT_MAX];
    unsigned long int count = 0;
    unsigned char *img, pixel;
    int img_w, img_h, channels, quality, img_fps, chunks_per_second;

    // UI chunks_per_second
    printf(" Quality (1-10): ");
    scanf("%d", &quality);
    if (quality > 10){quality = 10;}
    else if (quality < 1){quality = 1;}

    // UI source frame rate (fps)
    printf(" Original Framerate (24> fps recommended): ");
    scanf("%d", &img_fps);

    int prev_chunk_buff[1000][1000]; // chunk averages stored here for comparison
    int curr_chunk_buff[1000][1000];
    for(int y = 0; y < 1000; y++) // zero out buff
    {
        for(int x = 0; x < 1000; x++)
        {
            prev_chunk_buff[y][x] = 0;
            curr_chunk_buff[y][x] = 0;
        }
    }
    int priority_chunks[2000][2];

    bool once = true;
    bool skip = false;
    int chunks_needed = 1;
    int keyframe_period = 0;
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
                video_data[video_data_pos++] = stop_id;
                video_data[video_data_pos++] = stop_id;
                video_data[video_data_pos++] = stop_id;
                video_data[video_data_pos++] = stop_id;
                break;
            }
            skip = true;
        }

        if (!skip)
        {
            if (once) // do the following only once
            {
                printf(" Image size = %d x %d (100x56 or smaller recommended)\n", img_w, img_h);
                specs.width = (uint16_t)img_w;
                specs.height = (uint16_t)img_h;
                chunks_per_second = (img_w * img_h) / chunk_size; // number of chunks in frame
                chunks_per_second = (double)chunks_per_second * ((double)quality*0.1); // chunks_per_second based off 1-10 quality
                if (chunks_per_second < 1){chunks_per_second = 1;}
                specs.chunks_per_second = (uint32_t)chunks_per_second;
                chunks_needed = chunks_per_second / img_fps; // chunks needed per frame
                if (chunks_needed < 1){chunks_needed = 1;}
                specs.fps = img_fps;
                once = false;
            }

            if (keyframe_period == 0)
            {
                keyframe_period = img_fps;
                video_data[video_data_pos++] = key_chunk_id;

                for(int y = 0; y < img_h; y++)
                {
                    for(int x = 0; x < img_w; x++)
                    {
                        pixel = img[y*img_w+x];
                        pixel /= 16;
                        video_data[video_data_pos++] = pixel;
                    }
                    video_mem_allocation();
                }

                // fill prev_chunk_buff from keyframe
                for (int chunk_y = 0; chunk_y < (img_h / chunk_size); chunk_y++)
                {
                    for (int chunk_x = 0; chunk_x < (img_w / chunk_size); chunk_x++)
                    {
                        pixel = img[((chunk_size/2)+(chunk_y*chunk_size))*img_w+((chunk_size/2)+(chunk_x*chunk_size))];
                        prev_chunk_buff[chunk_y][chunk_x] = pixel;
                    }
                }

            } else {
                keyframe_period--;

                // find current chunk averages and store in curr_chunk_buff
                for (int chunk_y = 0; chunk_y < (img_h / chunk_size); chunk_y++)
                {
                    for (int chunk_x = 0; chunk_x < (img_w / chunk_size); chunk_x++)
                    {
                        pixel = img[((chunk_size/2)+(chunk_y*chunk_size))*img_w+((chunk_size/2)+(chunk_x*chunk_size))];
                        curr_chunk_buff[chunk_y][chunk_x] = pixel;
                    }
                }

                // compare curr chunk averages to prev and prioritize the biggest differences
                int peak_diff = 0;
                int flagged_chunk[2];
                int diff = 0;
                int chunk_count = 0;
                while (chunk_count < chunks_needed)
                {
                    peak_diff = 0;
                    for (int chunk_y = 0; chunk_y < (img_h / chunk_size); chunk_y++)
                    {
                        for (int chunk_x = 0; chunk_x < (img_w / chunk_size); chunk_x++)
                        {
                            diff = curr_chunk_buff[chunk_y][chunk_x] - prev_chunk_buff[chunk_y][chunk_x];
                            if (diff < 0)
                            {
                                diff = 0 - diff;
                            }
                            if (diff > peak_diff)
                            {
                                peak_diff = diff;
                                flagged_chunk[0] = chunk_y;
                                flagged_chunk[1] = chunk_x;
                            }
                        }
                    }
                    curr_chunk_buff[flagged_chunk[0]][flagged_chunk[1]] = prev_chunk_buff[flagged_chunk[0]][flagged_chunk[1]];
                     // ^^ equalize flagged chunk so it isn't re-detected
                    priority_chunks[chunk_count][0] = flagged_chunk[0];
                    priority_chunks[chunk_count++][1] = flagged_chunk[1];
                }

                // copy curr_chunk_buff to prev_chunk_buff
                for (int chunk_y = 0; chunk_y < (img_h / chunk_size); chunk_y++)
                {
                    for (int chunk_x = 0; chunk_x < (img_w / chunk_size); chunk_x++)
                    {
                        prev_chunk_buff[chunk_y][chunk_x] = curr_chunk_buff[chunk_y][chunk_x];
                    }
                }

                // send selected chunks to memory and prev_chunk_buff
                // chunk id, chunk_y, chunk_x, chunk_pixels...
                for(int chunk = 0; chunk < chunks_needed; chunk++)
                {
                    video_data[video_data_pos++] = std_chunk_id;
                    video_data[video_data_pos++] = priority_chunks[chunk][0]; // y
                    video_data[video_data_pos++] = priority_chunks[chunk][1]; // x
                    for (int y = 0; y < chunk_size; y++)
                    {
                        for (int x = 0; x < chunk_size; x++)
                        {
                            pixel = img[(y+(priority_chunks[chunk][0]*chunk_size))*img_w+(x+(priority_chunks[chunk][1]*chunk_size))];
                            pixel /= 16;
                            video_data[video_data_pos++] = pixel;
                        }
                    }
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
        video_mem_allocation();
    }
}

void print_pixel(unsigned char pixel)
{
    switch (pixel)
    {
        case 0: printf("#"); break;
        case 1: printf("#"); break;
        case 2: printf("X"); break;
        case 3: printf("0"); break;
        case 4: printf("O"); break;
        case 5: printf("C"); break;
        case 6: printf("{"); break;
        case 7: printf("]"); break;
        case 8: printf("|"); break;
        case 9: printf(";"); break;
        case 10: printf(":"); break;
        case 11: printf("~"); break;
        case 12: printf("-"); break;
        case 13: printf("."); break;
        case 14: printf(" "); break;
        case 15: printf(" "); break;
        default:
            printf(" ");
            break;
    }
}

void delay(int delay_ms)
{
    // Storing start time
    clock_t start_time = clock();

    // looping till required time is not achieved
    while (clock() < start_time + (delay_ms*1000));
}

void play()
{
    unsigned char pixel;
    unsigned char frame_buff[1000][1000];
    system("clear");
    if (video_data == NULL || video_data_size == 0)
    {
        printf(" Nothing in RAM..\n");
        return;
    }
    video_data_pos = 0;

    int chunk_count = 0;
    int delay_trigger = specs.chunks_per_second / specs.fps;

    while (video_data_pos < video_data_size)
    {
        if (video_data[video_data_pos] == stop_id)
        {
            break;
        } else if (video_data[video_data_pos] == key_chunk_id)
        {
            chunk_count = delay_trigger;
            video_data_pos++;
            for(int y = 0; y < specs.height; y++)
            {
                for(int x = 0; x < specs.width; x++)
                {
                    pixel = video_data[video_data_pos++];
                    frame_buff[y][x] = pixel;
                }
            }
        } else if (video_data[video_data_pos] == std_chunk_id)
        {
            chunk_count++;
            video_data_pos++;
            int chunk_y = video_data[video_data_pos++];
            int chunk_x = video_data[video_data_pos++];
            for(int y = 0; y < chunk_size; y++)
            {
                for(int x = 0; x <chunk_size; x++)
                {
                    pixel = video_data[video_data_pos++];
                    frame_buff[y+(chunk_y*chunk_size)][x+(chunk_x*chunk_size)] = pixel;
                }
            }

        } else {
            video_data_pos++;
        }
        if (chunk_count >= delay_trigger)
        {
            chunk_count = 0;
            system("clear");
            for(int y = 0; y < specs.height; y++)
            {
                for(int x = 0; x < specs.width; x++)
                {
                    pixel = frame_buff[y][x];
                    print_pixel(pixel);
                }
                printf("\n");
            }
            delay(1000 / specs.fps);
        }
    }
}

int main()
{
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
            } else if (strComp(command, "play\0"))
            {
                play();
            }

            if (token != NULL)
            {
                free(token);
            }
            break;
        }
    }

    if (video_data != NULL)
    {
        free(video_data);
    }

    return 0;
}
