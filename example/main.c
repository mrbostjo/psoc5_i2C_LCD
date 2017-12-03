/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include "LCD.h"

void i2c_send(uint8_t data)
{
    if(I2C_Master_MasterSendStart(0x27, 0))
    {
        if(I2C_Master_MasterWriteByte(data))
        {
            I2C_Master_MasterSendStop();
        }
    }
}

void testPulse(uint8_t data)
{
    i2c_send(data);
    CyDelay(250);
    i2c_send(0);
    CyDelay(100);
}

int main(void)
{
    CyDelay(100);
    CyGlobalIntEnable; /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    I2C_Master_Start();
    //         addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
    LCD_config(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
    //LCD_config(0x27, 2, 1, 0, 4, 5, 6, 7);
    LCD_backlight();
    LCD_begin(20, 4, LCD_5x8DOTS);
    LCD_clear();
    
    LCD_writeString("A beginning of test");
    CyDelay(2000);
    int d = 1;
    for(;;)
    {
        /* Place your application code here. */
        LCD_writeString("A long test!!!\0");
        CyDelay(500);
        //pulseEnable(2);
        LCD_clear();
        LCD_setCursor(0, d++%4);
        //LCD_home();
        CyDelay(500);
    }
}



/* [] END OF FILE */
