#include "LCD_1in44.h"
#include "GUI_Paint.h"
#include "DEV_Config.h"
#include "Debug.h"
#include <cmath>
#include <stdlib.h>
#include <string>
#include "Infrared.h"

#define key0 15
#define key1 17
#define key2 2
#define key3 3

// api reference gathered from digging about
// DEV_Module_Init() not 100% sure what this does lmao
//      return code != 0 is bad
// DEV_Module_Exit()
//
// DEV_Delay_ms()
//      sleepy times
// LCD_1IN44_Init(HORIZONTAL/VERTICAL)
//      init screen in horiz or vert display ( i think this just changes the coord system ) 
// LCD_1IN44_Clear(COLOR)
//      clear screen with provided color
// Paint_DrawString_EN(x, y, string, &Font[1-20], FONTCOLOR, BGCOLOR);
//
// Input stuff
/*
    int key0 = 15; 
    int key1 = 17; 
    int key2 = 2; 
    int key3 = 3; 
    
    SET_Infrared_PIN(key0);    
    SET_Infrared_PIN(key1);
    SET_Infrared_PIN(key2);
    SET_Infrared_PIN(key3);

    DEV_Digital_Read(key#)
        returns true or false if currently held... i think

*/
// Drawing shit
// Paint_DrawRectangle(88, 98, 123, 128, YELLOW, DOT_PIXEL_1X1,DRAW_FILL_FULL)
//      (topleftx, toplefty, bottomrightx, bottomrighty, COL, PIXEL_SIZE???, FILLSTYLE?);
// Paint_DrawCircle(95, 25, 15, GREEN, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
//      (x, y, r, COL, PIXEL_SIZE?, FILLSTYLE)
// Paint_DrawPoint(122,5, BLACK, DOT_PIXEL_5X5, DOT_FILL_RIGHTUP);
//      (x, y, COL, PIXEL_SIZE?, FILLSTYLE)
// Paint_DrawLine( 10,  10, 40, 40, MAGENTA, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
//      (x1, y1, x2, y2, COL, PIXEL_SIZE, LINESTYLE)

// I think this sets up the image.. cache..? im not sure but looking thru everything has 
//      LCD_1IN44_Display(BlackImage);
// after all draw calls so maybe everything is drawn onto BlackImage for some reason then
// doing this displays it
/*
    UDOUBLE Imagesize = LCD_1IN44_HEIGHT*LCD_1IN44_WIDTH*2;
    UWORD *BlackImage;
    if((BlackImage = (UWORD *)malloc(Imagesize)) == NULL) {
        printf("Failed to apply for black memory...\r\n");
        exit(0);
    }
    // 1.Create a new image cache named IMAGE_RGB and fill it with white
    Paint_NewImage((UBYTE *)BlackImage,LCD_1IN44.WIDTH,LCD_1IN44.HEIGHT, 0, WHITE);
    Paint_SetScale(65);
    Paint_Clear(WHITE);
    Paint_SetRotate(ROTATE_0);
    Paint_Clear(WHITE);

    dont forget to free(BlackImage)
*/

bool isKeyDown(int keycode){
        return DEV_Digital_Read(keycode) == 0;
}

int main(int argc, char** argv){
        // initialize

        stdio_init_all();

        if(DEV_Module_Init()!=0){
                return -1;
        }

        LCD_1IN44_Init(HORIZONTAL);
        LCD_1IN44_Clear(WHITE);

        // setup screen buffer..?
        UDOUBLE Imagesize = LCD_1IN44_HEIGHT*LCD_1IN44_WIDTH*2;
        UWORD *BlackImage;
        if((BlackImage = (UWORD *)malloc(Imagesize)) == NULL) {
                printf("Failed to apply for black memory...\r\n");
                exit(0);
        }

        Paint_NewImage((UBYTE *)BlackImage,LCD_1IN44.WIDTH,LCD_1IN44.HEIGHT, 0, WHITE);
        Paint_SetScale(65);
        Paint_Clear(WHITE);
        Paint_SetRotate(ROTATE_0);
        Paint_Clear(WHITE);
        //

        // init input shit

        SET_Infrared_PIN(key0);    
        SET_Infrared_PIN(key1);
        SET_Infrared_PIN(key2);
        SET_Infrared_PIN(key3);
        //
        //

        absolute_time_t time_at_prev_frame {get_absolute_time()};
        // main loop
        while(true){
                // calculate dt
                absolute_time_t time_at_curr_frame {get_absolute_time()};
                int64_t time_diff_us {absolute_time_diff_us(time_at_prev_frame, time_at_curr_frame)};
                float dt {static_cast<float>(time_diff_us) / 1000000}; // dt in terms of seconds cause thats how my brain likes it
                int fps {static_cast<int>(std::floor(1/dt))};
                time_at_prev_frame = time_at_curr_frame;
                //

                // clear screen
                // this drops fps by like 40% -> Paint_DrawRectangle(0, 0, LCD_1IN44.WIDTH, LCD_1IN44.HEIGHT, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
                Paint_Clear(WHITE);

                // draw fps in top right
                Paint_DrawString_EN(90, 0, ("fps: "+std::to_string(fps)).c_str(), &Font8, BLACK, WHITE);

                if(isKeyDown(key0))
                        Paint_DrawRectangle(16, 16, 32, 32, BLUE, DOT_PIXEL_1X1, DRAW_FILL_FULL);

                // display buffered screen
                LCD_1IN44_Display(BlackImage);
        }
        
        return 0;
}
