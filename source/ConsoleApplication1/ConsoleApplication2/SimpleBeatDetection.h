#ifndef SIMPLEBEATDETECTION_H
#define SIMPLEBEATDETECTION_H
#include <cstdio>
#include "constants.h"
#include <math.h>

using namespace std;
const int buffSize = 15;

class AvgBuffer{
	__int64 energyBuff[buffSize];	//Keep 1 sec as running average
	__int64 amplitudeBuff[buffSize];	//Keep 1 sec as running average
	int currentIndex;
public:
	AvgBuffer();
	void insert(int value, __int64 energySum);
	__int64 getEnergy();
	int getAmplitude();
	int getVariance();
	float getSensitivityConstant();
	float getSensitivityConstant2();

	long getLastEnergy(int index);
};

AvgBuffer::AvgBuffer(){
	for (int i = 0; i < buffSize; i++){
		energyBuff[i] = 0;
		amplitudeBuff[i] = 0;
	}
	currentIndex = -1;
}

void AvgBuffer::insert(int amplitudeSum, __int64 energySum){
	if (currentIndex == buffSize - 1){
		currentIndex = -1;
	}
	currentIndex += 1;

	amplitudeBuff[currentIndex] = amplitudeSum;
	energyBuff[currentIndex] = energySum;
}

__int64 AvgBuffer::getEnergy(){
	__int64 total = 0;
	for (int i = 0; i < buffSize; i++){
		total += energyBuff[i];
	}
	return total / buffSize;
}

int AvgBuffer::getAmplitude(){
	int total = 0;
	for (int i = 0; i < buffSize; i++){
		total += amplitudeBuff[i];
	}
	return total / buffSize;
}

int AvgBuffer::getVariance(){
	__int64 average = getAmplitude();
	__int64 total = 0;
	for (int i = 0; i < buffSize; i++){
		total += (amplitudeBuff[i] - average) * (amplitudeBuff[i] - average);
	}
	return total / buffSize;
}

float AvgBuffer::getSensitivityConstant(){
	return (-0.00000000233 * getVariance() + 2);
}

float AvgBuffer::getSensitivityConstant2(){
	return (-0.00000000233 * getVariance() + 1.7);
}

long AvgBuffer::getLastEnergy(int index){
	return energyBuff[buffSize - index];
}

class BeatDetector{
	AvgBuffer* avgBuffer;
public:
	BeatDetector();
	~BeatDetector();
	virtual void detectBeat(bool* highlight, int* dot);

};

int debug_test_max = 0;

void BeatDetector::detectBeat(bool* beats, int dot[980]){
	__int64 currentE;	//Current Energy total (A^2)
	int currentA;		//Current amplitude total
	for (int k = 0; k < 90; k++){
		for (int i = 0; i < 45; i++){
			currentE = 0;
			currentA = 0;
			for (int j = 0; j < 980; j++){
				currentE += (__int64)(dot[j]-320) * (__int64)(dot[j]-320);
				currentA += abs(dot[j] - 320);
			}
			avgBuffer->insert(currentA, currentE);
			beats[i + k * 45] = false;
			if (debug_test_max < avgBuffer->getVariance())
				debug_test_max = avgBuffer->getVariance();
			//cout << avgBuffer->getSensitivityConstant() << endl;
			if (currentE > avgBuffer->getSensitivityConstant()* avgBuffer->getEnergy() ){
				beats[i + k * 45] = true;
			}
			dot += 980;
		}
	}


	for (int i = 4049; i > 4; i--){
		if (beats[i]){
			if (beats[i - 1] || beats[i - 2] || beats[i - 3] || beats[i - 4])
				beats[i] = false;
		}
	}
	cout << "max" << debug_test_max << endl;
}

BeatDetector::BeatDetector(){
	avgBuffer = new AvgBuffer();
}
BeatDetector::~BeatDetector(){
	delete(avgBuffer);
}
#endif