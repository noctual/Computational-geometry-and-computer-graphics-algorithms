#pragma once
#pragma comment(lib,"winmm.lib")

#include <Windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <iostream>
#include <vector>

#include "libs/Gist/Gist.h"

#define BUFF_SIZE 1000

class Microphone
{
public:
	Microphone();
	const std::vector<float> MagnitudeSpectrum();
	void ClearBuffer();
	~Microphone();

private:
	float buffers[BUFF_SIZE];
	int frameSize = BUFF_SIZE;
	int sampleRate = 44100;
	Gist<float>* gist = nullptr;
	WAVEFORMATEX wfx = { };
	WAVEHDR headers = { };
	HWAVEIN wi;
	bool processed = false;
};
