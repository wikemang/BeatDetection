#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>
#include "FreeImage.h"
#include <time.h>
#include "PlayFile.h"
#include <thread> 

#include <dshow.h>
#include <cstdio>

using namespace std;

const int compression = 64;
const int playAtMarker = 200;
RGBQUAD blue;
RGBQUAD black;
RGBQUAD red;

void initColors(){
	blue.rgbBlue = 255;
	blue.rgbGreen = 100;
	blue.rgbRed = 100;
	blue.rgbReserved = 125;

	black.rgbBlue = 0;
	black.rgbGreen = 0;
	black.rgbRed = 0;
	black.rgbReserved = 125;

	red.rgbBlue = 80;
	red.rgbGreen = 80;
	red.rgbRed = 200;
	red.rgbReserved = 125;
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
	for (unsigned x = 0; x < width; x++) {
		for (unsigned y = 0; y < height; y++) {
			FreeImage_SetPixelColor(dib, x, y, &blue);
			if (y == dot[x] && dot[x] != 0){
				FreeImage_SetPixelColor(dib, x, y, &black);
				FreeImage_SetPixelColor(dib, x - 1, y, &black);
				FreeImage_SetPixelColor(dib, x, y - 1, &black);
				FreeImage_SetPixelColor(dib, x - 1, y - 1, &black);
			}
		}
	}
}


void drawWave(FIBITMAP *dib, int dot[100000], int width, int height){
	int i = 0;
	int lowest, highest;
	for (unsigned x = 0; x < width; x++) {
		i += compression;
		lowest = 700;
		highest = -300;
		for (int j = 0; j < compression; j++){
			if (dot[i + j] != 0){
				if (dot[i + j] <= lowest)
					lowest = dot[i + j];
				if (dot[i + j] >= highest)
					highest = dot[i + j];
			}
		}

		for (unsigned y = 0; y < height; y++) {
			FreeImage_SetPixelColor(dib, x, y, &blue);
		}
		if (lowest != 700 && highest != -300){
			for (unsigned y = lowest; y < highest; y++) {
				FreeImage_SetPixelColor(dib, x, y, &black);
			}
		}
	}

	for (unsigned x = playAtMarker; x < playAtMarker + 4; x++) {
		for (unsigned y = 0; y < height; y++) {
			FreeImage_SetPixelColor(dib, x, y, &red);
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

short getnum(char lsb, char msb){
	short num = (msb << 8) | lsb;
	return num;
}


// For IID_IGraphBuilder, IID_IMediaControl, IID_IMediaEvent
#pragma comment(lib, "strmiids.lib")
// Obviously change this to point to a valid mp3 file.
const wchar_t* filePath = L"../../../sistar.wav";

int main(){
	/*Sound stuff*/
	IGraphBuilder *pGraph = NULL;
	IMediaControl *pControl = NULL;
	IMediaEvent   *pEvent = NULL;

	// Initialize the COM library.
	HRESULT hr = ::CoInitialize(NULL);
	if (FAILED(hr))
	{
		::printf("ERROR - Could not initialize COM library");
		return 0;
	}

	// Create the filter graph manager and query for interfaces.
	hr = ::CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
		IID_IGraphBuilder, (void **)&pGraph);
	if (FAILED(hr))
	{
		::printf("ERROR - Could not create the Filter Graph Manager.");
		return 0;
	}

	hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
	hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);




	initColors();
	ifstream wavfile("../../../sistar.wav", ios::binary);
	char buffer[4];
	for (int j = 0; j < 22; j++){	//Wav header
		wavfile.read(buffer, 2);
	}

	FIBITMAP *dib = FreeImage_Load(FIF_PNG, "../../../asdf.png", PNG_DEFAULT);
	int width = FreeImage_GetWidth(dib);
	int height = FreeImage_GetHeight(dib);

	HDC hDC = GetDC(NULL);
	RECT rcDest;
	rcDest.left = 0;
	rcDest.top = 0;
	rcDest.right = 1000;
	rcDest.bottom = 640;

	SetStretchBltMode(hDC, COLORONCOLOR);


	int i = 0;
	while (wavfile.read(buffer, 4)){	//Skip buffer to...
		break;
		i++;
		if (abs(getnum(buffer[0], buffer[1])) >= 15){
			cout << "break at " << i << endl;
			break;
		}
	}

	int* dot = new int[1200000];
	for (int i = 0; i < 1200000; i++){
		dot[i] = 0;
	}
	i = 0;
	while (wavfile.read(buffer, 4)){
		dot[i] = (getnum(buffer[0], buffer[1]) + 32767) * 640 / 65536;
		if (abs(dot[i] - 320) < 4){
			dot[i] = 0;
		}
		i++;
		if (i == 1000000)
			break;
	}
	DWORD timer1, timer2;
	//thread first(playFile);
	// Build the graph.
	cout << "start???" << endl;
	hr = pGraph->RenderFile(filePath, NULL);
	if (SUCCEEDED(hr))
	{
		// Run the graph.
		cout << "step2" << endl;
		hr = pControl->Run();
		if (SUCCEEDED(hr))
		{
			// Wait for completion.
			long evCode;
			HRESULT asdf = pEvent->WaitForCompletion(450, &evCode);
			timer1 = GetTickCount();
			while (true){
				//cout << "-";

				dot += compression * 69;	//Frequency: 44100
				drawWave(dib, dot, width, height);
				StretchDIBits(hDC, rcDest.left, rcDest.top,
					rcDest.right - rcDest.left, rcDest.bottom - rcDest.top,
					0, 0, FreeImage_GetWidth(dib), FreeImage_GetHeight(dib),
					FreeImage_GetBits(dib), FreeImage_GetInfo(dib), DIB_RGB_COLORS, SRCCOPY);
				timer2 = GetTickCount();

				DWORD diff = timer2 - timer1;
				timer1 = GetTickCount();
				cout << CLOCKS_PER_SEC << endl;
				HRESULT asdf = pEvent->WaitForCompletion(101 - diff, &evCode);
				if (asdf == VFW_E_WRONG_STATE)
					break;
			}
			cout << "end???" << endl;

			// Note: Do not use INFINITE in a real application, because it
			// can block indefinitely.
		}
	}


	// Clean up in reverse order.
	pEvent->Release();
	pControl->Release();
	pGraph->Release();
	::CoUninitialize();

	delete(dot);
	FreeImage_Unload(dib);

	return 0;
}