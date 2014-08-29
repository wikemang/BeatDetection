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
	FIBITMAP *dib = FreeImage_Load(FIF_BMP, "asdf.bmp", BMP_DEFAULT);
	FreeImage_Unload(dib);
	return 0;
}