#include <stdio.h>
#include "lcd.h"
#include <stdio.h>
#include "FreeRTOS.h" 
#include <string.h>
#include "task.h"

#define LCD_WIDTH 16

char stringToScroll[] = "DTEK0068 Embedded Microprocessor Systems";

void scroll(void* parameter)
{
    uint8_t offset = 0;
    uint8_t dir = 1;
    uint8_t length = strlen(stringToScroll);
    for (;;) 
    {
        char mutate_string[length+1];
        strcpy(mutate_string, stringToScroll);
        
        if(dir == 1 && offset < length-LCD_WIDTH)
        {
            offset += 1;
        } else if (dir == 0 && offset > 0 ){
            offset -= 1;
        } else if (dir == 1 && offset >= length-LCD_WIDTH){
            dir = 0;
            offset -= 1;
        } else if (dir == 0 && offset <= 0){
            dir = 1;
            offset += 1;
        }
        
        lcd_cursor_set(1,0);
        
        mutate_string[offset + LCD_WIDTH] = '\0';
        
        lcd_write(mutate_string + offset);
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL); 
}
