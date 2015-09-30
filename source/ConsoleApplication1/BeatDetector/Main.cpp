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


void initColors(){
	//blue-purple
	backgroundColor.rgbBlue = 255;
	backgroundColor.rgbGreen = 100;
	backgroundColor.rgbRed = 100;
	backgroundColor.rgbReserved = 125;

	//black
	foregroundColor.rgbBlue = 0;
	foregroundColor.rgbGreen = 0;
	foregroundColor.rgbRed = 0;
	foregroundColor.rgbReserved = 125;

	//red
	markerColor.rgbBlue = 80;
	markerColor.rgbGreen = 80;
	markerColor.rgbRed = 200;
	markerColor.rgbReserved = 125;

	//teal
	beatColor.rgbBlue = 180;
	beatColor.rgbGreen = 180;
	beatColor.rgbRed = 0;
	beatColor.rgbReserved = 125;
}

//Draws current waveform onto screen.
void drawWave(FIBITMAP *dib, int high[COMPRESSION], int low[COMPRESSION], int width, int height, bool highlight[COMPRESSION]){
	for (unsigned x = 0; x < width; x++) {
		for (unsigned y = 0; y < height; y++) {
			FreeImage_SetPixelColor(dib, x, y, &backgroundColor);
		}
		if (low[x] != DEFAULTLOW && high[x] != DEFAULTHIGH){
			for (unsigned y = low[x]; y < high[x]; y++) {
				FreeImage_SetPixelColor(dib, x, y, &foregroundColor);
			}
		}
		if (highlight[x] || highlight[x - 1] || highlight[x - 2]){
			for (unsigned y = 270; y < 350; y++) {
				FreeImage_SetPixelColor(dib, x, y, &beatColor);
			}
		}
	}

	for (unsigned x = playAtMarker; x < playAtMarker + 4; x++) {
		for (unsigned y = 0; y < height; y++) {
			FreeImage_SetPixelColor(dib, x, y, &markerColor);
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
const wchar_t* fileName = defaultFile;	//Format is needed for pGraph

int* dot = new int[maxDataSize];	//Raw waveform data
int* tempDot;						//Intermediate waveform data

//Variables used to draw waveform on screen
int* highdot;
int* lowdot;
bool* highlight;

FIBITMAP *dib = FreeImage_Load(FIF_PNG, "../../../template.png", PNG_DEFAULT);	//Default animation area
int width = FreeImage_GetWidth(dib);
int height = FreeImage_GetHeight(dib);
HDC hDC = GetDC(NULL);
RECT rcDest;

//Draws the waveform for 1 frame (screen's worth) of data.
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

//TODO: refactor this function.
//Reads the file and initializes waveform for drawing. Also preforms beat detection as part of waveform initialization.
void readWav(){
	double* data = (double*)fftw_malloc(sizeof(double) * maxDataSize);	//Change initialization to be based off of input file. Perhaps 2 passes.
	double* dataTemp = (double*)fftw_malloc(sizeof(double) * maxDataSize);
	fftw_complex* dataF = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * maxDataSize);
	initColors();
	ifstream wavfile(fileName, ios::binary);
	char buffer[4];
	string header = "";
	for (int j = 0; j < 11; j++){	//Skip .wav header
		wavfile.read(buffer, 4);
		string buff(buffer, 4);
		header += buff;
	}


	SetStretchBltMode(hDC, COLORONCOLOR);

	for (int i = 0; i < maxDataSize; i++){
		data[i] = 0;
	}
	
	int dataCount = 0;	//Number of data points in the waveform.
	while (wavfile.read(buffer, 4)){	//TODO: work with both channels
		data[dataCount] = getnum(buffer[0], buffer[1]);

		dot[dataCount] = (data[dataCount] + 32767) * 640 / 65536;

		dataCount++;
	}

	tempDot = new int[dataCount];
	highdot = new int[dataCount];
	lowdot = new int[dataCount];
	highlight = new bool[dataCount];

	int* origDot = dot;
	for (int i = 0; i < dataCount / COMPRESSION; i++){
		initHiLo(dot, height, &highdot[i], &lowdot[i]);	//Initialize wave drawing
		highlight[i] = false;
		dot += COMPRESSION;
	}
	dot = origDot;

	/*Initialize beat data.*/
	int beatCount = dataCount / beatLength + 1;
	bool* beats = new bool[beatCount];
	bool* tempBeats = new bool[beatCount];
	for (int i = 0; i < beatCount; i++){
		beats[i] = false;
		tempBeats[i] = false;
	}

	//Creates fft plans.
	fftw_plan fft = fftw_plan_dft_r2c_1d(dataCount, data, dataF, FFTW_ESTIMATE);
	fftw_plan ifft = fftw_plan_dft_c2r_1d(dataCount, dataF, dataTemp, FFTW_ESTIMATE);

	fftw_execute(fft);
	const int totalBands = 6;
	/*Calculate indices for separating data by frequency into several frequency bands*/
	int freqBandStartIndex[totalBands + 1];
	_int64 total = 0;
	for (int i = 0; i < dataCount / 2; i++){
		total += (dataF[i][0] * dataF[i][0] + dataF[i][1] * dataF[i][1]) / 100000000;	//Estimation used to avoid overflow
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


	/*This block first filters out all data not in the current frequency band.
	Beats are then detected from the IFFT of the current frequency band
	*/
	for (int freqBand = 0; freqBand < totalBands; freqBand++){
		cout << "Starting FFT #" << freqBand << "..." << endl;
		fftw_execute(fft);	//TODO: Copy into dataF instead.

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

		//Detects beats of the current frequency band, and puts the beats onto the total beats array.
		cout << "Detecting Beats of " << fileName << "..." << endl;
		BeatDetector* beatDetector = new BeatDetector();
		beatDetector->detectBeat(tempBeats, tempDot, beatCount);	//Writes the beats
		for (int i = 0; i < beatCount; i++){
			beats[i] = beats[i] || tempBeats[i];
		}

		delete (beatDetector);
	}

	//Consecutively detected beats are likely just one beat at the beginning of the beats.
	for (int i = beatCount - 1; i > 4; i--){
		if (beats[i]){
			if (beats[i - 1] || beats[i - 2] || beats[i - 3] || beats[i - 4])
				beats[i] = false;
		}
	}

	for (int i = 0; i < beatCount; i++){
		if (beats[i]){
			for (int j = 0; j < 9; j++){
				highlight[i * 20 + 5 + j] = true;
			}
		}
	}
	delete(beats);
	delete(tempBeats);





	fftw_destroy_plan(fft);
	fftw_destroy_plan(ifft);
	fftw_free(data);
	fftw_free(dataF);

	
}

void playFile(){

	int* origHighDot = highdot;
	int* origLowDot = lowdot;
	bool* origHighlight = highlight;

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

	hr = pGraph->RenderFile(fileName, NULL);
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
				/*It appears that pEvent->WaitForCompletion takes a random amount of time to execute, having weak correlations to its eventTime parameter.
				To compensate for this, a "feedback" loop is implemented, without and rates considered. Program is still haveing syncing issues between threads.*/
				
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

	delete dot;
	delete origHighDot;
	delete origLowDot;
	delete origHighlight;

	pEvent->Release();
	pControl->Release();
	pGraph->Release();
	::CoUninitialize();
}

int main(int argc, char* argv[]){
	if (argc > 1){
		string temp = argv[1];
		fileName = wstring(temp.begin(), temp.end()).c_str();
		int a = 0;
		1 / a;
	}
	
	readWav();
	cout << "Ready. Press enter to continue." << endl;
	string temp;
	cin >> temp;
	//return 0;
	rcDest.left = 0;
	rcDest.top = 0;
	rcDest.right = 1000;
	rcDest.bottom = 640;
	thread first(playFile);
	first.join();
	cout << "end2???" << endl;


	return 0;
}