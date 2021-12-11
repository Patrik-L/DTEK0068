#include <stdio.h>
#include "lcd.h"
#include <stdio.h>
#include "FreeRTOS.h" 
#include <string.h>
#include "task.h"
#


char display_text[sizeof(char) * 16];


char *clearRow = "                ";

char *stringToScroll = "DTEK0068";

void scroll(void* parameter)
{
    uint8_t offset = 0;
    uint8_t dir = 1;
    uint8_t length = strlen(stringToScroll);
    for (;;) 
    {
        if(dir == 1 && offset < length)
        {
            offset += 1;
        } else if (dir == 0 && offset > 0 ){
            offset -= 1;
        } else if (dir == 1 && offset >= length){
            dir = 0;
            offset -= 1;
        } else if (dir == 0 && offset <= 0){
            dir = 1;
            offset += 1;
        }
        
        
        
        lcd_cursor_set(1,0);
        lcd_write(clearRow);
        
        lcd_cursor_set(1,offset);
        
        
        
        lcd_write(stringToScroll);
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL); 
}
