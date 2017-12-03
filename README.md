# psoc5_i2C_LCD
Arduino newcrystallib i2c lcd library,  very simplified and converted from c++ to c, for use with cypress psoc5lp .

This is very simplistic conversion of I2C part of NewLiquidCrystalLib library for Arduino, to be used with Cypress psoc5lp.
LiquidCrystal.cpp && LiquidCrystal_I2C.cpp have been just converted to C.

For working it needs I2C master component, which's name has to be set in LCD.h macro I2C_MasterCommand(command) I2C_Master_ ## command,
where I2CMaster_ has to be replaced with I2C master component name.

Original Arduino lib lives here: https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home .
