// ---------------------------------------------------------------------------
// Created by Francisco Malpartida on 20/08/11.
// Copyright 2011 - Under creative commons license 3.0:
//        Attribution-ShareAlike CC BY-SA
//
// This software is furnished "as is", without technical support, and with no 
// warranty, express or implied, as to its usefulness for any purpose.
//
// Thread Safe: No
// Extendable: Yes
//
// @file LCD.cpp
// This file implements a basic liquid crystal library that comes as standard
// in the Arduino SDK.
// 
// @brief 
// This is a basic implementation of the HD44780 library of the
// Arduino SDK. This library is a refactored version of the one supplied
// in the Arduino SDK in such a way that it simplifies its extension
// to support other mechanism to communicate to LCDs such as I2C, Serial, SR, ...
// The original library has been reworked in such a way that this will be
// the base class implementing all generic methods to command an LCD based
// on the Hitachi HD44780 and compatible chipsets.
//
// This base class is a pure abstract class and needs to be extended. As reference,
// it has been extended to drive 4 and 8 bit mode control, LCDs and I2C extension
// backpacks such as the I2CLCDextraIO using the PCF8574* I2C IO Expander ASIC.
//
//
// @version API 1.1.0
//
// 2012.03.29 bperrybap - changed comparision to use LCD_5x8DOTS rather than 0
// @author F. Malpartida - fmalpartida@gmail.com
// ---------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "project.h"
//#include <CyLib.h>


//extern "C" void __cxa_pure_virtual() { while (1); }
#include "LCD.h"

void delay(uint32 miliseconds)
{
    CyDelay(miliseconds);
}

void delayMicroseconds(uint16 microseconds)
{
    CyDelayUs(microseconds);
}

void _i2c_write(uint8_t data)
{
    if(I2C_MasterSendStart(_Addr, 0))
    {
        if(I2C_MasterWriteByte(data))
        {
            I2C_MasterSendStop();
        }
    }
}

void pulseEnable (uint8_t data)
{
//    if(I2C_MasterSendStart(_Addr, 0))
//    {
//        I2C_MasterWriteByte(data | _En); // En HIGH
//        CyDelayUs(5000);
//        I2C_MasterWriteByte(data & ~_En); //En LOW
//        I2C_MasterSendStop();
//    }
   _i2c_write (data | _En);   // En HIGH
    //CyDelay(100);
   _i2c_write (data & ~_En);  // En LOW
//    _i2c_write(data);
    //CyDelay(100);
//    _i2c_write(0);
}

    /*!
    @method     
    @abstract   Writes an 4 bit value to the LCD.
    @discussion Writes 4 bits (the least significant) to the LCD control data lines.
    @param      value[in] Value to write to the LCD
    @param      more[in]  Value to distinguish between command and data.
    COMMAND == command, DATA == data.
    */
void write4bits ( uint8_t value, uint8_t mode ) 
{
   uint8_t pinMapValue = 0;
   
   // Map the value to LCD pin mapping
   // --------------------------------
   for ( uint8_t i = 0; i < 4; i++ )
   {
      if ( ( value & 0x1 ) == 1 )
      {
         pinMapValue |= _data_pins[i];
      }
      value = ( value >> 1 );
   }
   
   // Is it a command or data
   // -----------------------
   if ( mode == DATA )
   {
      mode = _Rs;
   }
   
   pinMapValue |= mode | _backlightStsMask;
   pulseEnable ( pinMapValue );
}

/*!
    @function
    @abstract   Send a particular value to the LCD.
    @discussion Sends a particular value to the LCD. This is a pure abstract
    method, therefore, it is implementation dependent of each derived class how
    to physically write to the LCD.
    
    Users should never call this method.
    
    @param      value[in] Value to send to the LCD.
    @result     mode LOW - write to the LCD CGRAM, HIGH - write a command to
    the LCD.
    */
