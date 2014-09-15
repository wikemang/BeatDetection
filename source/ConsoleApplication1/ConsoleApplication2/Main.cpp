#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>
#include "FreeImage.h"

using namespace std;

short getnum(char lsb, char msb){
	short num = (msb << 8) | lsb;
	return num;
}

void fillcurve(int dots[1000]){
	int silence_duration = 0;
	int previous_dot = 0;
	int inc;
	for (int i = 0; i < 1000; i++){
		if (dots[i] != 0){
			if (silence_duration == 0){
				previous_dot = dots[i];
				continue;
			}
			inc = (dots[i] - previous_dot) / silence_duration;
			for (int j = i - silence_duration; j < i; j++){
				previous_dot += inc;
				dots[j] = previous_dot;
			}
			previous_dot = dots[i];
			silence_duration = 0;
		}
		else{
			silence_duration++;
		}
	}
}

void drawCurve(FIBITMAP *dib, int dot[1000], int width, int height){
	RGBQUAD blue;
	blue.rgbBlue = 255;
	blue.rgbGreen = 100;
	blue.rgbRed = 100;
	blue.rgbReserved = 125;
	RGBQUAD black;
	black.rgbBlue = 0;
	black.rgbGreen = 0;
	black.rgbRed = 0;
	black.rgbReserved = 125;
	for (unsigned x = 0; x < width; x++) {
		for (unsigned y = 0; y < height; y++) {
			FreeImage_SetPixelColor(dib, x, y, &blue);
			if (y == dot[x] && dot[x] != 0){
				FreeImage_SetPixelColor(dib, x, y, &black);
				FreeImage_SetPixelColor(dib, x-1, y, &black);
				FreeImage_SetPixelColor(dib, x, y-1, &black);
				FreeImage_SetPixelColor(dib, x-1, y-1, &black);
			}
		}
	}
}

int countzeros(int dots[1000]){
	int count = 0;
	for (int i = 0; i < 1000; i++)
	if (dots[i] == 0)
		count++;
	return count;
}

int main(){
	ifstream wavfile("../../../sistar.wav", ios::binary);
	int i = 0;
	char buffer[4];
	for (int j = 0; j < 22; j++){	//Wav header
		wavfile.read(buffer, 2);
	}

	FIBITMAP *dib = FreeImage_Load(FIF_PNG, "../../../asdf.png", PNG_DEFAULT);
	int width = FreeImage_GetWidth(dib);
	int height = FreeImage_GetHeight(dib);

	/*
	// Calculate the number of bytes per pixel (3 for 24-bit or 4 for 32-bit)
	int bytespp = FreeImage_GetLine(dib) / FreeImage_GetWidth(dib);
	for (unsigned y = 0; y < height; y++) {
		BYTE *bits = FreeImage_GetScanLine(dib, y);
		for (unsigned x = 0; x < width; x++) {
			// Set pixel color to green with a transparency of 128
			bits[FI_RGBA_RED] = 0;
			bits[FI_RGBA_GREEN] = 255;
			bits[FI_RGBA_BLUE] = 0;
			bits[FI_RGBA_ALPHA] = 128;
			// jump to next pixel
			bits += bytespp;
		}
	}*/


	while (wavfile.read(buffer, 4)){	//Skip buffer to...
		i++;
		if (abs(getnum(buffer[0], buffer[1])) >= 200){
			cout << "break at " << i << endl;
			break;
		}
		if (i == 150000)
			break;
	}

	int dot[1000];
	i = 0;
	while (wavfile.read(buffer, 4)){
		dot[i] = (getnum(buffer[0], buffer[1]) + 32767) * 640 / 65536;
		if (abs(dot[i] - 320) < 4){
			dot[i] = 0;
		}
		i++;
		if (i == 1000)
			break;
	}
	//fillcurve(dot);
	cout << countzeros(dot) << endl;


	/*
	for (unsigned x = 0; x < width; x++) {
		for (unsigned y = 0; y < height; y++) {
			FreeImage_SetPixelColor(dib, x, y, &blue);
			if (y == dot[x] && dot[x] != 0)
				FreeImage_SetPixelColor(dib, x, y, &black);
		}
	}*/
	drawCurve(dib, dot, width, height);


	HDC hDC = GetDC(NULL);
	RECT rcDest;
	rcDest.left = 0;
	rcDest.top = 0;
	rcDest.right = 1000;
	rcDest.bottom = 640;

	SetStretchBltMode(hDC, COLORONCOLOR);
	StretchDIBits(hDC, rcDest.left, rcDest.top,
		rcDest.right - rcDest.left, rcDest.bottom - rcDest.top,
		0, 0, FreeImage_GetWidth(dib), FreeImage_GetHeight(dib),
		FreeImage_GetBits(dib), FreeImage_GetInfo(dib), DIB_RGB_COLORS, SRCCOPY);

	FreeImage_Unload(dib);

	return 0;
}