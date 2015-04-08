/*************************************************
//
// I2Cmod.cpp
// Copyright (C) 2015, Y. Nomura, all right reserved.
// This software is released under the MIT License.
//
// http://opensource.org/licenses/mit-license.php
//
*************************************************/

#include<unistd.h>
#include <iostream>

#include "i2c.h"
#include "i2cdevice.h"

// TODO: insert other include files here

// TODO: insert other definitions and declarations here
void changechar(uint8_t *buff,int32_t num,uint32_t fig){
	uint32_t pow=1;
	if(fig>8){fig=8;}

	for(uint32_t i=0;i<fig-1;i++){
		pow*=10;
	}
	if(num<0){
		*buff++='-';
	}
	else{*buff++=0x20;}

	for(uint32_t i=0;i<fig;++i){
		for(int8_t j=9;j>=0;j--){
			uint32_t compnum=j*pow;
			if(num >= compnum){
				*buff++=0x30+j;
				num-=compnum;
				break;
			}
		}
		if(pow==1){
			break;
		}
		pow/=10;
	}
}


int main(void) {

    // TODO: insert code here

    // Force the counter to be placed into memory

	uint8_t buff[8];
	uint8_t buff2[8];
	i2c::DeviceState device_state;

	class i2c::AQM0802 disp1;
	class i2c::LPS331AP presclass1;
	class i2c::ADT7410 temperatureclass1;
	class i2c::SHT21 humidity1;

    // Enter an infinite loop, just incrementing a counter
	device_state=disp1.SetLocate(0,0);
	std::cout<<device_state<<",i2cstate:"<< i2c::GetI2CState()<<std::endl;
	device_state=disp1.SetLcdData("Hello!",6);
	
	std::cout<<device_state<<",i2cstate:"<< i2c::GetI2CState()<<std::endl;
	
	sleep(1);

while(1) {
	int32_t press,press_tmp;
	int32_t tmp;
	int32_t rh,rh_tmp;
	double press_d,press_tmp_d;
	double tmp_d;
	double rh_d,rh_tmp_d;	

	device_state=presclass1.GetData(&press_d, &press_tmp_d);
	if(device_state==i2c::FUNC_OK){
		
		std::cout<<press_d<<"hPa,"<<press_tmp_d<<"C"<<std::endl;
		press=press_d*10;
		press_tmp=press_tmp_d*10;
		changechar(buff,press,5);
		buff[0]=buff[1];
		buff[1]=buff[2];
		buff[2]=buff[3];
		buff[3]=buff[4];
		buff[4]='.';
		buff[6]='h';
		buff[7]='P';

		changechar(buff2,press_tmp,3);
		buff2[4]=buff2[3];
		buff2[3]='.';
		buff2[5]=0xdf;
		buff2[6]='C';

		disp1.ClearDisp();
		device_state=disp1.SetLocate(0,0);
		device_state=disp1.SetLcdData(buff,8);
		device_state=disp1.SetLocate(1,1);
		device_state=disp1.SetLcdData(buff2,7);
	}
	else{
		std::cout<<"devicestate"<<device_state<<",i2cstate:"<< i2c::GetI2CState()<<std::endl;
		buff[0]='E';
		buff[1]='r';
		buff[2]='r';
		buff[3]='o';
		buff[4]='r';
		disp1.ClearDisp();
		device_state=disp1.SetLocate(0,0);
		device_state=disp1.SetLcdData(buff,5);
	}
	sleep(1);

	device_state=temperatureclass1.GetData(&tmp_d);
	if(device_state==i2c::FUNC_OK){
		std::cout<<tmp_d<<"C"<<std::endl;
		tmp=tmp_d*10;
		changechar(buff2,tmp,3);
		buff2[4]=buff2[3];
		buff2[3]='.';
		buff2[5]=0xdf;
		buff2[6]='C';

		disp1.ClearDisp();
//		device_state=disp1.SetLocate(0,0);
//		device_state=disp1.SetLcdData(buff,8);
		device_state=disp1.SetLocate(1,1);
		device_state=disp1.SetLcdData(buff2,8);
	}

	else{
		std::cout<<"devicestate"<<device_state<<",i2cstate:"<< i2c::GetI2CState()<<std::endl;
		buff[0]='E';
		buff[1]='r';
		buff[2]='r';
		buff[3]='o';
		buff[4]='r';
		buff[5]='2';
		buff[6]=0x20;
		buff[7]=0x20;

		disp1.ClearDisp();
		device_state=disp1.SetLocate(0,0);
		device_state=disp1.SetLcdData(buff,8);
//		device_state=disp1.SetLocate(1,1);
//		device_state=disp1.SetLcdData(buff2,7);
	}

	sleep(1);

	device_state=humidity1.GetData(&rh_d,&rh_tmp_d);
	if(device_state==i2c::FUNC_OK){
		std::cout<<rh_d<<"%,"<<rh_tmp_d<<"C"<<std::endl;
		rh=rh_d*10;
		rh_tmp=rh_tmp_d*10;
		buff[0]=0x20;
		buff[1]=0x20;
		changechar(&buff[2],rh,3);
		buff[6]=buff[5];
		buff[5]='.';
		buff[7]=0x25;

		changechar(buff2,rh_tmp,3);
		buff2[4]=buff2[3];
		buff2[3]='.';
		buff2[5]=0xdf;
		buff2[6]='C';

		disp1.ClearDisp();
		device_state=disp1.SetLocate(0,0);
		device_state=disp1.SetLcdData(buff,8);
		device_state=disp1.SetLocate(1,1);
		device_state=disp1.SetLcdData(buff2,8);

	}

	else{
		std::cout<<"devicestate"<<device_state<<",i2cstate:"<< i2c::GetI2CState()<<std::endl;
		buff[0]='E';
		buff[1]='r';
		buff[2]='r';
		buff[3]='o';
		buff[4]='r';
		buff[5]='3';
		buff[6]=0x20;
		buff[7]=0x20;
		disp1.ClearDisp();
		device_state=disp1.SetLocate(0,0);
		device_state=disp1.SetLcdData(buff,8);
//		device_state=disp1.SetLocate(1,1);
//		device_state=disp1.SetLcdData(buff2,7);


	}

	sleep(1);
}


    return 0 ;
}
