/**
 * Esri CityEngine SDK Maya Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 *
 * See README.md in https://github.com/Esri/esri-cityengine-sdk for build instructions.
 *
 * Copyright (c) 2012-2019 Esri R&D Center Zurich
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

#include "node/Utilities.h"

#include "maya/MString.h"

#include <cstdio>
#include <cstdarg>


namespace prtu {


	const char* filename(const char* path) {
		while (*(--path) != '\\');
		return path + 1;
	}


	template<> char getDirSeparator() {
#ifdef _MSC_VER
		static const char SEPARATOR = '\\';
#else
		static const char SEPARATOR = '/';
#endif
		return SEPARATOR;
	}


	template<> wchar_t getDirSeparator() {
#ifdef _MSC_VER
		static const wchar_t SEPARATOR = L'\\';
#else
		static const wchar_t SEPARATOR = L'/';
#endif
		return SEPARATOR;
	}


#if DO_DBG == 1

	void dbg(const char* fmt, ...) {
		va_list args;

		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
		printf("\n");
		fflush(0);
	}

	void wdbg(const wchar_t* fmt, ...) {
		va_list args;

		va_start(args, fmt);
		vwprintf(fmt, args);
		va_end(args);
		wprintf(L"\n");
		fflush(0);
	}

#else
	void dbg(const char* fmt, ...) { }
	void wdbg(const wchar_t* fmt, ...) { }
#endif

	int fromHex(wchar_t c) {
		switch (c) {
		case '0': return 0;	case '1': return 1;	case '2': return 2;	case '3': return 3;	case '4': return 4;
		case '5': return 5;	case '6': return 6;	case '7': return 7;	case '8': return 8;	case '9': return 9;
		case 'a': case 'A': return 0xa;
		case 'b': case 'B': return 0xb;
		case 'c': case 'C': return 0xc;
		case 'd': case 'D': return 0xd;
		case 'e': case 'E': return 0xe;
		case 'f': case 'F': return 0xf;
		default:
			return 0;
		}
	}

	const wchar_t HEXTAB[] = L"0123456789ABCDEF";

	wchar_t toHex(int i) {
		return HEXTAB[i & 0xF];
	}

	void toHex(wchar_t* color, double r, double g, double b) {
		color[1] = toHex(((int)(r * 255)) >> 4);
		color[2] = toHex((int)(r * 255));
		color[3] = toHex(((int)(g * 255)) >> 4);
		color[4] = toHex((int)(g * 255));
		color[5] = toHex(((int)(b * 255)) >> 4);
		color[6] = toHex((int)(b * 255));
	}


	MString toCleanId(const MString& name) {
		const unsigned int len = name.numChars();
		const wchar_t*     wname = name.asWChar();
		wchar_t*           dst = new wchar_t[len + 1];
		for (unsigned int i = 0; i < len; i++) {
			wchar_t c = wname[i];
			if ((c >= '0' && c <= '9') ||
				(c >= 'A' && c <= 'Z') ||
				(c >= 'a' && c <= 'z'))
				dst[i] = c;
			else
				dst[i] = '_';
		}
		dst[len] = L'\0';
		MString result(dst);
		delete[] dst;
		return result;
	}


	int32_t computeSeed(const MFloatPoint& p) {
		int32_t seed = static_cast<int32_t>(p[0]);
		seed ^= static_cast<int32_t>(p[2]);
		seed %= 714025;
		return seed;
	}


	int32_t computeSeed(MFloatPointArray& vertices) {
		MFloatPoint a(0.0, 0.0, 0.0);
		for (unsigned int vi = 0; vi < vertices.length(); vi++) {
			a += vertices[vi];
		}
		a = a / static_cast<float>(vertices.length());
		return computeSeed(a);
	}


	int32_t computeSeed(const double* vertices, size_t count) {
		MFloatPoint a(0.0, 0.0, 0.0);
		for (unsigned int vi = 0; vi < count; vi += 3) {
			a[0] += static_cast<float>(vertices[vi + 0]);
			a[1] += static_cast<float>(vertices[vi + 1]);
			a[2] += static_cast<float>(vertices[vi + 2]);
		}
		a = a / static_cast<float>(count);
		return computeSeed(a);
	}


} // namespace prtu
