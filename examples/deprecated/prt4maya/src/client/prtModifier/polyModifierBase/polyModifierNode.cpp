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

// 
// File: polyModifier.cpp
//
// Dependency Graph Node: polyModifier
//
// Author: Lonnie Li
//

#include "polyModifierNode.h"

// General Includes
//
#include <maya/MGlobal.h>

MObject     polyModifierNode::inMesh;
MObject     polyModifierNode::outMesh;

polyModifierNode::polyModifierNode()
{}

polyModifierNode::~polyModifierNode()
{}

