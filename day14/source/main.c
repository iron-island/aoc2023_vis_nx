#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <switch.h>
#include <math.h>

#define LINE_LEN 102
#define V_TILT_THRESH 0.25
#define H_TILT_THRESH 0.20
#define DELAY_MS 10

int calculate_load(char** grid, int num_rows, int num_cols){
    int r;
    int c;
    int row_load;
    int load;

    load = 0;
    for (r = 0; r < num_rows; r++){
        row_load = 0;
        for (c = 0; c < num_cols; c++){
            if (grid[r][c] == 'O'){
                row_load = row_load + 1;
            }
        }
        load = load + row_load*(num_rows-r);
    }

    return load;
}

void print_grid(char** grid, int num_rows, int num_cols){
    int r;
    int c;

    for (r = 0; r < num_rows; r++){
        printf("    ");
        // TODO: add more efficient printing of whole rows, per character printing
        //       for now to prevent the null terminator from being printed
        for (c = 0; c < num_cols; c++){
            printf("%c", grid[r][c]);
        }
        printf("  \n");
    } 
};

void init_grid(char** grid, int num_rows, int num_cols){
    FILE *fptr;
    int r;
    int c;

    char row[LINE_LEN];

    fptr = fopen("./input_dec14.txt", "r");

    r = 0;
    while (fgets(row, LINE_LEN, fptr)){
        if (r >= num_rows){
            break;
        }

        for (c = 0; c < num_cols; c++){
            grid[r][c] = row[c];
        }
        r = r + 1;
    }

    fclose(fptr);
}

