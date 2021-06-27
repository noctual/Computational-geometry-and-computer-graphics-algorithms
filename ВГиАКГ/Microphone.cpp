#include "Microphone.h"

Microphone::Microphone()
{
	wfx.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
	wfx.nChannels = 1;
	wfx.nSamplesPerSec = 44100;
	wfx.wBitsPerSample = 32;
	wfx.nBlockAlign = wfx.wBitsPerSample * wfx.nChannels / 8;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

	waveInOpen(&wi, WAVE_MAPPER, &wfx, NULL, NULL, CALLBACK_NULL | WAVE_FORMAT_DIRECT);

	headers.lpData = (LPSTR)buffers;
	headers.dwBufferLength = BUFF_SIZE;
	
	waveInPrepareHeader(wi, &headers, sizeof(headers));
	waveInAddBuffer(wi, &headers, sizeof(headers));
	waveInStart(wi);

	gist = new Gist<float>(frameSize, sampleRate);
}

const std::vector<float> Microphone::MagnitudeSpectrum() {
	if (!processed) {
		gist->processAudioFrame((float*)headers.lpData, frameSize);
		processed = true;
	}
	return gist->getMagnitudeSpectrum();
}

void Microphone::ClearBuffer() {
	headers.dwFlags = 0;
	headers.dwBytesRecorded = 0;

	waveInPrepareHeader(wi, &headers, sizeof(headers));
	waveInAddBuffer(wi, &headers, sizeof(headers));
	processed = false;
}

Microphone::~Microphone()
{
	waveInStop(wi);
	waveInUnprepareHeader(wi, &headers, sizeof(headers));
	waveInClose(wi);
	delete gist;
}
