#ifndef SIMPLEBEATDETECTION_H
#define SIMPLEBEATDETECTION_H
#include <cstdio>
#include "constants.h"
#include <math.h>

using namespace std;
const int buffSize = 15;

class AvgBuffer{
	__int64 energyBuff[buffSize];		//Circular buffer to store energy (A^2)
	__int64 amplitudeBuff[buffSize];	//Circular buffer to store A
	int currentIndex;					//Current index of circular buffers.
public:
	AvgBuffer();
	void insert(int value, __int64 energySum);
	__int64 getEnergy();				//Get average energy within buffer.
	int getAmplitude();					//Get average amplitude within buffer
	int getVariance();
	float getSensitivityConstant();		//Sensitivity constant calculated with variance. Currently experimentally adjusted.
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
	virtual void detectBeat(bool* highlight, int* dot, int beatCount);

};

void BeatDetector::detectBeat(bool* beats, int dot[980], int beatCount){
	__int64 currentE;	//Current Energy total (A^2), sum of [beatLength] data points
	int currentA;		//Current amplitude total, sum of [beatLength] data points
	for (int k = 0; k < beatCount / scrollRate; k++){
		for (int i = 0; i < scrollRate; i++){
			currentE = 0;
			currentA = 0;
			for (int j = 0; j < beatLength; j++){	//Caculates current total energy and amplitude
				currentE += (__int64)(dot[j]-320) * (__int64)(dot[j]-320);
				currentA += abs(dot[j] - 320);
			}
			avgBuffer->insert(currentA, currentE);
			if (currentE > avgBuffer->getSensitivityConstant()* avgBuffer->getEnergy() ){	//If current energy is greater than the previous average energy by some threashold, a beat is detected.
				beats[i + k * scrollRate] = true;
			}
			dot += beatLength;
		}
	}


	for (int i = beatCount - 1; i > 4; i--){	//Breaks up "Thick" beats.
		if (beats[i]){
			if (beats[i - 1] || beats[i - 2] || beats[i - 3] || beats[i - 4])
				beats[i] = false;
		}
	}
}

BeatDetector::BeatDetector(){
	avgBuffer = new AvgBuffer();
}
BeatDetector::~BeatDetector(){
	delete(avgBuffer);
}
#endif