void send(uint8_t value, uint8_t mode)
{
    // No need to use the delay routines since the time taken to write takes
   // longer that what is needed both for toggling and enable pin an to execute
   // the command.
   
   if ( mode == FOUR_BITS )
   {
      write4bits( (value & 0x0F), COMMAND );
   }
   else 
   {
      write4bits( (value >> 4), mode );
      write4bits( (value & 0x0F), mode);
   }
}


// General LCD commands - generic methods used by the rest of the commands
// ---------------------------------------------------------------------------
/*!
    @function
    @abstract   Send a command to the LCD.
    @discussion This method sends a command to the LCD by setting the Register
    select line of the LCD.
    
    This command shouldn't be used to drive the LCD, only to implement any other
    feature that is not available on this library.
    
    @param      value[in] Command value to send to the LCD (COMMAND, DATA or
    FOUR_BITS).
    */
void command(uint8_t value) 
{
   send(value, COMMAND);
}



// Internal LCD variables to control the LCD shared between all derived
// classes.
uint8_t _displayfunction;  // LCD_5x10DOTS or LCD_5x8DOTS, LCD_4BITMODE or 
                          // LCD_8BITMODE, LCD_1LINE or LCD_2LINE
uint8_t _displaycontrol;   // LCD base control command LCD on/off, blink, cursor
                          // all commands are "ored" to its contents.
uint8_t _displaymode;      // Text entry mode to the LCD
uint8_t _numlines;         // Number of lines of the LCD, initialized with begin()
uint8_t _cols;             // Number of columns in the LCD
t_backlighPol _polarity;   // Backlight polarity

