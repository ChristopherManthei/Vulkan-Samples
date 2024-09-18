#include "TimeMeasurements.h"

void TimingsOfType::addTiming(uint64_t value)
{
	// Just ignore all values that go over the measurement limit
	if (mNextIdxToFill >= mDataPoints.size())
		return;

	mDataPointsLock.lock();
	mDataPoints[mNextIdxToFill] = value;
	mNextIdxToFill++;
	mDataPointsLock.unlock();
}

SummarizedTimings TimingsOfType::calculateSummarizations()
{
	SummarizedTimings summary = {};
	std::vector<uint64_t> data;

	// Copy the original vector, as it is faster than sorting and summarizing, to not block the data collection to much and not alter the original data.
	mDataPointsLock.lock();
	data.reserve(mNextIdxToFill);
	std::copy(mDataPoints.begin(), mDataPoints.begin() + mNextIdxToFill, std::back_inserter(data));
	mDataPointsLock.unlock();

	std::sort(data.begin(), data.end());

	if (data.size() >= 1)
	{
		summary.mMin     = data[0];
		summary.mAverage = std::accumulate(data.begin(), data.end(), 0ll) / data.size();
		summary.mMean    = data[data.size() / 2];
		 
		uint64_t varianceTemp = 0;
		for (auto measurement : data)
		{
			int64_t difference = static_cast<int64_t>(measurement) - static_cast<int64_t>(summary.mMean);
			varianceTemp += difference * difference;
		}
		summary.mVariance = static_cast<float>(varianceTemp) / static_cast<float>(data.size() - 1);

		summary.mP90 = data[data.size() * 0.9f];
		summary.mP95 = data[data.size() * 0.95f];
		summary.mP99 = data[data.size() * 0.99f];
		summary.mMax = data[data.size() - 1];
	}

	return summary;
}

nlohmann::json TimingsOfType::toJson()
{
	SummarizedTimings summary = calculateSummarizations();

	nlohmann::json timingsOfTypeJsonObj;
	timingsOfTypeJsonObj["DataPointsCount"] = mNextIdxToFill;

	nlohmann::json dataPointsJsonArray = nlohmann::json::array();
	for (int i = 0; i < mNextIdxToFill; ++i)
	{
		dataPointsJsonArray.push_back(mDataPoints[i]);
	}
	timingsOfTypeJsonObj["DataPoints"] = dataPointsJsonArray;

	nlohmann::json summaryJsonObj;
	summaryJsonObj["min"]           = summary.mMin;
	summaryJsonObj["avg"]           = summary.mAverage;
	summaryJsonObj["mean"]          = summary.mMean;
	summaryJsonObj["variance"]      = summary.mVariance;
	summaryJsonObj["p90"]           = summary.mP90;
	summaryJsonObj["p95"]           = summary.mP95;
	summaryJsonObj["p99"]           = summary.mP99;
	summaryJsonObj["max"]           = summary.mMax;
	timingsOfTypeJsonObj["Summary"] = summaryJsonObj;

	return timingsOfTypeJsonObj;
}

// Converts the full TimeMeasurements to a json object and writes it to a file called "data.json"
void TimeMeasurements::writeToJsonFile()
{
	nlohmann::json rootJsonObj;
	nlohmann::json measurementsJsonArray = nlohmann::json::array();

	for (auto iter = mTimes.begin(); iter != mTimes.end(); iter++)
	{
		nlohmann::json timingsOfTypeJsonObj = iter->second->toJson();
		timingsOfTypeJsonObj["Name"]        = MeasurementPointsUtils::MeasurementPointsToString(iter->first);
		measurementsJsonArray.push_back(timingsOfTypeJsonObj);
	}

	rootJsonObj["Measurements"] = measurementsJsonArray;

// Paths differ between windows and android
// I have no idea whether the android path will be the same between different android devices, but this one did work at one point on a Quest 3
#ifdef _WIN32
	const char *filePath = "./data.json";
#else
	const char *filePath = "/data/data/com.khronos.vulkan_samples/files/data.json";
#endif

	std::string outJson = rootJsonObj.dump(4);        // Dump with pretty printing and a width of 4 spaces.

	// That will spike our CPU hard, but as we should stop measuring afterwards it's fine.
	FILE *outFile = std::fopen(filePath, "w");
	std::fwrite(outJson.data(), sizeof(char), outJson.size(), outFile);
	std::fclose(outFile);
}
