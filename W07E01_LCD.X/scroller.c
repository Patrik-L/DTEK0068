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
        // Adding 1 to length to accommodate null terminator.
        char mutate_string[length+1];
        // We don't wan't to mangle our original string.
        strcpy(mutate_string, stringToScroll);
        
        // Moving an the offset index back and forth
        // between 0 and length-lcd_width.
        if(dir == 1 && offset < length-LCD_WIDTH)
        {
            offset += 1;
        } else if (dir == 0 && offset > 0 )
        {
            offset -= 1;
        } else if (dir == 1 && offset >= length-LCD_WIDTH)
        {
            dir = 0;
            offset -= 1;
        } else if (dir == 0 && offset <= 0)
        {
            dir = 1;
            offset += 1;
        }
        
        // We want to write at the bottom line
        lcd_cursor_set(1,0);
        
        // Inject a null terminator at lcd_width so that we don't write
        // too much to the lcd.
        mutate_string[offset + LCD_WIDTH] = '\0';
        
        lcd_write(mutate_string + offset);
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL); 
}
