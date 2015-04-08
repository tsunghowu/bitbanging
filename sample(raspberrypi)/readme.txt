/*************************************************
//
// Copyright (C) 2015, Y. Nomura, all right reserved.
// This software is released under the MIT License.
//
// http://opensource.org/licenses/mit-license.php
//
*************************************************/

This sample requires two more files.
	"raspGPIO.h" and "raspGPIO.cpp"

	you can get "http://www.myu.ac.jp/~xkozima/lab/raspTutorial3.html".

	Functions in these files provide GPIO operation for BCM2835. So, work in type A, A+, B, B+.
	Not work in raspberry pi 2 (I do not confirm).

This sample includes the following file.
	-I2Cmod.cpp (main file)
	-i2cdevice.h and i2cdevice.cpp (provide using method of some module)
	-i2c.h and i2c.cpp (provide i2c bit banging )