// PUBLIC METHODS
// ---------------------------------------------------------------------------
// When the display powers up, it is configured as follows:
// 0. LCD starts in 8 bit mode
// 1. Display clear
// 2. Function set: 
//    DL = 1; 8-bit interface data 
//    N = 0; 1-line display 
//    F = 0; 5x8 dot character font 
// 3. Display on/off control: 
//    D = 0; Display off 
//    C = 0; Cursor off 
//    B = 0; Blinking off 
// 4. Entry mode set: 
//    I/D = 1; Increment by 1 
//    S = 0; No shift 
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a application starts (and the
// LiquidCrystal constructor is called).
// A call to begin() will reinitialize the LCD.
//

void config (uint8_t lcd_Addr, uint8_t En, uint8_t Rw, uint8_t Rs, 
                                uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7 )
{
   _Addr = lcd_Addr;
   
   _backlightPinMask = 0;
   _backlightStsMask = LCD_BACKLIGHT;
   _polarity = POSITIVE;
   
   _En = ( 1 << En );
   _Rw = ( 1 << Rw );
   _Rs = ( 1 << Rs );
   
   // Initialise pin mapping
   _data_pins[0] = ( 1 << d4 );
   _data_pins[1] = ( 1 << d5 );
   _data_pins[2] = ( 1 << d6 );
   _data_pins[3] = ( 1 << d7 );   
}

void LCD_config(uint8_t lcd_Addr, uint8_t En, uint8_t Rw,
                                     uint8_t Rs, uint8_t d4, uint8_t d5,
                                     uint8_t d6, uint8_t d7, uint8_t backlighPin, 
                                     t_backlighPol pol )
{
   config(lcd_Addr, En, Rw, Rs, d4, d5, d6, d7);
   LCD_setBacklightPin(backlighPin, pol);
}

//void LCD_config (uint8_t lcd_Addr, uint8_t En, uint8_t Rw, uint8_t Rs, 
//                uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7 )
//{
//    config(lcd_Addr, En, Rw, Rs, d4, d5, d6, d7);
//}

void LCD_begin(uint8_t cols, uint8_t lines, uint8_t dotsize) 
{
   if (lines > 1) 
   {
      _displayfunction |= LCD_2LINE;
   }
   _numlines = lines;
   _cols = cols;
   
   // for some 1 line displays you can select a 10 pixel high font
   // ------------------------------------------------------------
   if ((dotsize != LCD_5x8DOTS) && (lines == 1)) 
   {
      _displayfunction |= LCD_5x10DOTS;
   }
   
   // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
   // according to datasheet, we need at least 40ms after power rises above 2.7V
   // before sending commands. Arduino can turn on way before 4.5V so we'll wait 
   // 50
   // ---------------------------------------------------------------------------
   delay (100); // 100ms delay
   
   //put the LCD into 4 bit or 8 bit mode
   // -------------------------------------
   if (! (_displayfunction & LCD_8BITMODE)) 
   {
      // this is according to the hitachi HD44780 datasheet
      // figure 24, pg 46
      
      // we start in 8bit mode, try to set 4 bit mode
      // Special case of "Function Set"
      send(0x03, FOUR_BITS);
      delayMicroseconds(4500); // wait min 4.1ms
      
      // second try
      send ( 0x03, FOUR_BITS );
      delayMicroseconds(150); // wait min 100us
      
      // third go!
      send( 0x03, FOUR_BITS );
      delayMicroseconds(150); // wait min of 100us
      
      // finally, set to 4-bit interface
      send ( 0x02, FOUR_BITS );
      delayMicroseconds(150); // wait min of 100us

   } 
   else 
   {
      // this is according to the hitachi HD44780 datasheet
      // page 45 figure 23
      
      // Send function set command sequence
      command(LCD_FUNCTIONSET | _displayfunction);
      delayMicroseconds(4500);  // wait more than 4.1ms
      
      // second try
      command(LCD_FUNCTIONSET | _displayfunction);
      delayMicroseconds(150);
      
      // third go
      command(LCD_FUNCTIONSET | _displayfunction);
      delayMicroseconds(150);

   }
   
   // finally, set # lines, font size, etc.
   command(LCD_FUNCTIONSET | _displayfunction);
   delayMicroseconds ( 60 );  // wait more
   
   // turn the display on with no cursor or blinking default
   _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
   LCD_display();
   
   // clear the LCD
   LCD_clear();
   
   // Initialize to default text direction (for romance languages)
   _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
   // set the entry mode
   command(LCD_ENTRYMODESET | _displaymode);

   LCD_backlight();

}

// Common LCD Commands
// ---------------------------------------------------------------------------
void LCD_clear()
{
   command(LCD_CLEARDISPLAY);             // clear display, set cursor position to zero
   delayMicroseconds(HOME_CLEAR_EXEC);    // this command is time consuming
}

void LCD_home()
{
   command(LCD_RETURNHOME);             // set cursor position to zero
   delayMicroseconds(HOME_CLEAR_EXEC);  // This command is time consuming
}

void LCD_setCursor(uint8_t col, uint8_t row)
{
   const uint8_t row_offsetsDef[]   = { 0x00, 0x40, 0x14, 0x54 }; // For regular LCDs
   const uint8_t row_offsetsLarge[] = { 0x00, 0x40, 0x10, 0x50 }; // For 16x4 LCDs
   
   if ( row >= _numlines ) 
   {
      row = _numlines-1;    // rows start at 0
   }
   
   // 16x4 LCDs have special memory map layout
   // ----------------------------------------
   if ( _cols == 16 && _numlines == 4 )
   {
      command(LCD_SETDDRAMADDR | (col + row_offsetsLarge[row]));
   }
   else 
   {
      command(LCD_SETDDRAMADDR | (col + row_offsetsDef[row]));
   }
   
}

// Turn the display on/off
void LCD_noDisplay() 
{
   _displaycontrol &= ~LCD_DISPLAYON;
   command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LCD_display() 
{
   _displaycontrol |= LCD_DISPLAYON;
   command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void LCD_noCursor() 
{
   _displaycontrol &= ~LCD_CURSORON;
   command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LCD_cursor() 
{
   _displaycontrol |= LCD_CURSORON;
   command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns on/off the blinking cursor
void LCD_noBlink() 
{
   _displaycontrol &= ~LCD_BLINKON;
   command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LCD_blink() 
{
   _displaycontrol |= LCD_BLINKON;
   command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void LCD_scrollDisplayLeft(void) 
{
   command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

void LCD_scrollDisplayRight(void) 
{
   command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void LCD_leftToRight(void) 
{
   _displaymode |= LCD_ENTRYLEFT;
   command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void LCD_rightToLeft(void) 
{
   _displaymode &= ~LCD_ENTRYLEFT;
   command(LCD_ENTRYMODESET | _displaymode);
}

// This method moves the cursor one space to the right
void LCD_moveCursorRight(void)
{
   command(LCD_CURSORSHIFT | LCD_CURSORMOVE | LCD_MOVERIGHT);
}

// This method moves the cursor one space to the left
void LCD_moveCursorLeft(void)
{
   command(LCD_CURSORSHIFT | LCD_CURSORMOVE | LCD_MOVELEFT);
}


// This will 'right justify' text from the cursor
void LCD_autoscroll(void) 
{
   _displaymode |= LCD_ENTRYSHIFTINCREMENT;
   command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void LCD_noAutoscroll(void) 
{
   _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
   command(LCD_ENTRYMODESET | _displaymode);
}

// Write to CGRAM of new characters
void LCD_createChar(uint8_t location, uint8_t charmap[]) 
{
   location &= 0x7;            // we only have 8 locations 0-7
   
   command(LCD_SETCGRAMADDR | (location << 3));
   delayMicroseconds(30);
   
   for (uint8_t i = 0; i < 8; i++)
   {
      LCD_write(charmap[i]);      // call the virtual write method
      delayMicroseconds(40);
   }
}

#ifdef __AVR__
void LCD_createChar(uint8_t location, const char *charmap)
{
   location &= 0x7;   // we only have 8 memory locations 0-7
   
   command(LCD_SETCGRAMADDR | (location << 3));
   delayMicroseconds(30);
   
   for (uint8_t i = 0; i < 8; i++)
   {
      write(pgm_read_byte_near(charmap++));
      delayMicroseconds(40);
   }
}
#endif // __AVR__
void LCD_setBacklightPin ( uint8_t value, t_backlighPol pol )
{
   _backlightPinMask = ( 1 << value );
   _polarity = pol;
   LCD_setBacklight(BACKLIGHT_OFF);
}

void LCD_setBacklight( uint8_t value ) 
{
   // Check if backlight is available
   // ----------------------------------------------------
   if ( _backlightPinMask != 0x0 )
   {
      // Check for polarity to configure mask accordingly
      // ----------------------------------------------------------
      if  (((_polarity == POSITIVE) && (value > 0)) || 
           ((_polarity == NEGATIVE ) && ( value == 0 )))
      {
         _backlightStsMask = _backlightPinMask & LCD_BACKLIGHT;
      }
      else 
      {
         _backlightStsMask = _backlightPinMask & LCD_NOBACKLIGHT;
      }
      _i2c_write( _backlightStsMask );
   }
}

//
// Switch on the backlight
void LCD_backlight ( void )
{
   LCD_setBacklight(255);
}

//
// Switch off the backlight
void LCD_noBacklight ( void )
{
   LCD_setBacklight(0);
}

//
// Switch fully on the LCD (backlight and LCD)
void LCD_on ( void )
{
   LCD_display();
   LCD_backlight();
}

//
// Switch fully off the LCD (backlight and LCD) 
void LCD_off ( void )
{
   LCD_noBacklight();
   LCD_noDisplay();
}

void LCD_write(uint8_t value)
{
   send(value, DATA);
}

void LCD_writeString(char* string)
{
    if(string != NULL)
    {
        char *c = string;
        while(*c)
        {
            LCD_write(*c++);
            //CyDelay(500);
        }
    }
}





