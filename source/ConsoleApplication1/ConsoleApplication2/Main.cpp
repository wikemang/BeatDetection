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

const int COMPRESSION = 64;
const int playAtMarker = 200;
const int DEFAULTHIGH = -300;
const int DEFAULTLOW = 700;
RGBQUAD blue;
RGBQUAD black;
RGBQUAD red;
RGBQUAD yellow;

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

	yellow.rgbBlue = 180;
	yellow.rgbGreen = 180;
	yellow.rgbRed = 0;
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


void drawWave(FIBITMAP *dib, int high[COMPRESSION], int low[COMPRESSION], int width, int height, bool highlight[COMPRESSION]){
	for (unsigned x = 0; x < width; x++) {
		for (unsigned y = 0; y < height; y++) {
			FreeImage_SetPixelColor(dib, x, y, &blue);
		}
		if (low[x] != DEFAULTLOW && high[x] != DEFAULTHIGH){
			for (unsigned y = low[x]; y < high[x]; y++) {
				FreeImage_SetPixelColor(dib, x, y, &black);
			}
		}
		if (highlight[x] || highlight[x - 1] || highlight[x - 2]){
			for (unsigned y = 250; y < 350; y++) {
				FreeImage_SetPixelColor(dib, x, y, &yellow);
			}
		}
	}

	for (unsigned x = playAtMarker; x < playAtMarker + 4; x++) {
		for (unsigned y = 0; y < height; y++) {
			FreeImage_SetPixelColor(dib, x, y, &red);
		}
	}
}

void initHiLo(int dot[COMPRESSION], int height, int* high, int* low){
	int lowest, highest;
	lowest = DEFAULTLOW;
	highest = DEFAULTHIGH;
	for (int j = 0; j < COMPRESSION; j++){
		if (dot[j] != 0){
			if (dot[j] <= lowest)
				lowest = dot[j];
			if (dot[j] >= highest)
				highest = dot[j];
		}
	}
	*high = highest;
	*low = lowest;

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

void playFile(){
	/*Sound stuff*/
	IGraphBuilder *pGraph = NULL;
	IMediaControl *pControl = NULL;
	IMediaEvent   *pEvent = NULL;

	// Initialize the COM library.
	HRESULT hr = ::CoInitialize(NULL);
	if (FAILED(hr))
	{
		::printf("ERROR - Could not initialize COM library");
		return;
	}

	// Create the filter graph manager and query for interfaces.
	hr = ::CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
		IID_IGraphBuilder, (void **)&pGraph);
	if (FAILED(hr))
	{
		::printf("ERROR - Could not create the Filter Graph Manager.");
		return;
	}

	hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
	hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);

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
			while (true){
				HRESULT asdf = pEvent->WaitForCompletion(101, &evCode);
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
}

int main(){
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

	int* dot = new int[12000000];
	int* origdot = dot;
	for (int i = 0; i < 12000000; i++){
		dot[i] = 0;
	}
	int* highdot = new int[100000];
	int* lowdot = new int[100000];
	bool* highlight = new bool[100000];
	int* orighigh = highdot;
	int* origlow = lowdot;
	bool* orighighlight = highlight;
	i = 0;
	while (wavfile.read(buffer, 4)){
		dot[i] = (getnum(buffer[0], buffer[1]) + 32767) * 640 / 65536;
		if (abs(dot[i] - 320) < 4){
			dot[i] = 0;
		}
		i++;
		//if (i == 10000000)
		//	break;
	}
	cout << "here" << endl;
	for (int i = 0; i < 90000; i++){
		initHiLo(dot, height, &highdot[i], &lowdot[i]);
		dot += COMPRESSION;
	}
	highlight[2000] = true;
	highlight[2050] = true;
	cout << "here" << endl;
	dot = origdot;
	highdot = orighigh;
	lowdot = origlow;
	time_t timer1, timer2;
	thread first(playFile);
	Sleep(250);
	// Build the graph.
	cout << "start???" << endl;
	timer1 = clock();
	while (true){
		//cout << "-";

		dot += COMPRESSION * 50;	//Frequency: 44100
		highdot += 50;
		lowdot += 50;
		highlight += 50;

		drawWave(dib, highdot, lowdot, width, height, highlight);
		StretchDIBits(hDC, rcDest.left, rcDest.top,
			rcDest.right - rcDest.left, rcDest.bottom - rcDest.top,
			0, 0, FreeImage_GetWidth(dib), FreeImage_GetHeight(dib),
			FreeImage_GetBits(dib), FreeImage_GetInfo(dib), DIB_RGB_COLORS, SRCCOPY);
		timer2 = clock();

		time_t diff = timer2 - timer1;
		cout << diff << endl;
		Sleep(73 - diff);
		timer1 = clock();
	}
	cout << "end???" << endl;


	delete(origdot);
	delete(orighigh);
	delete(origlow);
	delete(orighighlight);
	FreeImage_Unload(dib);

	return 0;
}