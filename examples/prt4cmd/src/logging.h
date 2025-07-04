/**
 * CityEngine SDK CLI Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 *
 * See README.md in https://github.com/Esri/cityengine-sdk for build instructions.
 *
 * Copyright (c) 2012-2025 Esri R&D Center Zurich
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "prt/API.h"

#include <iostream>
#include <iterator>
#include <sstream>
#include <string>

/**
 * helper classes to redirect log events
 */

namespace logging {

struct Logger {};

const std::wstring LEVELS[] = {L"trace", L"debug", L"info", L"warning", L"error", L"fatal"};

// log to std streams
template <prt::LogLevel L>
struct StreamLogger : public Logger {
	explicit StreamLogger(std::wostream& out = std::wcout) : Logger(), mOut(out) {
		mOut << prefix();
	}
	virtual ~StreamLogger() {
		mOut << std::endl;
	}
	StreamLogger<L>& operator<<(std::wostream& (*x)(std::wostream&)) {
		mOut << x;
		return *this;
	}
	StreamLogger<L>& operator<<(const std::string& x) {
		std::copy(x.begin(), x.end(), std::ostream_iterator<char, wchar_t>(mOut));
		return *this;
	}
	template <typename T>
	StreamLogger<L>& operator<<(const T& x) {
		mOut << x;
		return *this;
	}
	static std::wstring prefix() {
		return L"[" + LEVELS[L] + L"] ";
	}
	std::wostream& mOut;
};

// log through the prt logger
template <prt::LogLevel L>
struct PRTLogger : public Logger {
	PRTLogger() : Logger() {}
	virtual ~PRTLogger() {
		prt::log(wstr.str().c_str(), L);
	}
	PRTLogger<L>& operator<<(std::wostream& (*x)(std::wostream&)) {
		wstr << x;
		return *this;
	}
	PRTLogger<L>& operator<<(const std::string& x) {
		std::copy(x.begin(), x.end(), std::ostream_iterator<char, wchar_t>(wstr));
		return *this;
	}
	template <typename T>
	PRTLogger<L>& operator<<(const T& x) {
		wstr << x;
		return *this;
	}
	std::wostringstream wstr;
};

// choose your logger (PRTLogger or StreamLogger)
template <prt::LogLevel L>
using LT = PRTLogger<L>;

using LOG_DBG_ = LT<prt::LOG_DEBUG>;
using LOG_INF_ = LT<prt::LOG_INFO>;
using LOG_WRN_ = LT<prt::LOG_WARNING>;
using LOG_ERR_ = LT<prt::LOG_ERROR>;

} // namespace logging

#define LOG_DBG logging::LOG_DBG_()
#define LOG_INF logging::LOG_INF_()
#define LOG_WRN logging::LOG_WRN_()
#define LOG_ERR logging::LOG_ERR_()
