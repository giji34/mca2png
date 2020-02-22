/* ////////////////////////////////////////////////////////////

YS Bitmap / YS PNG library
Copyright (c) 2014 Soji Yamakawa.  All rights reserved.
http://www.ysflight.com

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, 
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation 
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS 
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

File Name: encodersample.cpp
//////////////////////////////////////////////////////////// */

// Can be compiled as:
//   cl encodersample.cpp ..\src\yspngenc.cpp ..\src\yspng.cpp /I..\src

#include <stdio.h>
#include <yspngenc.h>


unsigned char greyscale16x16[]=
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x00,0xff,0xff,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x00,0x00,0xff,0xff,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x00,0x00,0x00,0xff,0xff,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x00,0x00,0x00,0x00,0xff,0xff,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x00,
	0x00,0x00,0x00,0x00,0x00,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

unsigned char rgb16x16[]=
{//   R   G   B   R   G   B   R   G   B   R   G   B   R   G   B   R   G   B   R   G   B   R   G   B   R   G   B   R   G   B   R   G   B   R   G   B   R   G   B   R   G   B   R   G   B   R   G   B
      0,  0,255,  0, 16,240,  0, 32,224,  0, 48,208,  0, 64,192,  0, 80,176,  0, 96,160,  0,112,144,  0,128,128,  0,144,112,  0,160, 96,  0,176, 80,  0,192, 64,  0,208, 48,  0,224, 32,  0,240, 16,
     16,  0,255, 16, 16,240, 16, 32,224, 16, 48,208, 16, 64,192, 16, 80,176, 16, 96,160, 16,112,144, 16,128,128, 16,144,112, 16,160, 96, 16,176, 80, 16,192, 64, 16,208, 48, 16,224, 32, 16,240, 16,
     32,  0,255, 32, 16,240, 32, 32,224, 32, 48,208, 32, 64,192, 32, 80,176, 32, 96,160, 32,112,144, 32,128,128, 32,144,112, 32,160, 96, 32,176, 80, 32,192, 64, 32,208, 48, 32,224, 32, 32,240, 16,
     48,  0,255, 48, 16,240, 48, 32,224, 48, 48,208, 48, 64,192, 48, 80,176, 48, 96,160, 48,112,144, 48,128,128, 48,144,112, 48,160, 96, 48,176, 80, 48,192, 64, 48,208, 48, 48,224, 32, 48,240, 16,
     64,  0,255, 64, 16,240, 64, 32,224, 64, 48,208, 64, 64,192, 64, 80,176, 64, 96,160, 64,112,144, 64,128,128, 64,144,112, 64,160, 96, 64,176, 80, 64,192, 64, 64,208, 48, 64,224, 32, 64,240, 16,
     80,  0,255, 80, 16,240, 80, 32,224, 80, 48,208, 80, 64,192, 80, 80,176, 80, 96,160, 80,112,144, 80,128,128, 80,144,112, 80,160, 96, 80,176, 80, 80,192, 64, 80,208, 48, 80,224, 32, 80,240, 16,
     96,  0,255, 96, 16,240, 96, 32,224, 96, 48,208, 96, 64,192, 96, 80,176, 96, 96,160, 96,112,144, 96,128,128, 96,144,112, 96,160, 96, 96,176, 80, 96,192, 64, 96,208, 48, 96,224, 32, 96,240, 16,
    112,  0,255,112, 16,240,112, 32,224,112, 48,208,112, 64,192,112, 80,176,112, 96,160,112,112,144,112,128,128,112,144,112,112,160, 96,112,176, 80,112,192, 64,112,208, 48,112,224, 32,112,240, 16,
    128,  0,255,128, 16,240,128, 32,224,128, 48,208,128, 64,192,128, 80,176,128, 96,160,128,112,144,128,128,128,128,144,112,128,160, 96,128,176, 80,128,192, 64,128,208, 48,128,224, 32,128,240, 16,
    144,  0,255,144, 16,240,144, 32,224,144, 48,208,144, 64,192,144, 80,176,144, 96,160,144,112,144,144,128,128,144,144,112,144,160, 96,144,176, 80,144,192, 64,144,208, 48,144,224, 32,144,240, 16,
    160,  0,255,160, 16,240,160, 32,224,160, 48,208,160, 64,192,160, 80,176,160, 96,160,160,112,144,160,128,128,160,144,112,160,160, 96,160,176, 80,160,192, 64,160,208, 48,160,224, 32,160,240, 16,
    176,  0,255,176, 16,240,176, 32,224,176, 48,208,176, 64,192,176, 80,176,176, 96,160,176,112,144,176,128,128,176,144,112,176,160, 96,176,176, 80,176,192, 64,176,208, 48,176,224, 32,176,240, 16,
    192,  0,255,192, 16,240,192, 32,224,192, 48,208,192, 64,192,192, 80,176,192, 96,160,192,112,144,192,128,128,192,144,112,192,160, 96,192,176, 80,192,192, 64,192,208, 48,192,224, 32,192,240, 16,
    208,  0,255,208, 16,240,208, 32,224,208, 48,208,208, 64,192,208, 80,176,208, 96,160,208,112,144,208,128,128,208,144,112,208,160, 96,208,176, 80,208,192, 64,208,208, 48,208,224, 32,208,240, 16,
    224,  0,255,224, 16,240,224, 32,224,224, 48,208,224, 64,192,224, 80,176,224, 96,160,224,112,144,224,128,128,224,144,112,224,160, 96,224,176, 80,224,192, 64,224,208, 48,224,224, 32,224,240, 16,
    240,  0,255,240, 16,240,240, 32,224,240, 48,208,240, 64,192,240, 80,176,240, 96,160,240,112,144,240,128,128,240,144,112,240,160, 96,240,176, 80,240,192, 64,240,208, 48,240,224, 32,240,240, 16
};

