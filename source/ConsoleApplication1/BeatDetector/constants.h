#ifndef CONSTANTS_H
#define CONSTANTS_H

const int COMPRESSION = 49;			//# of data used to make one pixel of screen
const int scrollRate = 45;			//scroll rate in pixels
const int beatLength = 44100 / scrollRate;	//Consider selecting constant optimal beat length instead.
const int intialFrameDelay = 4;		//delay between playAtMarker and start of music in frames.
const int playAtMarker = intialFrameDelay * scrollRate;
const int DEFAULTHIGH = -300;
const int DEFAULTLOW = 700;
RGBQUAD backgroundColor;
RGBQUAD foregroundColor;
RGBQUAD markerColor;
RGBQUAD beatColor;

const int maxDataSize = 16777216;	//2^24 likely upper bound. Can support files that are roughly 6.3 minutes and have a frequency of 44100.
//This program is meant to process music files between 1-4 minutes.


const wchar_t* defaultFile = L"C:\\Users\\Mike\\Desktop\\ddr\\DDr\\LTheme2.wav";


int tTime = 0;
int eventTime = 19;
float avgFreq = 19;
#endif