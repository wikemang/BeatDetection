#ifndef SIMPLEBEATDETECTION_H
#define SIMPLEBEATDETECTION_H
#include <cstdio>
#include "constants.h"
#include <math.h>

using namespace std;

class AvgBuffer{
	__int64 energyBuff[45];	//Keep 1 sec as running average
	__int64 amplitudeBuff[45];	//Keep 1 sec as running average
	int currentIndex;
public:
	AvgBuffer();
	void insert(int value, __int64 energySum);
	__int64 getAverageEnergy();
	int getAverage();
	float getVariance();
	float getSensitivityConstant();
};

AvgBuffer::AvgBuffer(){
	for (int i = 0; i < 45; i++){
		energyBuff[i] = 0;
		amplitudeBuff[i] = 0;
	}
	currentIndex = -1;
}

void AvgBuffer::insert(int amplitudeSum, __int64 energySum){
	if (currentIndex == 44){
		currentIndex = -1;
	}
	currentIndex += 1;

	amplitudeBuff[currentIndex] = amplitudeSum;
	energyBuff[currentIndex] = energySum;
}

__int64 AvgBuffer::getAverageEnergy(){
	__int64 total = 0;
	for (int i = 0; i < 45; i++){
		total += energyBuff[i];
	}
	return total / 45;
}

int AvgBuffer::getAverage(){
	int total = 0;
	for (int i = 0; i < 45; i++){
		total += amplitudeBuff[i];
	}
	return total / 45;
}

float AvgBuffer::getVariance(){
	__int64 average = getAverage();
	__int64 total = 0;
	for (int i = 0; i < 45; i++){
		total += (amplitudeBuff[i] - average) * (amplitudeBuff[i] - average);
	}
	return (float)total * 980 / 44100;
}

float AvgBuffer::getSensitivityConstant(){
	return (-0.0025714 * getVariance() + 1.5142857);
}

class BeatDetector{
	AvgBuffer* avgBuffer;
public:
	BeatDetector();
	~BeatDetector();
	virtual void detectBeat(bool* highlight, int* dot);

};

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
			if (currentE > 1.2* avgBuffer->getAverageEnergy()){
				beats[i + k * 45] = true;
			}
			dot += 980;
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