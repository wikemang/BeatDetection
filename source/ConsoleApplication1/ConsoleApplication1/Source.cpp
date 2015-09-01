#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include "ReadSm.h"

using namespace std;

const string out_offset = "-2.595";
const string out_sampleStart = "75.860";
const string out_sampleLength = "9.800";
const string out_bpms = "190.000;";

const int UP = 10;
const int DOWN = 100;
const int RIGHT = 1000;
const int LEFT = 1;

void writeTitle(ofstream* myfile){
	*myfile << "#TITLE:6;" << endl;
	*myfile << "#SUBTITLE:;" << endl;
	*myfile << "#ARTIST:D_VOR;" << endl;
	*myfile << "#TITLETRANSLIT:;" << endl;
	*myfile << "#ARTISTTRANSLIT:;" << endl;
	*myfile << "#CREDIT:Lisek;" << endl;
	*myfile << "#BANNER:bn.png;" << endl;
	*myfile << "#BACKGROUND:bg.png;" << endl;
	*myfile << "#LYRICSPATH:;" << endl;
	*myfile << "#CDTITTLE:;" << endl;
	*myfile << "#MUSIC:6.ogg;" << endl;
	*myfile << "#OFFSET:" << out_offset << ";" << endl;
	*myfile << "#SAMPLESTART:" << out_sampleStart << ";" << endl;
	*myfile << "#SAMPLELENGTHL:" << out_sampleLength << ";" << endl;
	*myfile << "#SELECTABLE:YES;" << endl;
	*myfile << "#BPMS:0.000=" << out_bpms << ";" << endl;
	*myfile << "#STOPS:;" << endl;
	*myfile << "#BGCHANGES:;" << endl;

}

void writeHeader(ofstream* myfile, string difficulty, int level){
	*myfile << endl;
	*myfile << "#NOTES:" << endl;
	*myfile << "\tdance-single:" << endl;
	*myfile << "\twike:" << endl;
	*myfile << "\t" << difficulty << ":" << endl;
	*myfile << "\t" << level << ":" << endl;
	*myfile << "\t0.5,0.5,0.5,0.5,0.5:" << endl;
}

int main()
{

	/*ofstream myfile;
	myfile.open("6.sm");
	writeTitle(&myfile);
	writeHeader(&myfile, "Easy", 2);
	myfile.close();*/

	read();
	return 0;
}