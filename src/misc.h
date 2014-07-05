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

#ifndef _MISC_H
#define	_MISC_H

#include "stdpch.h"
#include "main.h"

//<code/ryzom/client/src/misc.cpp>
void makeUp(NL3D::UInstance face, sint idMakeUp);
void setEmissive(NL3D::UInstance inst, NLMISC::CRGBA emit);
void dumpGenderInfo(const CGenderInfo *pGI, bool bMale);
void drawBox(const NLMISC::CVector &vMin, const NLMISC::CVector &vMax, const NLMISC::CRGBA &color);

#endif	/* _MISC_H */

