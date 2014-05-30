/**
 * Esri CityEngine SDK Unity Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

#pragma once

template<typename T>
inline void SafeDestroy(T*& ptr) {
	if(ptr != NULL) {
		ptr->destroy();
		ptr = NULL;
	}
}


template<typename T>
class ScopedObject {
private:
	typedef T* pointer;
	pointer m_obj;

public:
	explicit ScopedObject(pointer obj)
		: m_obj(obj) {}

	~ScopedObject() {
		if(m_obj)
			m_obj->destroy();
	}

	pointer get() {
		return m_obj;
	}

	pointer operator->() {
		return m_obj;
	}

	T& operator*() {
		return *m_obj;
	}

	void destroy() {
		if(m_obj) {
			m_obj->destroy();
			m_obj = NULL;
		}
	}
};
