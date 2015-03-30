/**********************************************
//	i2c.cpp
//
//	last update 2015/03/29
//	Copyright (C) 2015, Y. Nomrua, all right reserved.
//	This software is released under the MIT License, see LICENSE.md.
//
//	(Platform Dependent) 
//	
************************************************/


#include "i2c.h"
//platform dependent

namespace i2c{
	
//********************
// this module needs the following function (platform dependent)
// (shuld be define in other header file.)
// following functions are sample (maybe not work).
//
// GPIOInit()
// GPIOSetDir(uint32_t pin, uint32_t mode)
// GPIOSetValue(uint32_t pin, uint32_t mode)  //when output
// int32_t GPIOGetState(uint32_t pin) //when input
// wait_usec(uint32_t time) //wait 
// 
//
//********************
	
	
	void GPIOInit(){
		gpio_init();
	}
	void GPIOSetDir(uint32_t pin, uint32_t mode){
		gpio_configure(pin,mode);
	} 
	void GPIOSetValue(uint32_t pin, uint32_t value ){
		if(value==0){
			gpio_clear(pin);
		}
		else if(value==1){
			gpio_set(pin);
		}
	}

	int32_t GPIOGetState(uint32_t pin){
		return gpio_read(pin);
	}

	void wait_usec(uint32_t time){
		usleep(time);
	}


//********************
//
// local variable
// scope is only this file
//
//********************

	namespace{//scope is only this file
		const uint32_t gpio_sda = 2;
		const uint32_t gpio_scl= 3;

		const uint32_t gpio_low = 0;
		const uint32_t gpio_high = 1;

		const uint32_t gpio_input = 0x0;
		const uint32_t gpio_output =0x1;

		const uint32_t i2c_time = 2;
		const uint8_t i2cwirte =0x00;
		const uint8_t i2cread = 0x01;
	}



//********************
//
// local function (typically platform independent)
// scope is only this file
//
//********************
	namespace{//local funtion

		static I2CResult i2c_state = I2C_NO_INIT;

		/*************************
		Send Byte and receive ACK/NACK
		**************************/
		uint32_t I2CSendByte(const uint8_t data){
			int pinstate=0;

			for(int i=0;i<8;++i){
				//SCL LOW change data
				GPIOSetDir(gpio_scl, gpio_output);
				GPIOSetValue(gpio_scl, gpio_low);
		
				if(data&(0x01<<(7-i))){//next data is high
					if(!GPIOGetState(gpio_sda)){//pre state high
						GPIOSetDir(gpio_sda, gpio_input);
					}
					pinstate=1;
				}
				else{
					if(GPIOGetState(gpio_sda)){//pre state high
						GPIOSetDir(gpio_sda, gpio_output);
						GPIOSetValue(gpio_sda,gpio_low);
					}
					pinstate=0;
				}
				wait_usec(i2c_time);

				//SCL H transmit data
				GPIOSetDir(gpio_scl, gpio_input);
				while(!GPIOGetState(gpio_scl)){}//wait for  Clock stretching
		
				if(pinstate!=GPIOGetState(gpio_sda)){//some thing error
					return 2;
				}
				wait_usec(i2c_time);
			}

			//check ACK/NACK
			//SCL LOW
			GPIOSetDir(gpio_scl, gpio_output);
			GPIOSetValue(gpio_scl, gpio_low);

			GPIOSetDir(gpio_sda, gpio_input);

			wait_usec(i2c_time);

			//SCL HIGH
			GPIOSetDir(gpio_scl, gpio_input);
			while(!GPIOGetState(gpio_scl)){}//wait for  Clock stretching
	
			if(!GPIOGetState(gpio_sda)){//ACK
				wait_usec(i2c_time);
				return 0;
			}
			else{//NACK
				wait_usec(i2c_time);
				return 1;
			}
		}

		/*************************
		Send start condition
		**************************/

		uint32_t I2CSendStart(){
			uint32_t i=0;

			GPIOSetDir(gpio_sda, gpio_input);
			GPIOSetDir(gpio_scl, gpio_input);
			wait_usec(1);
	
			while(i<50){// if SCL or SDA is LOW  which means I2C busy
				if(!GPIOGetState(gpio_sda)){return 0x01;}
				if(!GPIOGetState(gpio_scl)){return 0x01;}
				wait_usec(1);
				i++;
			}
	
			GPIOSetDir(gpio_sda, gpio_output);
			GPIOSetValue(gpio_sda,gpio_low);
			wait_usec(i2c_time);

			return 0x08;
		}

