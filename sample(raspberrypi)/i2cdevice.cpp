/*************************************************
//
// i2cdevice.cpp
// Copyright (C) 2015, Y. Nomura, all right reserved.
// This software is released under the MIT License.
//
// http://opensource.org/licenses/mit-license.php
//
*************************************************/


#include "i2cdevice.h"
#include<unistd.h>
#include"i2c.h"
//#include "timer.h"
//platform independent


namespace i2c{

	/********************
	class I2CDevice (virtual class)
	********************/
	
	void I2CDevice::CheckInit(){
		if(device_state==SOFTWARE_RESET){
			SoftwareReset();
		}
	
		if(device_state==NO_DEVICE){
			Initialize(device_addr,device_mode);
		}
	}
	
	DeviceState I2CDevice::CheckI2CError(){
		I2CResult i2cerror=GetI2CState();

		if((i2cerror==SLAW_NACK)||(i2cerror==SLAR_NACK)){
				device_state=NO_DEVICE;
		}
		else{
			device_state=I2C_ERROR;
		}
		return device_state;
	}

	DeviceState I2CDevice::Initialize(uint8_t device_addr,uint8_t device_mode){
		device_state=NO_DEVICE;
		this->device_addr=device_addr;
		this->device_mode=device_mode;
		Initi2c();
		return device_state;
	}

#if USE_AQM0802

	/********************
	class AQM0802 
	********************/
	namespace{
		const uint8_t CMD=0x00;
		const uint8_t DAT=0x40;
	}


	DeviceState AQM0802::Initialize(uint8_t device_addr,uint8_t device_mode){
	
		I2CDevice::Initialize(device_addr,device_mode);

		// Wait 40ms
		wait_usec(100000);
		// Function set = 0x38
		if(Writei2c(this->device_addr,CMD,0x38)!=I2C_OK){return device_state;}
		// Wait 26.3us
		wait_usec(1000);
		// Function set = 0x39
		if(Writei2c(this->device_addr,CMD,0x39)!=I2C_OK){return device_state;}
		// Wait 26.3us
		wait_usec(1000);
		// Internal OSC frequency = 0x14
		if(Writei2c(this->device_addr,CMD,0x14)!=I2C_OK){return device_state;}
		// Wait 26.3us
		wait_usec(1000);
		// Contrast set = 0x70
		if(Writei2c(this->device_addr,CMD,0x70)!=I2C_OK){return device_state;}
		// Wait 26.3us
		wait_usec(1000);
		// Power/ICON/Contrast control = 0x56
		if(Writei2c(this->device_addr,CMD,0x56)!=I2C_OK){return device_state;}
		// Wait 26.3us
		wait_usec(1000);
		// Follower control = 0x6C
		if(Writei2c(this->device_addr,CMD,0x6c)!=I2C_OK){return device_state;}
		// Wait 200ms
		wait_usec(200000);
		// Function set = 0x38
		if(Writei2c(this->device_addr,CMD,0x38)!=I2C_OK){return device_state;}
		// Wait 26.3us
		wait_usec(1000);
		// Display ON/OFF control = 0x0C
		if(Writei2c(this->device_addr,CMD,0x0c)!=I2C_OK){return device_state;}
		// Wait 26.3us
		wait_usec(1000);
		// Clear Display = 0x01
		if(Writei2c(this->device_addr,CMD,0x01)!=I2C_OK){return device_state;}
		// Wait 1.08ms
		wait_usec(2000);
	
		device_state=FUNC_OK;
		return device_state;
	}

	DeviceState AQM0802::SetLcdData(const uint8_t *buffer,uint32_t length){
	
		//Check state
		CheckInit();
		if(device_state==NO_DEVICE){return device_state;}
	
		//maxsize of buff
		if(length>8){
			device_state=INVALID_PARAMETER;
			return device_state;
		}
	
		//write data with i2c
		if(Writei2c(device_addr,DAT,buffer,length)==I2C_OK){
			device_state=FUNC_OK;
		}
		else{
			CheckI2CError();
		}
		return device_state;
	}

