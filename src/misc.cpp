// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

//
// Functions in this file is from Ryzom client sources
// <code/ryzom/client/src/misc.cpp>
//

#include "stdpch.h"
#include "misc.h"

using namespace NL3D;
using namespace NLMISC;

//-----------------------------------------------
// makeUp :
// \param face : pointer on the face to make up (must not be null).
// \param idMakeUp : index of the make-up to apply.
// \warning This function does not check if 'face'  is valid.
//-----------------------------------------------
void makeUp(NL3D::UInstance face, sint idMakeUp) {
	static const char *tattooStr = "visage_makeup";

	// look for tattoo texture
	uint numMat = face.getNumMaterials();
	std::string texFilename;
	for (uint k = 0; k < numMat; ++k) {
		UInstanceMaterial im = face.getMaterial(k);
		sint numTex = im.getLastTextureStage();
		for (sint l = 0; l <= numTex; ++l) {
			if (im.isTextureFile(l)) { // one texture from a file ?
				// see if it is the texture used for tattoos
				texFilename = im.getTextureFileName(l);
				nlinfo("visage tex = %s", texFilename.c_str());
				std::string::size_type pos = texFilename.find(tattooStr, 0);
				if (pos != std::string::npos) {
					uint charIndex = (uint) (pos + strlen(tattooStr));
					if (texFilename.length() >= charIndex + 2) {
						texFilename[charIndex] = '0' + (unsigned char) (idMakeUp / 10);
						texFilename[charIndex + 1] = '0' + (unsigned char) (idMakeUp % 10);
						nlinfo(">> %s", texFilename.c_str());
						im.setTextureFileName(texFilename, l);
					}
				}
			}
		}
	}
} // makeUp //

//-----------------------------------------------
// highlight shape
//-----------------------------------------------
void setEmissive(NL3D::UInstance inst, NLMISC::CRGBA emit) {
	UShape shape = inst.getShape();
	uint numMats = shape.getNumMaterials();
	if (numMats == 0 || numMats != inst.getNumMaterials()) {
		return;
	}
	for (uint j = 0; j < numMats; j++) {

		inst.getMaterial(j).setEmissive(emit);
	}
}

//-----------------------------------------------
//
//-----------------------------------------------
void dumpGenderInfo(const CGenderInfo *pGI, bool bMale) {
	nldebug("GenderInfo:: (%s)", bMale ? "male" : "female");
	//nldebug("Race sheet: (%d): (%s), male: %d", RSid.asInt(), RSid.toString().c_str(), bMale);
	nldebug("UnknownItemName = {%s}", pGI->UnknownItemName.c_str());
	nldebug("Skelfilename = {%s}", pGI->Skelfilename.c_str());
	nldebug("AnimSetBaseName = {%s}", pGI->AnimSetBaseName.c_str());
	nldebug("Items: %d", (uint) SLOTTYPE::NB_SLOT);
	// FIXME: extern static uint NumUsedVisualSlots
	for (int slot = 0; slot < 9; slot++) {
		nldebug("> %d: {%s}", slot, pGI->Items[slot].c_str());
	}
	nldebug("LodCharacterName = {%s}", pGI->LodCharacterName.c_str());
	nldebug("LogCharacterDistance = %f", pGI->LodCharacterDistance);
	nldebug("CharacterScalePos = %f", pGI->CharacterScalePos);
	nldebug("GroundFX:");
	for (std::vector<CGroundFXSheet>::const_iterator it = pGI->GroundFX.begin(); it != pGI->GroundFX.end(); it++) {
		if ((sint) it->GroundID == -1) {
			break;
		}
		nldebug("%d: %d, {%s}", it->GroundID, it->IdFXName, it->getFXName().c_str());
	}
	nldebug("BlendShape Min/Max");
	for (int i = 0; i < NB_MORPH_TARGETS; i++) {
		nldebug("> %d: min:%f, max:%f", i, pGI->BlendShapeMin[i], pGI->BlendShapeMax[i]);
	}
	nldebug("NamePosZLow/Normal/High= {%f, %f, %f}", pGI->NamePosZLow, pGI->NamePosZNormal, pGI->NamePosZHigh);

}

//-----------------------------------------------
// drawBox :
// Draw a Box from 2 vectors.
//-----------------------------------------------
void drawBox(const CVector &vMin, const CVector &vMax, const CRGBA &color) {
	CLineColor line;
	line.Color0 = color;
	line.Color1 = color;
	// Bottom quad
	line = CLine(CVector(vMin.x, vMin.y, vMin.z), CVector(vMax.x, vMin.y, vMin.z));
	Driver->drawLine(line, GenericMat);
	line = CLine(CVector(vMax.x, vMin.y, vMin.z), CVector(vMax.x, vMax.y, vMin.z));
	Driver->drawLine(line, GenericMat);
	line = CLine(CVector(vMax.x, vMax.y, vMin.z), CVector(vMin.x, vMax.y, vMin.z));
	Driver->drawLine(line, GenericMat);
	line = CLine(CVector(vMin.x, vMax.y, vMin.z), CVector(vMin.x, vMin.y, vMin.z));
	Driver->drawLine(line, GenericMat);
	// Top quad
	line = CLine(CVector(vMin.x, vMin.y, vMax.z), CVector(vMax.x, vMin.y, vMax.z));
	Driver->drawLine(line, GenericMat);
	line = CLine(CVector(vMax.x, vMin.y, vMax.z), CVector(vMax.x, vMax.y, vMax.z));
	Driver->drawLine(line, GenericMat);
	line = CLine(CVector(vMax.x, vMax.y, vMax.z), CVector(vMin.x, vMax.y, vMax.z));
	Driver->drawLine(line, GenericMat);
	line = CLine(CVector(vMin.x, vMax.y, vMax.z), CVector(vMin.x, vMin.y, vMax.z));
	Driver->drawLine(line, GenericMat);
	// Sides Quad
	line = CLine(CVector(vMin.x, vMin.y, vMin.z), CVector(vMin.x, vMin.y, vMax.z));
	Driver->drawLine(line, GenericMat);
	line = CLine(CVector(vMax.x, vMin.y, vMin.z), CVector(vMax.x, vMin.y, vMax.z));
	Driver->drawLine(line, GenericMat);
	line = CLine(CVector(vMax.x, vMax.y, vMin.z), CVector(vMax.x, vMax.y, vMax.z));
	Driver->drawLine(line, GenericMat);
	line = CLine(CVector(vMin.x, vMax.y, vMin.z), CVector(vMin.x, vMax.y, vMax.z));
	Driver->drawLine(line, GenericMat);
} // drawBox //
