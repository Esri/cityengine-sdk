//-
// ==========================================================================
// Copyright 1995,2006,2008 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

#ifndef _polyModifierFty
#define _polyModifierFty

// 
// File: polyModifierFty.h
//
// polyModifier Factory: polyModifierFty
//
// Author: Lonnie Li
//
// Overview:
//
//		The polyModifierFty class is the main workhorse of the polyModifierCmd operation.
//		It is here that the actual operation is implemented. The idea of the factory is
//		to create a single implementation of the modifier that can be reused in more than
//		one place.
//
//		As such, the details of the factory are quite simple. Each factory contains a doIt()
//		method which should be overridden. This is the method which will be called by the
//		node and the command when a modifier is requested.
//
// How to use:
//
//		1) Create a factory derived from polyModifierFty
//		2) Add any input methods and members to the factory
//		3) Override the polyModifierFty::doIt() method
//
//			(a) Retrieve the inputs from the class
//			(b) Process the inputs
//			(c) Perform the modifier
//
//

// General Includes
//
#include <maya/MStatus.h>

class polyModifierFty
{
public:
						polyModifierFty();
	virtual				~polyModifierFty();

	// Pure virtual doIt()
	//
	virtual MStatus		doIt() = 0;
};

#endif