	DeviceState AQM0802::SetLcdData(const int8_t *buffer,uint32_t length){
		uint8_t buff[8];

		if(length>8){
			device_state=INVALID_PARAMETER;
			return device_state;
		}

		for(int i=0;i<length;i++){
			buff[i]=(uint8_t) *buffer++;
		}
		return SetLcdData(buff,length);
	}

	DeviceState AQM0802::SetLcdData(const char *buffer,uint32_t length){
		uint8_t buff[8];

		if(length>8){
			device_state=INVALID_PARAMETER;
			return device_state;
		}

		for(int i=0;i<length;i++){
			buff[i]=(uint8_t) *buffer++;
		}
		return SetLcdData(buff,length);
	}


	DeviceState AQM0802::SetLocate(uint32_t x,uint32_t y){
	
		//NO device
		CheckInit();
		if(device_state==NO_DEVICE){return device_state;}
		
		//y:0 or 1,x:0~7
		if((y>1)||(x>7)){
			device_state=INVALID_PARAMETER;
			return device_state;
		}
		//write data with i2c
		if(Writei2c(device_addr,CMD,(0x80 + y*0x40 + x))==I2C_OK){
			device_state=FUNC_OK;
		}else{
			CheckI2CError();
		}
		return device_state;
	}

	DeviceState AQM0802::ClearDisp(){

		//NO device
		CheckInit();
		if(device_state==NO_DEVICE){return device_state;}

		//Clear Display = 0x01
		if(Writei2c(device_addr,CMD,0x01)!=I2C_OK){
			return CheckI2CError();
		}
		
		wait_usec(2000);// Wait 1.08ms
		//Return Home = 0x03
		if(Writei2c(device_addr,CMD,0x03)!=I2C_OK){
			return CheckI2CError();
		}

		wait_usec(2000);// Wait 1.08ms
		device_state=FUNC_OK;
		return device_state;
	}

#endif
#if USE_LPS331AP

	/********************
	class LPS331AP 
	********************/
	
	void LPS331AP::SoftwareReset(){
		Writei2c(device_addr,0x21,0x84);
		wait_usec(100000);
		Writei2c(device_addr,0x21,0x00);
	}


	DeviceState LPS331AP::Initialize(uint8_t device_addr,uint8_t device_mode){
		uint8_t buff=0x00;

		I2CDevice::Initialize(device_addr,device_mode);
	
		if(Readi2c(this->device_addr,0x0f,&buff,1)!=I2C_OK){
			return device_state;
		}
		
		if(buff!=LPS331AP_WHO_AM_I){//confirm Who Am I
			return device_state;
		}

		if(Writei2c(this->device_addr,0x20,0x00)!=I2C_OK){//PD
			return device_state;
		}
		wait_usec(1000);
		/* mode 0x00: one-shot,0x01 1sps, other modes are written in datasheet*/

		if(this->device_mode>LPS_LAST-1){return INVALID_PARAMETER;}
	
		if(Writei2c(this->device_addr,0x20,0x80+(device_mode<<4)+0x02)!=I2C_OK){//PU and set mode
			return device_state;
		}
		wait_usec(10);
	
		device_state=FUNC_OK;
		return device_state;
	}

	DeviceState LPS331AP::ReadData(){
		uint8_t data[5];
	
		//device_state_check
		CheckInit();
		if(device_state==NO_DEVICE){return device_state;}
	
		//one-shot operation of wake up
		if(device_mode==LPS_ONESHOT){
			if(Writei2c(device_addr,0x21,0x01)!=I2C_OK){
				return CheckI2CError();
			}
			//check status_reg
			uint32_t i=0;
			uint8_t state;
			do{
				if(++i>100){//something error software reset;
					device_state=SOFTWARE_RESET;
					return device_state;
				}
				wait_usec(10000);
				if(Readi2c(device_addr,0x27,&state,1)!=I2C_OK){
					return CheckI2CError();
				}
			}while((state & 0x03)==0x00);
		}

		if(Readi2c(device_addr,0x28+0x80,data,5)!=I2C_OK){
			return CheckI2CError();
		}
		
		if((data[2]>0x7f)||(data[2]<0x10)){
			device_state=OUT_OF_RANGE;
			return device_state;
		}
		press_rawdata = ((data[2]&0x7f)<<16)+(data[1]<<8)+data[0];
		tmp_rawdata = (data[4]<<8)+data[3];
	
		device_state=FUNC_OK;
		return device_state;
	}

