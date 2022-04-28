#pragma once
#include "Singleton.h"

class GameTime : public Singleton<GameTime>
{
	friend class Singleton<GameTime>;
public:
	GameTime();

	float TotalTime()const; // in seconds
	float DeltaTime()const; // in seconds
	float FramesPerSecond()const;

	void Reset(); // Call before message loop.
	void Start(); // Call when unpaused.
	void Stop();  // Call when paused.
	void Tick();  // Call every frame.

private:
	double mSecondsPerCount;
	double mDeltaTime;
	double mFps;
	double mTimeAccumulated;

	__int64 mFrameCounter;

	__int64 mBaseTime;
	__int64 mPausedTime;
	__int64 mStopTime;
	__int64 mPrevTime;
	__int64 mCurrTime;

	bool mStopped;
};
