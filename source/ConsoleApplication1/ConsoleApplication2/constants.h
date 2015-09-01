const int COMPRESSION = 49;			//# of data used to make one pixel of screen
const int scrollRate = 45;			//scroll rate in pixels
const int intialFrameDelay = 4;		//delay between playAtMarker and start of music in frames.
const int playAtMarker = intialFrameDelay * scrollRate;
const int DEFAULTHIGH = -300;
const int DEFAULTLOW = 700;
RGBQUAD blue;
RGBQUAD black;
RGBQUAD red;
RGBQUAD yellow;



int tTime = 0;
int freq = 19;
float avgFreq = 19;
