#include "bbtcpa_i2c_driver.h"

#pragma  code_seg("")

#define uint32_t unsigned long
#define uint16_t unsigned int
#define uint8_t unsigned char

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
I2CResult Writei2c(uint8_t d_addr, uint8_t r_addr, uint8_t *buffer, uint32_t length);
//I2CResult Writei2c(uint8_t d_addr, uint8_t r_addr, uint8_t buffer);
I2CResult Readi2c(uint8_t d_addr, uint8_t r_addr, uint8_t *buffer, uint32_t length);
I2CResult Readi2c_no_r_addr(uint8_t d_addr, uint8_t *buffer, uint32_t length);
I2CResult GetI2CState();
void ReadIO (  WORD Address,  WORD OpFlag, void* Value );
void WriteIO (  WORD Address,  WORD OpFlag,  void* Value );

//********************
//
// local variable
// scope is only this file
//
//********************
const uint32_t gpio_sda = 19;
const uint32_t gpio_scl= 21;

const uint32_t gpio_low = 0;
const uint32_t gpio_high = 1;		//should not be used in code. no level high in the transcation. for strench.
const uint32_t gpio_input = 0x0;
const uint32_t gpio_output = 0x1;
const uint32_t i2c_time = 1;	//2 for 174KHz, 1 for 358KHz
const uint8_t i2cwrite =0x00;
const uint8_t i2cread = 0x01;


/*Board related setting starts. */
void gpio_init(){
	uint32_t ddGpioUse, ddGpioType, ddGpioLvl;
	
	ReadIO(0x500, AccWidthUint32, &ddGpioUse);
	ReadIO(0x504, AccWidthUint32, &ddGpioType);
	ReadIO(0x50C, AccWidthUint32, &ddGpioLvl);

//Program them to output low
	ddGpioUse |= (1<<19 | 1<<21);	//in use
	WriteIO(0x500, AccWidthUint32, &ddGpioUse );

//Program them to input
	ddGpioType |= (1<<19 | 1<<21);	//change back to input, for push pull.
	WriteIO(0x504, AccWidthUint32, &ddGpioType);
}

void usleep(uint32_t time){
	uint8_t b1usDelay = 12, bDummy;
	
/*
	mov	cx, 12			; For 1 microsec delay
bd_00:
	in	al, 61h
	loop	bd_00			; Complete 1 micro sec delay
*/	
	do {
		b1usDelay = 1; 
		while( b1usDelay-- ) {
			//WriteIO(0xED, AccWidthUint8, &bDummy);
			atom_delay();
		}
	} while(time-- != 0);
	
	return;	
}

void gpio_clear(uint32_t pin)
{
	uint32_t ddGpioLvl;
	ReadIO(0x50C, AccWidthUint32, &ddGpioLvl);
//Program it to output low
	ddGpioLvl &= (~(1<<pin));
	WriteIO(0x50C, AccWidthUint32, &ddGpioLvl);

	return;
}

//const uint32_t gpio_input = 0x0;
//const uint32_t gpio_output = 0x1;
void gpio_configure(uint32_t pin, uint32_t mode) 
{
	uint32_t ddGpioType;

	ReadIO(0x504, AccWidthUint32, &ddGpioType);
//const uint32_t gpio_input = 0x0;
//const uint32_t gpio_output =0x1;
	if( mode == gpio_input ) {
		ddGpioType |= (1<<pin);
		WriteIO(0x504, AccWidthUint32, &ddGpioType);			
	} else if(mode == gpio_output ){
		ddGpioType &=  (~(1<<pin));
		WriteIO(0x504, AccWidthUint32, &ddGpioType);
	}
	return;
}
void gpio_set(uint32_t pin)
{
	uint32_t ddGpioType;
	ReadIO(0x504, AccWidthUint32, &ddGpioType);
//Program it to input
	ddGpioType |= (1<<pin);
	WriteIO(0x504, AccWidthUint32, &ddGpioType);

	return;	
}

uint32_t gpio_read(uint32_t pin) 
{
	uint32_t ddGpioLvl;
	ReadIO(0x50C, AccWidthUint32, &ddGpioLvl);
	return (ddGpioLvl>>pin)&0x01;
}
/*Board related setting ends. */

