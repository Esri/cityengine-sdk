/**
 * Esri CityEngine SDK Unity Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

#pragma once

#include "prt/API.h"

/*
 *	Log handler that forwards messages to Unity via a provided callback (delegate)
 */
class UnityLogHandler : public prt::LogHandler {
public:
	typedef void (*LogMessageCallback)(const wchar_t* message, uint32_t level);

private:
	const prt::LogLevel* mLevels;
	size_t mCount;
	LogMessageCallback mCallback;

public:
	UnityLogHandler(const prt::LogLevel* levels, size_t count)
		: mLevels(levels)
		, mCount(count)
		, mCallback(NULL) {}

public:
	void setCallback(LogMessageCallback callback) {
		mCallback = callback;
	}

	LogMessageCallback getCallback() {
		return mCallback;
	}

	void handleLogEvent(const wchar_t* msg, prt::LogLevel level) {
		if(mCallback != NULL)
			mCallback(msg, static_cast<uint32_t>(level));
	}

	const prt::LogLevel* getLevels(size_t* count) {
		*count = mCount;
		return mLevels;
	}

	void getFormat(bool* dateTime, bool* level) {
		*dateTime = true;
		*level = true;
	}
};
