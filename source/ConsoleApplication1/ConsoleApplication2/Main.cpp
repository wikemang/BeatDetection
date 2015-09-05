#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include "FreeImage.h"
#include <time.h>
#include <thread> 
#include "constants.h"
#include "SimpleBeatDetection.h"

#include <dshow.h>
#include <cstdio>

using namespace std;


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
//Get numerical value of .wav word
short getnum(char lsb, char msb){
	short num = (msb << 8) | lsb;
	return num;
}




// For IID_IGraphBuilder, IID_IMediaControl, IID_IMediaEvent
#pragma comment(lib, "strmiids.lib")
// Obviously change this to point to a valid mp3 file.
const wchar_t* filePath = L"C:\\Program Files (x86)\\Stepmania 5\\Songs\\Test\\testsong\\6.wav";


//TODO: change initialization to be based off compression and size of file.
int* dot = new int[12000000];
int* highdot = new int[100000];
int* lowdot = new int[100000];
bool* highlight = new bool[100000];
FIBITMAP *dib = FreeImage_Load(FIF_PNG, "../../../asdf.png", PNG_DEFAULT);	//Default animation area
int width = FreeImage_GetWidth(dib);
int height = FreeImage_GetHeight(dib);
HDC hDC = GetDC(NULL);
RECT rcDest;

void drawFrame(){

	highdot += scrollRate;
	lowdot += scrollRate;
	highlight += scrollRate;

	drawWave(dib, highdot, lowdot, width, height, highlight);
	StretchDIBits(hDC, rcDest.left, rcDest.top,
		rcDest.right - rcDest.left, rcDest.bottom - rcDest.top,
		0, 0, FreeImage_GetWidth(dib), FreeImage_GetHeight(dib),
		FreeImage_GetBits(dib), FreeImage_GetInfo(dib), DIB_RGB_COLORS, SRCCOPY);
}

void readWav(){
	initColors();
	ifstream wavfile(filePath, ios::binary);
	char buffer[4];
	for (int j = 0; j < 22; j++){	//Wav header
		wavfile.read(buffer, 2);
	}


	rcDest.left = 0;
	rcDest.top = 0;
	rcDest.right = 1000;
	rcDest.bottom = 640;

	SetStretchBltMode(hDC, COLORONCOLOR);


	int i = 0;
	while (wavfile.read(buffer, 4)){	//TODO: work with both channels
		dot[i] = (getnum(buffer[0], buffer[1]) + 32767) * 640 / 65536;
		i++;
	}
	int* origDot = dot;
	for (int i = 0; i < 90000; i++){
		initHiLo(dot, height, &highdot[i], &lowdot[i]);	//Initialize wave drawing
		highlight[i] = false;
		dot += COMPRESSION;
	}
	BeatDetector* beatDetector = new BeatDetector();
	dot = origDot;

	bool beats[100000];
	beatDetector->detectBeat(beats, dot);	//Writes the beats
	cout << "END" << endl;
	for (int i = 0; i < 4050; i++){
		if (beats[i]){
			for (int j = 0; j < 9; j++){
				highlight[i * 20 + 5 + j] = true;
			}
		}
	}
	delete(beatDetector);
}

void playFile(){
	time_t timer1, timer2, timer3;
	timer3 = clock();
	/*Sound stuff*/
	IGraphBuilder *pGraph = NULL;
	IMediaControl *pControl = NULL;
	IMediaEvent   *pEvent = NULL;

	HRESULT hr = ::CoInitialize(NULL);
	if (FAILED(hr))
	{
		return;
	}

	hr = ::CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
		IID_IGraphBuilder, (void **)&pGraph);
	if (FAILED(hr))
	{
		return;
	}

	hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
	hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);

	hr = pGraph->RenderFile(filePath, NULL);
	if (SUCCEEDED(hr))
	{
		// Run the graph.
		hr = pControl->Run();
		Sleep(intialFrameDelay * 50);
		timer1 = clock();
		timer2 = clock();
		if (SUCCEEDED(hr))
		{
			// Wait for completion.
			long evCode;
			while (true){
				drawFrame();
				//cout << "Time diff: " << timer2 - timer1 << "tTime: " << tTime << endl;
				if (timer2 - timer1 - tTime <= -20){
					if (freq <= avgFreq + 1.5)
						freq += 1;
				}
				if (timer2 - timer1 - tTime >= 20){
					if (freq >= avgFreq - 1.5)
						freq -= 1;
				}
				avgFreq = (avgFreq * 39 + freq) / 40.0;
				tTime += 50;
				timer2 = clock();

				HRESULT res = pEvent->WaitForCompletion(freq, &evCode);	//Normal is 20 but the cpu has really unpredictable run times.


				if (res == VFW_E_WRONG_STATE)
					break;
			}
			cout << "end???" << endl;
		}
	}


	pEvent->Release();
	pControl->Release();
	pGraph->Release();
	::CoUninitialize();
}

int main(){
	readWav();
	thread first(playFile);
	while (true){
		Sleep(100);
	}
	cout << "end???" << endl;


	delete dot;
	delete highdot;
	delete lowdot;
	delete highlight;

	return 0;
}