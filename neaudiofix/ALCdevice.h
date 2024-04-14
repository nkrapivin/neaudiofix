#pragma once

#include "pch.h"

//#define __GHIDRA__ 1
typedef int ALint;
typedef ALint ALenum;
typedef ALint ALCenum;
typedef unsigned int ALuint;
typedef float ALfloat;
typedef unsigned char uint8;

enum Channel : int {
	FRONT_LEFT,
	FRONT_RIGHT,
	FRONT_CENTER,
	LFE,
	SIDE_LEFT,
	SIDE_RIGHT,
	BACK_LEFT,
	BACK_RIGHT,
	BACK_CENTER,
	OUTPUTCHANNELS
};

struct ALCdevice_struct;

struct ALCdevice_struct_vtbl {
	void(__stdcall* destructor)(ALCdevice_struct* This);
	void(__stdcall* ReorderChannels)(ALCdevice_struct* This);
	void(__stdcall* openPlayback)(ALCdevice_struct* This, char* _pDeviceName);
	void(__stdcall* closePlayback)(ALCdevice_struct* This);
	void(__stdcall* resetPlayback)(ALCdevice_struct* This);
	void(__stdcall* startPlayback)(ALCdevice_struct* This);
	void(__stdcall* stopPlayback)(ALCdevice_struct* This);
	void(__stdcall* pausePlayback)(ALCdevice_struct* This);
	void(__stdcall* resumePlayback)(ALCdevice_struct* This);
	void(__stdcall* openRecording)(ALCdevice_struct* This, int frequency, int bufferSize);
	void(__stdcall* closeRecording)(ALCdevice_struct* This);
	void(__stdcall* startRecording)(ALCdevice_struct* This);
	void(__stdcall* stopRecording)(ALCdevice_struct* This);
	void(__stdcall* getRecordingData)(ALCdevice_struct* This, void* outputData, int numSamples);
	int(__stdcall* getRecordingDataLen)(ALCdevice_struct* This);
};

struct Mutex {
	LPCRITICAL_SECTION criticalSection;
};

typedef void* (*PThreadFunc)(void* threadArg);
typedef HANDLE YYHTHREAD;
class CThread {
public:
	YYHTHREAD m_hThread;
	int m_errorCode;
	bool m_bTerminate;
	bool m_bRunning;
	bool m_bPaused;
	void* m_pFunctionArg;
	PThreadFunc m_pThreadFunc;
	Mutex* m_pTermMutex;
};

static constexpr int QUADRANT_NUM = 128;
static constexpr int LUT_NUM = 4 * QUADRANT_NUM;

struct ALCdevice_struct {
#ifndef __GHIDRA__
	virtual ~ALCdevice_struct() = 0;
	virtual void ReorderChannels() = 0;
	virtual void openPlayback(const char *_pDeviceName) = 0;
	virtual void closePlayback(void) = 0;
	virtual void resetPlayback(void) = 0;
	virtual void startPlayback(void) = 0;
	virtual void stopPlayback(void) = 0;
	virtual void pausePlayback(void) = 0;
	virtual void resumePlayback(void) = 0;
	virtual void openRecording(int frequency, int bufferSize) = 0;
	virtual void closeRecording(void) = 0;
	virtual void startRecording(void) = 0;
	virtual void stopRecording(void) = 0;
	virtual void getRecordingData(void* outputData, int numSamples) = 0;
	virtual int getRecordingDataLen(void) = 0;
#else
	ALCdevice_struct_vtbl *__vfptr;
#endif
	ALCenum error;
	ALenum format;
	ALuint frequency;
	ALuint updateSize;
	ALuint numUpdates;
	ALfloat headDampen;
	ALuint activeListenerMask;
	bool duplicateStereo;
	ALint numChan;
	ALfloat channelMatrix[Channel::OUTPUTCHANNELS][Channel::OUTPUTCHANNELS];
	Channel speaker2Chan[Channel::OUTPUTCHANNELS];
	ALfloat panningLUT[Channel::OUTPUTCHANNELS * LUT_NUM];
};

struct ALCdevice_wasapi : public ALCdevice_struct {
	CThread* m_pThread;
	uint8* m_data;
};

using ALCdevice = ALCdevice_wasapi;
struct ALsource;
struct ALbuffer;

struct ALlistener {
	float position[3];
	float velocity[3];
	float up[3];
	float forward[3];
	float gain;
	float metersPerUnit;
};

struct ALCcontext_struct {
	ALCcontext_struct* pNext;
	ALenum lastError;
	ALenum distanceModel;
	ALlistener _listener;
	ALfloat dopplerFactor;
	ALfloat dopplerVelocity;
	ALfloat flSpeedOfSound;
	ALuint activeListenerMask;
	Mutex* _mutex;
	ALsource* _pFirstSource;
	ALsource* _pLastSource;
	ALuint _sourceNumber;
	ALsource* _pFreeSources;
	ALbuffer* _pFirstBuffer;
	ALbuffer* _pLastBuffer;
	ALuint _bufferNumber;
	ALCdevice* _pDevice;
};

static_assert(sizeof(ALCdevice_wasapi) == 18856);
static_assert(sizeof(ALCcontext_struct) == 160);

using ALCcontext = ALCcontext_struct;
