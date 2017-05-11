#pragma once

#include "prt/API.h"

#include <string>
#include <iostream>
#include <sstream>
#include <iterator>


/**
 * helper classes to redirect log events
 */

namespace logging {

struct Logger { };

const std::wstring LEVELS[] = { L"trace", L"debug", L"info", L"warning", L"error", L"fatal" };

// log to std streams
template<prt::LogLevel L>
struct StreamLogger : public Logger {
	StreamLogger(std::wostream& out = std::wcout) : Logger(), mOut(out) { mOut << prefix(); }
	virtual ~StreamLogger() { mOut << std::endl; }
	StreamLogger<L>& operator<<(std::wostream&(*x)(std::wostream&)) { mOut << x; return *this; }
	StreamLogger<L>& operator<<(const std::string& x) { std::copy(x.begin(), x.end(), std::ostream_iterator<char, wchar_t>(mOut)); return *this; }
	template<typename T> StreamLogger<L>& operator<<(const T& x) { mOut << x; return *this; }
	static std::wstring prefix() { return L"[" + LEVELS[L] + L"] "; }
	std::wostream& mOut;
};

// log through the prt logger
template<prt::LogLevel L>
struct PRTLogger : public Logger {
	PRTLogger() : Logger() { }
	virtual ~PRTLogger() { prt::log(wstr.str().c_str(), L); }
	PRTLogger<L>& operator<<(std::wostream&(*x)(std::wostream&)) { wstr << x;  return *this; }
	PRTLogger<L>& operator<<(const std::string& x) {
		std::copy(x.begin(), x.end(), std::ostream_iterator<char, wchar_t>(wstr));
		return *this;
	}
	template<typename T> PRTLogger<L>& operator<<(const T& x) { wstr << x; return *this; }
	std::wostringstream wstr;
};

// choose your logger (PRTLogger or StreamLogger)
template<prt::LogLevel L>
using LT = PRTLogger<L>;

using _LOG_DBG = LT<prt::LOG_DEBUG>;
using _LOG_INF = LT<prt::LOG_INFO>;
using _LOG_WRN = LT<prt::LOG_WARNING>;
using _LOG_ERR = LT<prt::LOG_ERROR>;

} // namespace logging

#define LOG_DBG logging::_LOG_DBG()
#define LOG_INF logging::_LOG_INF()
#define LOG_WRN logging::_LOG_WRN()
#define LOG_ERR logging::_LOG_ERR()