		/*************************
		Send restart condition
		**************************/
		
		uint32_t I2CSendReStart(){
	
			//SCL LOW send start condition
			GPIOSetDir(gpio_scl, gpio_output);
			GPIOSetValue(gpio_scl, gpio_low);

			GPIOSetDir(gpio_sda, gpio_input);
			wait_usec(i2c_time);
	
			//set start condition
			GPIOSetDir(gpio_scl, gpio_input);
	
			while(!GPIOGetState(gpio_scl)){}//wait for  Clock stretching

			GPIOSetDir(gpio_sda, gpio_output);
			GPIOSetValue(gpio_sda,gpio_low);
			wait_usec(i2c_time);

			//0x10: restart condition has been transmitted.
			return 0x10;
		}

		
		/*************************
		Send SLA+R/W
		**************************/
		
		uint32_t I2CSendSLA(uint8_t d_addr){

			uint32_t buscheck=I2CSendByte(d_addr);
			if(!buscheck){//return 0 OK
				if(d_addr&i2cread){//SLA+R OK
				return 0x40;
				}else{//SLA+W OK
				return 0x18;
				}
			}
			else if(buscheck==1){// return 1 receive NACK
				if(d_addr&i2cread){//SLA+R NACK
					return 0x48;
				}else{//SLA+W NACK
					return 0x20;
				}
			}
			else if(buscheck==2){//Arbitation
				return 0x00;
			}
		}

		/*************************
		Send Byte Data 
		**************************/
		uint32_t I2CSendData(const uint8_t *buffer, uint32_t length){

			for(int i=0;i<length;++i){
				uint32_t buscheck=I2CSendByte(*buffer++);
				if(buscheck==1){//NACK
					return 0x30;
				}
				else if(buscheck==2){//something happend ?
					return 0x00;
				}
			}
			return 0x28;
		}

		/*************************
		Receive Byte Data 
		**************************/
		uint32_t I2CReceiveData( uint8_t *buffer, uint32_t length){
			uint32_t state;
	
			for(int i=0;i<length;++i){
				*buffer=0x00;

				GPIOSetDir(gpio_scl, gpio_output);
				GPIOSetValue(gpio_scl,gpio_low);
				GPIOSetDir(gpio_sda, gpio_input);
				wait_usec(i2c_time);
		
				for(int j=0;j<8;++j){
					wait_usec(i2c_time);
			
					//SCL HIGH
					GPIOSetDir(gpio_scl, gpio_input);
					while(!GPIOGetState(gpio_scl)){}//wait for  Clock stretching
			
					*buffer|=GPIOGetState(gpio_sda)<<(7-j);
					wait_usec(i2c_time);
			
					//SCL LOW
					GPIOSetDir(gpio_scl, gpio_output);
					GPIOSetValue(gpio_scl,gpio_low);
				}
				++buffer;
		
				if(i==length-1){//final NACK return
					wait_usec(i2c_time);

					//SCL HIGH
					GPIOSetDir(gpio_scl, gpio_input);
					while(!GPIOGetState(gpio_scl)){}//wait for  Clock stretching
					wait_usec(i2c_time);
					break;
				}
				else{//ACK return
					GPIOSetDir(gpio_sda, gpio_output);
					GPIOSetValue(gpio_sda,gpio_low);
					wait_usec(i2c_time);
			
					//SCL HIGH
					GPIOSetDir(gpio_scl, gpio_input);
					while(!GPIOGetState(gpio_scl)){}//wait for  Clock stretching
					wait_usec(i2c_time);
				}
			}
			return 0x58;
		}

		/*************************
		Send stop condition
		**************************/

		void I2CSendStop(){
	
			//SCL LOW
			GPIOSetDir(gpio_scl, gpio_output);
			GPIOSetValue(gpio_scl,gpio_low);
			GPIOSetDir(gpio_sda, gpio_output);
			GPIOSetValue(gpio_sda,gpio_low);

			wait_usec(i2c_time);
	
			GPIOSetDir(gpio_scl, gpio_input);
			while(!GPIOGetState(gpio_scl)){}//wait for  Clock stretching
			wait_usec(1);
	
			GPIOSetDir(gpio_sda, gpio_input);
		}

