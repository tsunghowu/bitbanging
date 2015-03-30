/*************************************************
//
// i2c.h
// Copyright (C) 2015, Y. Nomura, all right reserved.
// This software is released under the MIT License, see LICENSE.md.
//
*************************************************/

#ifndef __I2C_H_
#define __I2C_H_

//#include <cstdint>
#include <stdint.h>

namespace i2c{

	typedef enum{
		I2C_OK=0,
		SLAW_NACK,
		SLAR_NACK,
		T_DATA_NACK,
		I2C_NO_INIT,
		I2C_BUSY,
		I2C_TIME_OUT,
		I2C_BUS_ERROR
	}I2CResult;

	void Initi2c();
	
	void BusClear();

	I2CResult Writei2c(uint8_t d_addr, uint8_t r_addr,const uint8_t *buffer, uint32_t length);
	inline I2CResult Writei2c(uint8_t d_addr, uint8_t r_addr,uint8_t buffer){
		return Writei2c(d_addr,r_addr,&buffer,1);
	}

	I2CResult Readi2c(uint8_t d_addr, uint8_t r_addr, uint8_t *buffer, uint32_t length);
	I2CResult Readi2c(uint8_t d_addr, uint8_t *buffer, uint32_t length);

	I2CResult GetI2CState();
}

#endif