//********************
//
// local function (typically platform independent)
// scope is only this file
//
//********************
uint8_t bSerialData = 0;
I2CResult i2c_state = I2C_NO_INIT;		//Global variable, not sure it will work or not.

void GPIOInit(){
	gpio_init();
}

//const uint32_t gpio_input = 0x0;
//const uint32_t gpio_output = 0x1;
void GPIOSetDir(uint32_t pin, uint32_t mode){
	gpio_configure(pin,mode);
}


//const uint32_t gpio_low = 0;
//const uint32_t gpio_high = 1;		//should not be used in code. no level high in the transcation. for strench.
void GPIOSetValue(uint32_t pin, uint32_t value ){
	if(value==0){
		gpio_clear(pin);
	}
	else if(value==1){
		gpio_set(pin);
	}
}

uint32_t GPIOGetState(uint32_t pin){
	return gpio_read(pin);
}

void wait_usec(uint32_t time){
	usleep(time);
}


/*************************
Send Byte and receive ACK/NACK
**************************/
uint32_t I2CSendByte(const uint8_t data){
	int i;
	int pinstate=0;
	for(i=0;i<8;++i){
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
		while(!GPIOGetState(gpio_scl)){}//wait for Clock stretching
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
	while(!GPIOGetState(gpio_scl)){}//wait for Clock stretching
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
	while(i<50){// if SCL or SDA is LOW which means I2C busy
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
	while(!GPIOGetState(gpio_scl)){}//wait for Clock stretching
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
	int i;
	uint32_t buscheck;
	for(i=0;i<length;++i){
		buscheck=I2CSendByte(*buffer++);
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
	int i, j;
	for(i=0;i<length;++i){
		*buffer=0x00;
		GPIOSetDir(gpio_scl, gpio_output);
		GPIOSetValue(gpio_scl,gpio_low);
		GPIOSetDir(gpio_sda, gpio_input);
		wait_usec(i2c_time);
		for(j=0;j<8;++j){
			wait_usec(i2c_time);
			//SCL HIGH
			GPIOSetDir(gpio_scl, gpio_input);
			while(!GPIOGetState(gpio_scl)){}//wait for Clock stretching
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
			while(!GPIOGetState(gpio_scl)){}//wait for Clock stretching
			wait_usec(i2c_time);
			break;
		}
		else{//ACK return
			GPIOSetDir(gpio_sda, gpio_output);
			GPIOSetValue(gpio_sda,gpio_low);
			wait_usec(i2c_time);
			//SCL HIGH
			GPIOSetDir(gpio_scl, gpio_input);
			while(!GPIOGetState(gpio_scl)){}//wait for Clock stretching
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
	while(!GPIOGetState(gpio_scl)){}//wait for Clock stretching
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
	int i;
	GPIOSetDir(gpio_sda, gpio_input);
	for(i=0;i<9;++i){
		GPIOSetDir(gpio_scl, gpio_output);
		GPIOSetValue(gpio_scl, gpio_low);
		wait_usec(i2c_time);
		//SCL HIGH
		GPIOSetDir(gpio_scl, gpio_input);
		while(!GPIOGetState(gpio_scl)){}//wait for Clock stretching
			wait_usec(i2c_time);
		}
	I2CSendStop();
}
/****************************
i2c master write
*****************************/
I2CResult Writei2c( uint8_t d_addr, uint8_t r_addr,const uint8_t *buffer, uint32_t length)
{
	uint32_t state;
	if(i2c_state==I2C_NO_INIT){
		Initi2c();
	}
	state=I2CSendStart();
	if(state!=0x08){//wrong state
		return SendError(state);
	}
	state=I2CSendSLA(d_addr+i2cwrite);
	if(state!=0x18){//wrong state
		return SendError(state);
	}
//no slave address...	state=I2CSendData( &r_addr,1);
//no slave address...	if(state!=0x28){
//no slave address...		return SendError(state);
//no slave address...	}

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
	state=I2CSendSLA(d_addr+i2cwrite);
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
I2CResult Readi2c_no_r_addr(uint8_t d_addr, uint8_t *buffer, uint32_t length){
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




void
ReadIO (
  WORD Address,
  WORD OpFlag,
  void* Value
  )
{
  OpFlag = OpFlag & 0x7f;
  __asm {		//Calibrate ss, ds for pointer.
	push	ss
	pop	ds
  }
  switch ( OpFlag ) {
  case AccWidthUint8:
    *(BYTE*)Value = ReadIO8 (Address);
    break;
  case AccWidthUint16:
    *(WORD*)Value = ReadIO16 (Address);
    break;
  case AccWidthUint32:
    *(DWORD*)Value = ReadIO32 (Address);
    break;
  }
}

void
WriteIO (
  WORD Address,
  WORD OpFlag,
  void* Value
  )
{
  OpFlag = OpFlag & 0x7f;
  __asm {		//Calibrate ss, ds for pointer.
	push	ss
	pop	ds
  }
  switch ( OpFlag ) {
  case AccWidthUint8:
    WriteIO8 (Address, *(BYTE*)Value);
    break;
  case AccWidthUint16:
    WriteIO16 (Address, *(WORD*)Value);
    break;
  case AccWidthUint32:
    WriteIO32 (Address, *(DWORD*)Value);
    break;
  }
}

void SendCharToSerial(uint8_t bData) {
	WriteIO(0x3F8, AccWidthUint8, &bData);
}

void SendStringToSerial(uint8_t* sMessage, uint32_t nLength) {
	uint32_t i;
	for(i=0;i<nLength;i++)
		SendCharToSerial(sMessage[i]);
}
void outputAsciiData(uint32_t ddData){
	uint8_t i, bData;
	for(i=0;i<8;i++){
		bData = ((ddData << (i*4))&0xF0000000) >> 28;
		if(bData > 9) 
			SendCharToSerial( (bData-10) +'A');
		else 
			SendCharToSerial( (bData) +'0');
	}
}
//Main entrance.
uint8_t gI2CBuffer_0[128];
uint8_t gI2CBuffer_1[128];
uint8_t gI2CBuffer_2[128];
uint8_t gI2CBuffer_3[128];
uint8_t gI2CBuffer_4[128];
uint8_t gI2CBuffer_5[128];
uint8_t gI2CBuffer_6[128];
uint8_t gI2CBuffer_7[128];

void gpio_i2c_interface(uint8_t cmdType) 
{
	uint32_t i;
	uint8_t	*pBuffer;
	uint32_t nLength;
	uint8_t bTpmI2cAddress = 0x52;
			
	uint8_t	bTestOutput;
	i = 0;
	pBuffer = gI2CBuffer_0;
	//SendStringToSerial("gpio_i2c_interface\n", 19);
	
//	for(i=1*128;i<128*7;i++)
//		pBuffer[i] = 0x5A;

#if 0
	for(i=0;i<128*8;i++) {
		
		bTestOutput = pBuffer[i];
		bSerialData = bTestOutput;
		send_al_to_COM_port(bTestOutput);
		if(i%32==31) {
			bSerialData = 0x0D;
			send_al_ascii_to_COM_port(0x0D);
		}
		
	}
	bSerialData = 0x0D;
	send_al_ascii_to_COM_port(0);
#endif
	
	gpio_init();
	wait_usec(1000*100*3);	//puase for 2.2ms
	bTestOutput = 0x12;
	
	{
		//uint32_t ddGpioUse;
		//uint32_t ddGpioType;
		//uint32_t ddGpioLvl;

#if 0		//Gpio test, 174KHz.	
		for(i=0;i<3;i++) {
			

		//SendStringToSerial("set sda O_L ", 12);
			GPIOSetDir(gpio_sda, gpio_output);
			GPIOSetValue(gpio_sda,gpio_low);
		//SendStringToSerial("::: ", 4);
		/*
		ReadIO(0x500, AccWidthUint32, &ddGpioUse);
		ReadIO(0x504, AccWidthUint32, &ddGpioType);
		ReadIO(0x50C, AccWidthUint32, &ddGpioLvl);
		
		bTestOutput = ((ddGpioUse &0x00FF0000) >> 16);
		WriteIO(0x80, AccWidthUint8, &bTestOutput);
		usleep(1000*50);
		
		bTestOutput = ((ddGpioType &0x00FF0000) >> 16);
		WriteIO(0x80, AccWidthUint8, &bTestOutput);
		usleep(1000*50);
		
		bTestOutput = ((ddGpioLvl &0x00FF0000) >> 16);
		WriteIO(0x80, AccWidthUint8, &bTestOutput);
		usleep(1000*50);
		
		outputAsciiData( ddGpioUse );
		//SendStringToSerial(": ", 2);
		outputAsciiData( ddGpioType );
		//SendStringToSerial(": ", 2);
		outputAsciiData( ddGpioLvl );
		//SendStringToSerial(":\n", 2);
		*/
			//usleep(1000);
			wait_usec(i2c_time);
			
			GPIOSetDir(gpio_sda, gpio_input);
			
			wait_usec(i2c_time);
			//GPIOSetValue(gpio_sda,gpio_low);
			
		}
#endif			
	}

//Send write data i2C cmd. //Writei2c(uint8_t d_addr, uint8_t r_addr, uint8_t *buffer, uint32_t length)
#if 0
	{
		uint32_t nLength;
		uint8_t bTpmI2cAddress = 0x52;
		//00 c1 00 00 00 0c 00 00 00 99 00 01 
		/*
		gI2CBuffer_0[0] = 0x00;
		gI2CBuffer_0[1] = 0xC1;
		gI2CBuffer_0[2] = 0x00;
		gI2CBuffer_0[3] = 0x00;
		gI2CBuffer_0[4] = 0x00;
		gI2CBuffer_0[5] = 0x0C;
		gI2CBuffer_0[6] = 0x00;
		gI2CBuffer_0[7] = 0x00;
		gI2CBuffer_0[8] = 0x00;
		gI2CBuffer_0[9] = 0x99;
		gI2CBuffer_0[10] = 0x00;
		gI2CBuffer_0[11] = 0x01;
		*/
		nLength = 12;
		Writei2c( 0x52, 0x00, gI2CBuffer_0, nLength);
	}
#endif		
	
//Send read no address i2C cmd.
	if( pBuffer[128*8-1] == 'R' && (nLength = *(uint32_t*)&pBuffer[128*8-1-4]) )	//Receive
	{
		*(uint32_t*)&gI2CBuffer_0[0] = 0x00000000;
		while( *(uint32_t*)&gI2CBuffer_0[0] != 0x0000C400 ) {	//00 c4 00 00
			*(uint32_t*)&gI2CBuffer_0[0] = 0x00000000;
			Readi2c_no_r_addr(bTpmI2cAddress, gI2CBuffer_0, nLength);
			wait_usec(1000*100*3*15);	//2.2ms *3 * 15
		}
#if 0
		
		pBuffer = gI2CBuffer_0;
		for(i=0;i<nLength;i++) {
			bTestOutput = pBuffer[i];
			bSerialData = bTestOutput;
			send_al_to_COM_port(bTestOutput);
		
			if(i%32==31) {
				bSerialData = 0x0D;
				send_al_ascii_to_COM_port(0);
			}
		}
		bSerialData = 0x0D;
		send_al_ascii_to_COM_port(0);
		
#endif		
	} else if( pBuffer[128*8-1] == 'S' && (nLength = *(uint32_t*)&pBuffer[128*8-1-4]) ) {	//Send
#if 0
		pBuffer = gI2CBuffer_0;
		for(i=0;i<nLength;i++) {
			bTestOutput = pBuffer[i];
			bSerialData = bTestOutput;
			send_al_to_COM_port(bTestOutput);
		
			if(i%32==31) {
				bSerialData = 0x0D;
				send_al_ascii_to_COM_port(0);
			}
		}
		bSerialData = 0x0D;
		send_al_ascii_to_COM_port(0);
#endif		
		//wait_usec(0xA000000);
		
		Writei2c( bTpmI2cAddress, 0x00, gI2CBuffer_0, nLength);
		while(nLength--)
			wait_usec(1000);	//2.2ms
		//wait_usec(0xA000000);
	}
	
	return;
}

