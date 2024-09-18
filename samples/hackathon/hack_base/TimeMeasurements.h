#pragma once

#include "utils.h"

#include "json.hpp"

#include <algorithm>
#include <chrono>
#include <iterator>
#include <map>
#include <mutex>
#include <numeric>
#include <string>
#include <vector>

struct SummarizedTimings
{
	uint64_t  mMin;
	uint64_t  mAverage;
	uint64_t  mMean;
	uint64_t  mDeviation;
	uint64_t  mP90;
	uint64_t  mP95;
	uint64_t  mP99;
	uint64_t  mMax;
};

struct TimingsOfType
{
  public:
	void addTiming(uint64_t value);
	SummarizedTimings calculateSummarizations();
	nlohmann::json toJson();

  private:
	size_t                                                      mWarmUpCounter = 0;
	std::mutex                                                  mDataPointsLock;
	std::array<uint64_t, HackConstants::MaxNumberOfDataPoints>  mDataPoints;
	size_t                                                      mNextIdxToFill = 0;
};

class TimeMeasurements
{
  public:
	void addTime(MeasurementPoints label, uint64_t value)
	{
		if (!mEnabled)
			return;

		if (mTimes.count(label) == 0)
		{
			mTimesLock.lock();
			if (mTimes.count(label) == 0)
			{
				mTimes.try_emplace(label, std::unique_ptr<TimingsOfType>(new TimingsOfType()));
			}
			mTimesLock.unlock();
		}

		mTimes[label]->addTiming(value);
	}
	void writeToJsonFile();
	void disable()
	{
		mEnabled = false;
	}
	bool isEnabled() const
	{
		return mEnabled;
	}

  private:
	bool                                                        mEnabled = true;
	std::mutex                                                  mTimesLock;
	std::map<MeasurementPoints, std::unique_ptr<TimingsOfType>> mTimes;
};

class ScopedTiming
{
  public:
	ScopedTiming(TimeMeasurements &sw, MeasurementPoints label) :
	    mSw(sw), mLabel(label), mStartTime(std::chrono::high_resolution_clock::now()){};

	~ScopedTiming()
	{
		uint64_t duration = (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - mStartTime)).count();
		mSw.addTime(mLabel, duration);
	}

  private:
	TimeMeasurements                     &mSw;
	MeasurementPoints                     mLabel;
	std::chrono::steady_clock::time_point mStartTime;
};
