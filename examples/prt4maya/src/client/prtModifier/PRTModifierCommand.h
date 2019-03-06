#pragma once

#include "polyModifierBase/polyModifierCmd.h"

// based on the splitUVCommand and meshOpCommand Maya example . 
class PRTModifierCommand : public polyModifierCmd
{
public:

	PRTModifierCommand() = default;

	static void* creator();

	bool isUndoable() const override;

	MStatus doIt(const MArgList&) override;
	MStatus redoIt() override;
	MStatus undoIt() override;

	MStatus initModifierNode(MObject modifierNode) override;
	MStatus directModifier(MObject mesh) override;

private:

	MString mRulePkg;
};