unsigned char rgba16x16[]=
{//   R   G   B   A   R   G   B   A   R   G   B   A   R   G   B   A   R   G   B   A   R   G   B   A   R   G   B   A   R   G   B   A
    255,  0,255,255,255, 16,240,255,255, 32,224,255,255, 48,208,255,255, 64,192,255,255, 80,176,255,255, 96,160,255,255,112,144,255,
    255,128,128,255,255,144,112,255,255,160, 96,255,255,176, 80,255,255,192, 64,255,255,208, 48,255,255,224, 32,255,255,240, 16,255,
    240,  0,255,255,240, 16,240,255,240, 32,224,255,240, 48,208,255,240, 64,192,255,240, 80,176,255,240, 96,160,255,240,112,144,255,
    240,128,128,255,240,144,112,255,240,160, 96,255,240,176, 80,255,240,192, 64,255,240,208, 48,255,240,224, 32,255,240,240, 16,255,
    224,  0,255,255,224, 16,240,255,224, 32,224,255,224, 48,208,255,224, 64,192,255,224, 80,176,255,224, 96,160,255,224,112,144,255,
    224,128,128,255,224,144,112,255,224,160, 96,255,224,176, 80,255,224,192, 64,255,224,208, 48,255,224,224, 32,255,224,240, 16,255,
    208,  0,255,255,208, 16,240,255,208, 32,224,255,208, 48,208,255,208, 64,192,255,208, 80,176,255,208, 96,160,255,208,112,144,255,
    208,128,128,255,208,144,112,255,208,160, 96,255,208,176, 80,255,208,192, 64,255,208,208, 48,255,208,224, 32,255,208,240, 16,255,
    192,  0,255,255,192, 16,240,255,192, 32,224,255,192, 48,208,255,192, 64,192,255,192, 80,176,255,192, 96,160,255,192,112,144,255,
    192,128,128,255,192,144,112,255,192,160, 96,255,192,176, 80,255,192,192, 64,255,192,208, 48,255,192,224, 32,255,192,240, 16,255,
    176,  0,255,255,176, 16,240,255,176, 32,224,255,176, 48,208,255,176, 64,192,255,176, 80,176,255,176, 96,160,255,176,112,144,255,
    176,128,128,255,176,144,112,255,176,160, 96,255,176,176, 80,255,176,192, 64,255,176,208, 48,255,176,224, 32,255,176,240, 16,255,
    160,  0,255,255,160, 16,240,255,160, 32,224,255,160, 48,208,255,160, 64,192,255,160, 80,176,255,160, 96,160,255,160,112,144,255,
    160,128,128,255,160,144,112,255,160,160, 96,255,160,176, 80,255,160,192, 64,255,160,208, 48,255,160,224, 32,255,160,240, 16,255,
    144,  0,255,255,144, 16,240,255,144, 32,224,255,144, 48,208,255,144, 64,192,255,144, 80,176,255,144, 96,160,255,144,112,144,255,
    144,128,128,255,144,144,112,255,144,160, 96,255,144,176, 80,255,144,192, 64,255,144,208, 48,255,144,224, 32,255,144,240, 16,255,
    128,  0,255,255,128, 16,240,255,128, 32,224,255,128, 48,208,255,128, 64,192,255,128, 80,176,255,128, 96,160,255,128,112,144,255,
    128,128,128,255,128,144,112,255,128,160, 96,255,128,176, 80,255,128,192, 64,255,128,208, 48,255,128,224, 32,255,128,240, 16,255,
    112,  0,255,255,112, 16,240,255,112, 32,224,255,112, 48,208,255,112, 64,192,255,112, 80,176,255,112, 96,160,255,112,112,144,255,
    112,128,128,255,112,144,112,255,112,160, 96,255,112,176, 80,255,112,192, 64,255,112,208, 48,255,112,224, 32,255,112,240, 16,255,
     96,  0,255,255, 96, 16,240,255, 96, 32,224,255, 96, 48,208,255, 96, 64,192,255, 96, 80,176,255, 96, 96,160,255, 96,112,144,255,
     96,128,128,255, 96,144,112,255, 96,160, 96,255, 96,176, 80,255, 96,192, 64,255, 96,208, 48,255, 96,224, 32,255, 96,240, 16,255,
     80,  0,255,255, 80, 16,240,255, 80, 32,224,255, 80, 48,208,255, 80, 64,192,255, 80, 80,176,255, 80, 96,160,255, 80,112,144,255,
     80,128,128,255, 80,144,112,255, 80,160, 96,255, 80,176, 80,255, 80,192, 64,255, 80,208, 48,255, 80,224, 32,255, 80,240, 16,255,
     64,  0,255,255, 64, 16,240,255, 64, 32,224,255, 64, 48,208,255, 64, 64,192,255, 64, 80,176,255, 64, 96,160,255, 64,112,144,255,
     64,128,128,255, 64,144,112,255, 64,160, 96,255, 64,176, 80,255, 64,192, 64,255, 64,208, 48,255, 64,224, 32,255, 64,240, 16,255,
     48,  0,255,255, 48, 16,240,255, 48, 32,224,255, 48, 48,208,255, 48, 64,192,255, 48, 80,176,255, 48, 96,160,255, 48,112,144,255,
     48,128,128,255, 48,144,112,255, 48,160, 96,255, 48,176, 80,255, 48,192, 64,255, 48,208, 48,255, 48,224, 32,255, 48,240, 16,255,
     32,  0,255,255, 32, 16,240,255, 32, 32,224,255, 32, 48,208,255, 32, 64,192,255, 32, 80,176,255, 32, 96,160,255, 32,112,144,255,
     32,128,128,255, 32,144,112,255, 32,160, 96,255, 32,176, 80,255, 32,192, 64,255, 32,208, 48,255, 32,224, 32,255, 32,240, 16,255,
     16,  0,255,255, 16, 16,240,255, 16, 32,224,255, 16, 48,208,255, 16, 64,192,255, 16, 80,176,255, 16, 96,160,255, 16,112,144,255,
     16,128,128,255, 16,144,112,255, 16,160, 96,255, 16,176, 80,255, 16,192, 64,255, 16,208, 48,255, 16,224, 32,255, 16,240, 16,255
};

int main(void)
{
	YsRawPngEncoder encoder;
	encoder.EncodeToFile("greyscale16x16.png",16,16,8,0,greyscale16x16);
	encoder.EncodeToFile("rgb16x16.png",16,16,8,2,rgb16x16);
	encoder.EncodeToFile("rgba16x16.png",16,16,8,6,rgba16x16);
	return 0;
}
