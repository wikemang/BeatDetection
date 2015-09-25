#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include "FreeImage.h"
#include <time.h>
#include <thread> 
#include "constants.h"
#include "SimpleBeatDetection.h"
#include "fft/fftw3.h"

#include <dshow.h>
#include <cstdio>

using namespace std;
bool finishedPlaying;


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

//Draws the waveform for 1 frame (screen's worth) of data.
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
			for (unsigned y = 270; y < 350; y++) {
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

//Initialize 1 pixel of waveform data.
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

//Get numerical value of 2 bytes.
short getnum(char lsb, char msb){
	return (msb << 8) | (lsb & 255);
}




// For IID_IGraphBuilder, IID_IMediaControl, IID_IMediaEvent
#pragma comment(lib, "strmiids.lib")

//const wchar_t* filePath = L"C:\\Program Files (x86)\\Stepmania 5\\Songs\\kpop\\sistar\\sistar.wav";
const wchar_t* filePath = L"C:\\Users\\Mike\\Desktop\\ddr\\DDr\\paradisecut.wav";

//TODO: change initialization to be based off compression and size of file.
int* dot = new int[16777216];	//2^24 likely upper bound. Can support files that are roughly 6.3 minutes and have a frequency of 44100.
int* tempDot = new int[16777216];	//2^24

int* highdot = new int[10000000];
int* lowdot = new int[10000000];
bool* highlight = new bool[10000000];
FIBITMAP *dib = FreeImage_Load(FIF_PNG, "../../../template.png", PNG_DEFAULT);	//Default animation area
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
	double* data = (double*)fftw_malloc(sizeof(double) * 16777216);
	double* dataTemp = (double*)fftw_malloc(sizeof(double) * 16777216);
	fftw_complex* dataF = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * 16777216);
	initColors();
	ifstream wavfile(filePath, ios::binary);
	char buffer[4];
	string header = "";
	for (int j = 0; j < 11; j++){	//Wav header
		wavfile.read(buffer, 4);
		string buff(buffer, 4);
		header += buff;
	}


	rcDest.left = 0;
	rcDest.top = 0;
	rcDest.right = 1000;
	rcDest.bottom = 640;

	SetStretchBltMode(hDC, COLORONCOLOR);

	for (int i = 0; i < 16777216; i++){
		data[i] = 0;
	}
	
	int dataCount = 0;
	while (wavfile.read(buffer, 4)){	//TODO: work with both channels
		data[dataCount] = getnum(buffer[0], buffer[1]);

		dot[dataCount] = (data[dataCount] + 32767) * 640 / 65536;

		dataCount++;
	}
	int* origDot = dot;
	
	for (int i = 0; i < dataCount / COMPRESSION; i++){
		initHiLo(dot, height, &highdot[i], &lowdot[i]);	//Initialize wave drawing
		highlight[i] = false;
		dot += COMPRESSION;
	}
	dot = origDot;
	bool beats[4050];
	bool tempBeats[4050];
	for (int i = 0; i < 4050; i++){
		beats[i] = false;
		tempBeats[i] = false;
	}

	fftw_plan fft = fftw_plan_dft_r2c_1d(dataCount, data, dataF, FFTW_ESTIMATE);
	fftw_plan ifft = fftw_plan_dft_c2r_1d(dataCount, dataF, dataTemp, FFTW_ESTIMATE);
	
	const int totalBands = 6;
	int freqBandStartIndex[totalBands + 1];

	fftw_execute(fft);

	_int64 total = 0;
	for (int i = 0; i < dataCount / 2; i++){
		total += (dataF[i][0] * dataF[i][0] + dataF[i][1] * dataF[i][1]) / 100000000;	//Avoid overflow
	}
	_int64 current = 0;
	int i = 0;
	for (int j = 0; j < totalBands; j++){
		while (current < total * j / totalBands){
			current += (dataF[i][0] * dataF[i][0] + dataF[i][1] * dataF[i][1]) / 100000000;
			i++;
		}
		freqBandStartIndex[j] = i;
	}
	freqBandStartIndex[totalBands] = dataCount / 2;

	for (int freqBand = 0; freqBand < totalBands; freqBand++){
		cout << "Starting FFT #" << freqBand << "..." << endl;
		fftw_execute(fft);

		for (int i = 0; i < freqBandStartIndex[freqBand]; i++){
			dataF[i][0] = 0;
			dataF[i][1] = 0;
		}
		for (int i = freqBandStartIndex[freqBand + 1]; i < dataCount; i++){
			dataF[i][0] = 0;
			dataF[i][1] = 0;
		}
		cout << "Starting IFFT #" << freqBand << "..." << endl;
		fftw_execute(ifft);


		//Debug file writing
		string fileName = "modified" + to_string(freqBand) + ".wav";
		cout << "Writing File " << fileName << "..." << endl;
		ofstream oFile(fileName, ios::out | ios::binary);
		oFile.write(header.c_str(), 44);
		__int16 char2;
		for (int i = 0; i < dataCount; i++){
			char2 = dataTemp[i] / dataCount;
			char maxChar = 255;
			oFile.put((char)(char2 % 256));
			oFile.put((char)(char2 >> 8));

			char2 = dataTemp[i] / dataCount;
			oFile.put((char)(char2 % 256));
			oFile.put((char)(char2 >> 8));

			tempDot[i] = (char2 + 32767) * 640 / 65536;
		}
		oFile.close();

		//First band tends to contain a lot of very low frequency sound that can't be heard, so we exclude it from beat detection.
			BeatDetector* beatDetector = new BeatDetector();
			beatDetector->detectBeat(tempBeats, tempDot);	//Writes the beats
			for (int i = 0; i < 4050; i++){
				beats[i] = beats[i] || tempBeats[i];
			}

			delete (beatDetector);
	}

	for (int i = 4049; i > 4; i--){
		if (beats[i]){
			if (beats[i - 1] || beats[i - 2] || beats[i - 3] || beats[i - 4])
				beats[i] = false;
		}
	}

	for (int i = 0; i < 4050; i++){
		if (beats[i]){
			for (int j = 0; j < 9; j++){
				highlight[i * 20 + 5 + j] = true;
			}
		}
	}





	fftw_destroy_plan(fft);
	fftw_destroy_plan(ifft);
	fftw_free(data);
	fftw_free(dataF);

	
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
				
				if (timer2 - timer1 - tTime <= -20){
					if (eventTime <= avgFreq + 1)
						eventTime += 1;
				}
				if (timer2 - timer1 - tTime >= 20){
					if (eventTime >= avgFreq - 1)
						eventTime -= 1;
				}
				avgFreq = (avgFreq * 39 + eventTime) / 40.0;
				//cout << eventTime << "\t" << -(timer2 - timer1 - tTime) << endl;
				HRESULT res = pEvent->WaitForCompletion(eventTime, &evCode);	//Normal is 20 but the cpu has really unpredictable run times.
				tTime += 50;
				timer2 = clock();


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
	finishedPlaying = true;
}

int main(){
	
	readWav();
	cout << "Ready. Press enter to continue." << endl;
	string temp;
	cin >> temp;
	//return 0;
	thread first(playFile);
	finishedPlaying = false;
	while (!finishedPlaying){
		Sleep(1000);
	}
	cout << "end2???" << endl;


	delete dot;
	delete highdot;
	delete lowdot;
	delete highlight;

	return 0;
}