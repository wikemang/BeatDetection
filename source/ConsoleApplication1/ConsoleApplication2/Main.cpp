#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "CImg.h"

using namespace std;
using namespace cimg_library;

int main(){
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
	}
	CImg<unsigned char> visu(4000, 200, 1, 3, 0);
	const unsigned char red[] = { 255, 0, 0 }, green[] = { 0, 255, 0 }, blue[] = { 0, 0, 255 };
	CImgDisplay draw_disp(visu, "Intensity profile");
	visu.fill(0);
	//visu.fill(0).draw_graph(image.get_crop(0, y, 0, 0, image.width() - 1, y, 0, 0), red, 1, 1, 0, 255, 0);
	//visu.draw_graph(image.get_crop(0, y, 0, 1, image.width() - 1, y, 0, 1), green, 1, 1, 0, 255, 0);
	//visu.draw_graph(image.get_crop(0, y, 0, 2, image.width() - 1, y, 0, 2), blue, 1, 1, 0, 255, 0).display(draw_disp);
	for (int i = 0; i < avg.size(); i++){
		visu.draw_point(i, int(avg[i] + 175), red).display(draw_disp);
		cout << avg[i] << endl;
	}
	visu.save("asdf.bmp", -1, 6);
	return 0;
}