int main(int argc, char **argv){
    // Grid variables
    const int num_rows = 35;
    const int num_cols = 71; // switch width is 79 characters
    char** grid;
    int r;
    int c;
    char curr_char;
    char prev_char;    

    // Tilt variables
    float horizontal_tilt;
    float vertical_tilt;
    float abs_h_tilt;
    float abs_v_tilt;
    int tilt_config;
    int load;

    // Tick variables, based on:
    //   https://github.com/switchbrew/switch-examples/tree/master/graphics/opengl/es2gears/source/main.c
    double t;
    static u64 origTicks = UINT64_MAX;

    tilt_config = 4;

    // 6-axis accelerometer reading based on:
    //   https://github.com/switchbrew/switch-examples/tree/master/hid/sixaxis/source/main.c

    consoleInit(NULL);

    // Configure our supported input layout: a single player with standard controller styles
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    // Initialize the default gamepad (which reads handheld mode inputs as well as the first connected controller)
    PadState pad;
    padInitializeDefault(&pad);

    // It's necessary to initialize these separately as they all have different handle values
    HidSixAxisSensorHandle handles[4];
    hidGetSixAxisSensorHandles(&handles[0], 1, HidNpadIdType_Handheld, HidNpadStyleTag_NpadHandheld);
    hidGetSixAxisSensorHandles(&handles[1], 1, HidNpadIdType_No1,      HidNpadStyleTag_NpadFullKey);
    hidGetSixAxisSensorHandles(&handles[2], 2, HidNpadIdType_No1,      HidNpadStyleTag_NpadJoyDual);
    hidStartSixAxisSensor(handles[0]);
    hidStartSixAxisSensor(handles[1]);
    hidStartSixAxisSensor(handles[2]);
    hidStartSixAxisSensor(handles[3]);

    printf("\x1b[1;1HAdvent of Code 2023 Day 14: Parabolic Reflector Dish");

    // Initialize grid
    grid = malloc(num_rows*sizeof(char*));

    for (r = 0; r < num_rows; r++){
        grid[r] = malloc(num_cols*sizeof(char*));
    }
    init_grid(grid, num_rows, num_cols);
    load = calculate_load(grid, num_rows, num_cols);

    // Main loop
    while(appletMainLoop())
    {
        // Scan the gamepad. This should be done once for each frame
        padUpdate(&pad);

        // padGetButtonsDown returns the set of buttons that have been
        // newly pressed in this frame compared to the previous one
        u64 kDown = padGetButtonsDown(&pad);

        if (kDown & HidNpadButton_Plus) break; // break in order to return to hbmenu

        // Read from the correct sixaxis handle depending on the current input style
        HidSixAxisSensorState sixaxis = {0};
        u64 style_set = padGetStyleSet(&pad);
        if (style_set & HidNpadStyleTag_NpadHandheld)
            hidGetSixAxisSensorStates(handles[0], &sixaxis, 1);
        else if (style_set & HidNpadStyleTag_NpadFullKey)
            hidGetSixAxisSensorStates(handles[1], &sixaxis, 1);
        else if (style_set & HidNpadStyleTag_NpadJoyDual) {
            // For JoyDual, read from either the Left or Right Joy-Con depending on which is/are connected
            u64 attrib = padGetAttributes(&pad);
            if (attrib & HidNpadAttribute_IsLeftConnected)
                hidGetSixAxisSensorStates(handles[2], &sixaxis, 1);
            else if (attrib & HidNpadAttribute_IsRightConnected)
                hidGetSixAxisSensorStates(handles[3], &sixaxis, 1);
        }

        // Calculate ticks to delay grid calculations
        if (origTicks == UINT64_MAX)
           origTicks = armGetSystemTick();

        u64 ticksElapsed = armGetSystemTick() - origTicks;
        //t = (ticksElapsed * 625 / 12) / 1000000000.0; // t in seconds?
        t = (ticksElapsed * 625 / 12) / 1000000.0; // t in milliseconds?

        if (t > DELAY_MS){
            origTicks = UINT64_MAX;

            printf("\x1b[3;1H");

            horizontal_tilt = sixaxis.direction.direction[2][0];
            vertical_tilt   = sixaxis.direction.direction[1][2];

            // For debugging
            //printf("Horizontal tilt: %.4f\n", horizontal_tilt);
            //printf("Vertical tilt  : %.4f\n", vertical_tilt);

            // Tilt configuration:
            //   0: North
            //   1: West
            //   2: South
            //   3: East
            //   4: PAUSED
            
            // Since the accelerometer is sensitive, only change the tilt configuration
            //   when the readings exceed a certain threshold
            abs_h_tilt = fabs(horizontal_tilt);
            abs_v_tilt = fabs(vertical_tilt);
            if ((abs_v_tilt > V_TILT_THRESH) || (abs_h_tilt > H_TILT_THRESH)){
                // Gives priority to vertical tilt due to if statement
                // TODO: give priority to direction that has larger tilt
                if (abs_v_tilt > abs_h_tilt){
                    if (vertical_tilt > 0.0){
                        tilt_config = 2;
                    } else{
                        tilt_config = 0;
                    }
                } else{
                    if (horizontal_tilt > 0.0) {
                        tilt_config = 3;
                    } else{
                        tilt_config = 1;
                    }
                }
            }

            switch(tilt_config){
                case 0:  printf("Tilt configuration: North \n"); break;
                case 1:  printf("Tilt configuration: West  \n"); break;
                case 2:  printf("Tilt configuration: South \n"); break;
                case 3:  printf("Tilt configuration: East  \n"); break;
                default: printf("Tilt configuration: PAUSED\n");
            }

            // Tilt

            prev_char = '#';
            if (tilt_config == 0){
                // tilt north
                for (c = 0; c < num_cols; c++){
                    for (r = 0; r < num_rows; r++){
                        curr_char = grid[r][c];
                        if ((r > 0) && (curr_char == 'O') && (prev_char == '.')){
                            grid[r-1][c] = 'O';
                            grid[r][c] = '.';
                        }
                        prev_char = curr_char;
                    }
                }
            } else if (tilt_config == 2){
                // tilt south
                for (c = 0; c < num_cols; c++){
                    for (r = (num_rows-1); r >= 0; r--){
                        curr_char = grid[r][c];
                        if ((r < (num_rows-1)) && (curr_char == 'O') && (prev_char == '.')){
                            grid[r+1][c] = 'O';
                            grid[r][c] = '.';
                        }
                        prev_char = curr_char;
                    }
                }
            } else if (tilt_config == 1){
                // tilt west
                for (r = 0; r < num_rows; r++){
                    for (c = 0; c < num_cols; c++){
                        curr_char = grid[r][c];
                        if ((c > 0) && (curr_char == 'O') && (prev_char == '.')){
                            grid[r][c-1] = 'O';
                            grid[r][c] = '.';
                        }
                        prev_char = curr_char;
                    }
                }
            } else if (tilt_config == 3){
                // tilt east
                for (r = 0; r < num_rows; r++){
                    for (c = (num_cols-1); c >= 0; c--){
                        curr_char = grid[r][c];
                        if ((c < (num_cols-1)) && (curr_char == 'O') && (prev_char == '.')){
                            grid[r][c+1] = 'O';
                            grid[r][c] = '.';
                        }
                        prev_char = curr_char;
                    }
                }
            }
            // Calculate load, if tilt is to the west or east, load won't change
            // TODO: this can be optimized by calculating load while moving the rocks
            if ((tilt_config == 0) || (tilt_config == 2)){
                load = calculate_load(grid, num_rows, num_cols);
            }

            printf("Load on north support beam: %5d\n", load);
            printf("\x1b[8;1H");
            print_grid(grid, num_rows, num_cols);
        }

        consoleUpdate(NULL);
    }
    free(grid);

    hidStopSixAxisSensor(handles[0]);
    hidStopSixAxisSensor(handles[1]);
    hidStopSixAxisSensor(handles[2]);
    hidStopSixAxisSensor(handles[3]);

    consoleExit(NULL);

    return 0;
}
