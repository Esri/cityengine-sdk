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

#pragma once

#include "prt/Callbacks.h"
#include "prtx/Material.h"

class IMayaCallbacks : public prt::Callbacks {
public:
	virtual ~IMayaCallbacks() { }

	virtual void setVertices(const double* vtx, size_t size) = 0;
	virtual void setNormals(const double* nrm, size_t size) = 0;
	virtual void setUVs(const double* u, const double* v, size_t size) = 0;
    virtual void setMaterial(uint32_t start, uint32_t count, const prtx::MaterialPtr& mat) = 0;

	virtual void setFaces(
			const uint32_t* counts, size_t countsSize,
			const uint32_t* connects, size_t connectsSize,
			const uint32_t* uvCounts, size_t uvCountsSize,
			const uint32_t* uvConnects, size_t uvConnectsSize
	) = 0;

	virtual void createMesh() = 0;
	virtual void finishMesh() = 0;

};