		/*************************
		Check and Return I2C state 
		**************************/
		I2CResult SendError(uint32_t state){
			if(state==0x00){
				i2c_state=I2C_BUS_ERROR;
			}else if(state==0x20){
				i2c_state=SLAW_NACK;
			}else if(state==0x30){
				i2c_state=T_DATA_NACK;
			}else if(state==0x48){
				i2c_state=SLAR_NACK;
			}else if(state=0x01){//my local difinition
				i2c_state=I2C_BUSY;
			}else{
				i2c_state=I2C_BUS_ERROR;
			}
			I2CSendStop();
			return i2c_state;
		}
	}//end of namespace{}

	/****************************
	public function
	*****************************/
	/****************************
	Initialize i2c
	*****************************/

	void Initi2c(){

		if(i2c_state!=I2C_NO_INIT){
			return;
		}
		gpio_init();
		GPIOSetDir(gpio_scl, gpio_input);
		GPIOSetDir(gpio_sda, gpio_input);
		
		i2c_state=I2C_OK;
	}
	
	/****************************
	i2c bus clear
	*****************************/
	void BusClear(){
		GPIOSetDir(gpio_sda, gpio_input);

		for(int i=0;i<9;++i){
			GPIOSetDir(gpio_scl, gpio_output);
			GPIOSetValue(gpio_scl, gpio_low);
			wait_usec(i2c_time);

			//SCL HIGH
			GPIOSetDir(gpio_scl, gpio_input);
			while(!GPIOGetState(gpio_scl)){}//wait for  Clock stretching
			wait_usec(i2c_time);
			}
		
		I2CSendStop();
	
	}
	
	/****************************
	i2c master write
	*****************************/
	
	I2CResult Writei2c( uint8_t d_addr, uint8_t r_addr,const uint8_t *buffer, uint32_t length){
	
		uint32_t state;
		if(i2c_state==I2C_NO_INIT){
			Initi2c();
		}
	
		state=I2CSendStart();
		if(state!=0x08){//wrong state
			return SendError(state);
		}
		state=I2CSendSLA(d_addr+i2cwirte);
		if(state!=0x18){//wrong state
			return SendError(state);
		}
	
		state=I2CSendData( &r_addr,1);
		if(state!=0x28){
			return SendError(state);
		}
	
		state=I2CSendData(buffer,length);
		if(state!=0x28){
			return SendError(state);
		}
	
		I2CSendStop();
		i2c_state= I2C_OK;
		return i2c_state;
	}

	/****************************
	i2c master read
	*****************************/
	I2CResult Readi2c(uint8_t d_addr, uint8_t r_addr, uint8_t *buffer, uint32_t length){
		uint32_t state;

		if(i2c_state==I2C_NO_INIT){
			Initi2c();
		}

		state=I2CSendStart();

		if(state!=0x08){//wrong state
			return SendError(state);
		}
		state=I2CSendSLA(d_addr+i2cwirte);
		if(state!=0x18){//wrong state
			return SendError(state);
		}
	
		state=I2CSendData( &r_addr,1);
		if(state!=0x28){
			return SendError(state);
		}

		state=I2CSendReStart();
		if(state!=0x10){//wrong state
			return SendError(state);
		}

		state=I2CSendSLA(d_addr+i2cread);
		if(state!=0x40){//wrong state
			return SendError(state);
		}
	
		state=I2CReceiveData(buffer,length);
			if(state!=0x58){//wrong state
			return SendError(state);
		}
	
		I2CSendStop();
		//usually state==0x58;
		i2c_state= I2C_OK;
		return i2c_state;
	}
	
	/****************************
	i2c master read 2
	*****************************/
	I2CResult Readi2c(uint8_t d_addr, uint8_t *buffer, uint32_t length){
		uint32_t state;

		if(i2c_state==I2C_NO_INIT){
			Initi2c();
		}

		state=I2CSendStart();

		if(state!=0x08){//wrong state
			return SendError(state);
		}

		state=I2CSendSLA(d_addr+i2cread);
		if(state!=0x40){//wrong state
			return SendError(state);
		}
	
		state=I2CReceiveData(buffer,length);
			if(state!=0x58){//wrong state
			return SendError(state);
		}
	
		I2CSendStop();
		//usually state==0x58;
		i2c_state= I2C_OK;
		return i2c_state;
	}
	
	/****************************
	Return state
	*****************************/
	
	I2CResult GetI2CState(){
		return i2c_state;
	}
}//end of namespace i2c{}

