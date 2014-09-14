#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "FreeImage.h"

using namespace std;

int main(){
	/*
	ifstream infile("../../../sample-data.txt");
	string line;
	double left, right;
	vector<double> avg;
	getline(infile, line);
	while (line.substr(0, 6) != "[-inf]"){
		getline(infile, line);
	}
	for (int i = 0; i < 4000; i++){
		getline(infile, line);
		size_t loc = line.find("\t");
		if (loc != string::npos) {
			left = atof(line.substr(0, loc).c_str());
			right = atof(line.substr(loc + 1).c_str());
		}
		if (i == 3000){
			cout << loc << " " << endl;
			cout << line.substr(0, loc).c_str() << " " << line.substr(loc + 1).c_str() << endl;
			cout << left << " " << right << endl;
		}
		avg.push_back((left + right) / 2);
	}*/
	FIBITMAP *dib = FreeImage_Load(FIF_BMP, "asdf.png", BMP_DEFAULT);
	// Calculate the number of bytes per pixel (3 for 24-bit or 4 for 32-bit)
	cout << "adsf" << endl;
	int bytespp = FreeImage_GetLine(dib) / 25362;
	cout << "adsf" << endl;
	cout << FreeImage_GetHeight(dib) << " " << 25362 << " " << bytespp << endl;
	for (unsigned y = 0; y < FreeImage_GetHeight(dib); y++) {
		BYTE *bits = FreeImage_GetScanLine(dib, y);
		for (unsigned x = 0; x < 25362; x++) {
			// Set pixel color to green with a transparency of 128
			/*bits[FI_RGBA_RED] = 0;
			bits[FI_RGBA_GREEN] = 255;
			bits[FI_RGBA_BLUE] = 0;
			bits[FI_RGBA_ALPHA] = 128;
			// jump to next pixel
			bits += bytespp;*/
		}
	}

	FreeImage_Unload(dib);
	return 0;
}