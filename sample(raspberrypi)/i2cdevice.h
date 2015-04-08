/*************************************************
//
// i2cdevice.h
// Copyright (C) 2015, Y. Nomura, all right reserved.
// This software is released under the MIT License.
//
// http://opensource.org/licenses/mit-license.php
//
*************************************************/

#ifndef __I2Cdevice_H_
#define __I2Cdevice_H_

//#include <cstdint>
#include <stdint.h>


/*
//	use device 0:disable 1:enable
*/
#define USE_AQM0802 1
#define USE_LPS331AP 1
#define USE_ADT7410 1
#define USE_SHT21 1


namespace i2c{

	/* status of i2c device */
	typedef enum{
		FUNC_OK=0,
		NO_DEVICE,
		I2C_ERROR,
		INVALID_PARAMETER,
		OUT_OF_RANGE,
		SOFTWARE_RESET,
	}DeviceState;

	/* masterclass of I2Cdevicd (virtual class, can not creat object) */

	class I2CDevice{
		private:
		I2CDevice(const I2CDevice&);
		I2CDevice& operator=(const I2CDevice&);
		virtual void SoftwareReset(){}
	protected:
		DeviceState device_state;
		uint8_t device_addr,device_mode;
		I2CDevice(){device_state=NO_DEVICE;}
		void CheckInit();
		DeviceState CheckI2CError();
		
	public:
		virtual DeviceState Initialize(uint8_t device_addr,uint8_t device_mode);
		DeviceState GetDeviceState()const{return device_state;}
	};


#if USE_AQM0802

#define  AQM0802_ADDR 0x7c

	/* class of AQM0802 (may be OK for AQM1602) */

	class AQM0802 : public I2CDevice{
		public:
		AQM0802(){Initialize(AQM0802_ADDR);}
		AQM0802(uint8_t device_addr){Initialize(device_addr);}
	
		DeviceState Initialize(uint8_t device_addr,uint8_t device_mode=0);
		DeviceState SetLcdData(const uint8_t *buffer,uint32_t length);
		DeviceState SetLcdData(const int8_t *buffer,uint32_t length);
		DeviceState SetLcdData(const char *buffer,uint32_t length);
		DeviceState SetLocate(uint32_t x,uint32_t y);
		DeviceState ClearDisp();
	};

#endif

#if USE_LPS331AP

#define LPS331AP_WHO_AM_I 0xbb
#define LPS331AP_ADDR	0xb8

	/* class of LPS331AP or LPS21AP? */

	typedef enum{
		LPS_ONESHOT=0,
		LPS_1SPS,//P:1SPS,T:1SPS
		LPS_71SPS,//P:7SPS,T:1SPS
		LPS_121SPS,//P:12.5SPS,T:1SPS
		LPS_251SPS,//P:25SPS,T:1SPS
		LPS_77SPS,//P:7SPS,T:7SPS
		LPS_1212SPS,//P:12.5SPS,T:12.5SPS
		LPS_2525SPS,//P:25SPS,T:25SPS
		LPS_LAST,
	}LPS331APMeasureMode;

	class LPS331AP : public I2CDevice{
	private:
		uint32_t press_rawdata;
		int16_t tmp_rawdata;
	
		void SoftwareReset();
		DeviceState ReadData();
	public:
		LPS331AP(){Initialize(LPS331AP_ADDR,(uint8_t) LPS_1SPS);}
		LPS331AP(uint8_t device_addr,uint8_t device_mode){Initialize(device_mode,device_addr);}

		DeviceState Initialize(uint8_t device_addr,uint8_t device_mode);
		DeviceState GetData(double *pre_data);
		DeviceState GetData(double *pre_data,double *tmp_data);
	};

#endif

#if USE_ADT7410

#define ADT7410_ADDR	0x90

	/*class of ADT7410*/

	typedef enum{
		ADT_CONT=0,
		ADT_ONESHOT,
		ADT_1SPS,//T:1SPS
		ADT_SHUT,//shutdown
		ADT_LAST,
	}ADT7410MeasureMode;


	class ADT7410:public I2CDevice{
	private:
		int16_t tmp_rawdata;

		DeviceState ReadData();
		void SoftwareReset();

	public:
		ADT7410(){Initialize(ADT7410_ADDR,(uint8_t)ADT_CONT);}
		ADT7410(uint8_t device_addr,uint8_t device_mode){Initialize(device_addr,device_mode);}

		DeviceState Initialize(uint8_t device_addr,uint8_t device_mode);
		DeviceState GetData(double *tmp_data);
};

#endif

#if USE_SHT21

#define SHT21_ADDR 0x80

	/*class of SHT21*/

	class SHT21:public I2CDevice{
	private:
		uint16_t hum_rawdata,tmp_rawdata;
		DeviceState ReadData();

	public:
		SHT21(){Initialize(SHT21_ADDR);};
		SHT21(uint8_t device_addr){Initialize(device_addr);}
		DeviceState Initialize(uint8_t device_addr,uint8_t device_mode=0);
		DeviceState GetData(double *hum_data);
		DeviceState GetData(double *hum_data,double *tmp_data);
	};

#endif

}
#endif