	DeviceState LPS331AP::GetData(double *pre_data){
		if(ReadData()!=FUNC_OK){return device_state;}

		*pre_data=(double)press_rawdata/4096.0;// (/4096)
		return device_state;
	}


	DeviceState LPS331AP::GetData(double *pre_data,double *tmp_data){

		if(GetData(pre_data)!=FUNC_OK){return device_state;}

		*tmp_data=42.5+(double)tmp_rawdata/480.0;
		return device_state;
	}

#endif

#if USE_ADT7410

	/********************
	class ADT7410 
	********************/

	void ADT7410::SoftwareReset(){
		Writei2c(device_addr,0x2f,0x00,0);
		wait_usec(100000);
	}

	DeviceState ADT7410::Initialize(uint8_t device_addr,uint8_t device_mode){
	
		I2CDevice::Initialize(device_addr,device_mode);
	
		/*0x00 continuous mode, 0x01 one shot, 0x10 one-sps,0x11 shut down */

		if(this->device_mode>ADT_LAST-1){return INVALID_PARAMETER;}

		if(Writei2c(this->device_addr,0x03,0x80+(this->device_mode<<5))!=I2C_OK){
			return device_state;
		}
	
		device_state=FUNC_OK;
		return device_state;
	}

	DeviceState ADT7410::ReadData(){
		uint8_t buff[2],state=0x00;
	
		CheckInit();
		if(device_state==NO_DEVICE){return device_state;}

		if(device_mode==ADT_ONESHOT){//one-shot
			state=0xa0;
			if(Writei2c(device_addr,0x03,&state,1)!=I2C_OK){
				return CheckI2CError();
			}
		//check status_reg
		wait_usec(250000);
		}

		state=0x80;
		uint32_t i=0;
		do{
			if(++i>100){//something error software reset;
				device_state=SOFTWARE_RESET;
				return device_state;
			}
			wait_usec(10000);
			if(Readi2c(device_addr,0x02,&state,1)!=I2C_OK){
				return CheckI2CError();
			}
		}while(state & 0x80);
	
		if(Readi2c(device_addr,0x00,buff,2)!=I2C_OK){
			return CheckI2CError();
		}
		tmp_rawdata=(buff[0]<<8)+(buff[1]);

		device_state=FUNC_OK;
		return device_state;
	}

	DeviceState ADT7410::GetData(double *tmp_data){
		if(ReadData()!=FUNC_OK){return device_state;}

		*tmp_data=(double)tmp_rawdata/128.0;//128
		return device_state;
	}

#endif
#if USE_SHT21

	/********************
	class ADT7410 
	********************/
	
	
#define SHT21_READ_RH 0xe5
#define SHT21_READ_TMP 0xe3

	DeviceState SHT21::Initialize(uint8_t device_addr,uint8_t device_mode){
		I2CDevice::Initialize(device_addr,device_mode);
		device_state=FUNC_OK;
		return device_state;
	}

	DeviceState SHT21::ReadData(){
		uint8_t buff[3];

		//buff[3]:dummy;
		if(Readi2c(device_addr,SHT21_READ_RH,buff,3)!=I2C_OK){
			return CheckI2CError();
		}
		buff[1]&=0xfc;
		hum_rawdata=(buff[0]<<8)+buff[1];

		if(Readi2c(device_addr,SHT21_READ_TMP,buff,3)!=I2C_OK){
			return CheckI2CError();
		}
		buff[1]&=0xfc;
		tmp_rawdata=(buff[0]<<8)+buff[1];

		return FUNC_OK;
	}
	
	DeviceState SHT21::GetData(double *hum_data){

		if(ReadData()!=FUNC_OK){return I2C_ERROR;}
		*hum_data=125*(double)hum_rawdata/65536.0-6;
		return FUNC_OK;
	}
	
	DeviceState SHT21::GetData(double *hum_data,double *tmp_data){

		if(GetData(hum_data)!=FUNC_OK){return I2C_ERROR;}
		*tmp_data=175.72*(double)tmp_rawdata/65536.0-46.85;
		return FUNC_OK;
	}

#endif

}
