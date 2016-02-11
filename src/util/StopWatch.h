#ifndef UTIL_STOPWATCH_H_
#define UTIL_STOPWATCH_H_

#include <chrono>
#include <ostream>
#include <string>
#include <vector>

class StopWatch {
private:
	typedef std::chrono::time_point<std::chrono::system_clock> timePoint;
	typedef std::chrono::milliseconds mili_sec;
	typedef std::chrono::microseconds micro_sec;

	std::vector<long> splitTimes;

	timePoint currentTime;

public:
	void start() {
		splitTimes.clear();
		currentTime = std::chrono::system_clock::now();
	}

	long stop() {
		splitTimes.push_back(
				static_cast<long>(std::chrono::duration_cast < micro_sec
						> (std::chrono::system_clock::now() - currentTime).count()));
		return splitTimes.back();
	}

	void split() {
		stop();
	}

	void splitRestart() {
		timePoint tp = std::chrono::system_clock::now();
		splitTimes.push_back(
				static_cast<long>(std::chrono::duration_cast < micro_sec
						> (tp - currentTime).count()));
		currentTime = tp;
	}

	void clear() {
		splitTimes.clear();
	}

	std::vector<long> getSplitTimes() {
		return splitTimes;
	}

	long getLastSplit() {
		return splitTimes.back();
	}

	long averageSplit() {
		return sumSplits() / splitTimes.size();
	}

	long sumSplits() {
		long sum = 0;
		for (auto time : splitTimes) {
			sum += time;
		}
		return sum;
	}

};

#endif
