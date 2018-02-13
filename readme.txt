# Build a wrapper function to convert TPM 1.2 TIS(LPC bus) to i2c bus(bitbang, GPIO) and forward the commands to Atmel 3205 I2C TPM.

# This work is to enable i2c tpm 1.2 function on Intel ICH/PCH platform. 

# Because this code is for AMI Core 8 legacy code(x86 assembly), it requires to use VC compiler to compile the source(b.bat) 
into assembly and then remove the text segment declaraion and copy the processed code to AMI Core 8 TCPA stack (CSP\EM\TCPA\TPM12\tpm12.asm).
