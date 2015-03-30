/*************************************************
//
// Copyright (C) 2015, Y. Nomura, all right reserved.
// This software is released under the MIT License, see LICENSE.md.
//
*************************************************/
# bitbanging
i2c bit banging (sample of raspberry pi )

This program requires the following function

-void wait_usec(uint32_t time);
	Provide the function of wait micro sec.

-void GPIOInit();
	write the function of pin initialization.

-void GPIOSetDir(uint32_t pin, uint32_t mode);
	Set a pin to assigned mode (0:input ,1 output).

-void GPIOSetValue(uint32_t pin, uint32_t value );
	Set a pin to assigned level (0: low, 1: high).

- int32_t GPIOGetState(uint32_t pin){
	Get a pin level in any case.
	return 0 means low level and return 1 means high level.


This program provides the following type and functions

-I2CResult : 
new type used as the returning value

-void Initi2c();
initialize i2c pin.

-void BusClear();
Send 9 clock and Stop condition.
 
-I2CResult Writei2c(uint8_t d_addr, uint8_t r_addr,const uint8_t *buffer, uint32_t length);
	Write multi byte data,
	where d_addr means device address (8 bits), 
	r_addr means first register address to write data, 
	*buffer means pointer to variable of transmitted data,
	length means the number of written byte data,

-I2CResult Writei2c(uint8_t d_addr, uint8_t r_addr,uint8_t buffer);
	This function is equal to write “Writei2c(d_addr, r_addr, &buffer,1);”

-I2CResult Readi2c(uint8_t d_addr, uint8_t r_addr, uint8_t *buffer, uint32_t length);
	Read multi byte data,
	where d_addr means device address (8 bits), 
	r_addr means first register address to read data, 
	*buffer means pointer to variable assigned received data,
	length means the number of written byte data,

-I2CResult Readi2c(uint8_t d_addr, uint8_t *buffer, uint32_t length);
	Read multi byte data without sending register address.

-I2CResult GetI2CState();
	Get I2CState of last action.

