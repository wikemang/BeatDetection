#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <list>
#include <vector>
using namespace std;

class Note{
	int _arrow;
	int _time;	//in ms
	int _duration;

public:
	Note(int arrow, int time, int duration);
	Note(int arrow, int time);
	int getTime();
	int getArrow();
};

Note::Note(int arrow, int time, int duration){
	Note(arrow, time);
	_duration = duration;
}

Note::Note(int arrow, int time){
	_arrow = arrow;
	_time = time;
}

int Note::getTime(){
	return _time;
}

int Note::getArrow(){
	return _arrow;
}


list<Note*> songNotes;
vector<string> barNotes;
int SM_MUSIC = 0;
int SM_OFFSET = 1;
int SM_START = 2;
int SM_BPM = 3;
int SM_NOTE = 4;


string SM_CONST[5] = { "#MUSIC",
"#OFFSET",
"#SAMPLESTART",
"#BPMS",
"#NOTES" };

string music_file;
float offset;
float start;
int bpm;
float spb;

double songTime = 0;

int stepToInt(string step){
	int stepInt = 0;
	for (int i = 0; i < 4; i++){
		if (step[i] == '1'){
			stepInt += (1 << (3 - i));
		}
	}
	return stepInt;
}

string intToStep(int step){
	if (step == 1){
		return "right";
	}
	if (step == 2){
		return "up";
	}
	if (step == 4){
		return "down";
	}
	if (step == 8){
		return "left";
	}
	return "unavi";
}

void storeSteps(ifstream& wavfile){
	songTime = offset + spb * 4;
	string buffer;
	for (int i = 0; i < 5; i++){
		getline(wavfile, buffer);
	}

	while (getline(wavfile, buffer)){
		if (buffer == ";" || buffer == ","){
			for (int i = 0; i < barNotes.size(); i++){
				if (stepToInt(barNotes[i]))
					songNotes.push_back(new Note(stepToInt(barNotes[i]), (int)(songTime * 1000)));
				songTime += spb * 4 / barNotes.size();
			}
			barNotes.clear();
			if (buffer == ";")
				break;
		}
		else{
			barNotes.push_back(buffer);
		}
	}
}

int read()
{
	string file = "C:\\Program Files (x86)\\Stepmania 5\\Songs\\Test\\testsong\\6.sm";
	ifstream wavfile(file);
	string buffer, titleBuffer;
	//cout << (2 << 1) << endl;
	while (getline(wavfile, buffer)){
		int colon_sep = buffer.find_first_of(':');
		if (colon_sep == -1) continue;
		titleBuffer = buffer.substr(0, colon_sep);
		if (titleBuffer == SM_CONST[SM_NOTE])
			storeSteps(wavfile);
		if (titleBuffer == SM_CONST[SM_MUSIC])
			music_file = buffer.substr(colon_sep + 1, buffer.length() - colon_sep - 2);
		if (titleBuffer == SM_CONST[SM_OFFSET])
			offset = stof(buffer.substr(colon_sep + 1, buffer.length() - colon_sep - 2));
		if (titleBuffer == SM_CONST[SM_START])
			start = stof(buffer.substr(colon_sep + 1, buffer.length() - colon_sep - 2));
		if (titleBuffer == SM_CONST[SM_BPM]){
			string bpmstr = buffer.substr(colon_sep + 1, buffer.length() - colon_sep - 2);
			bpm = stoi(bpmstr.substr(bpmstr.find_first_of('=') + 1));
			spb = (float)60 / bpm;
			cout << bpm << " bpm " << spb << endl;
		}
	}

	cout << music_file << " | " << offset << " | " << start << " | " << bpm << endl;
	for (list<Note*>::iterator it = songNotes.begin(); it != songNotes.end(); ++it){
		cout << intToStep((*it)->getArrow()) << ": " << (*it)->getTime() << endl;
	}
	cout << "Steps: " << songNotes.size() << endl;
	return 0;
}