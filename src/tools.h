/*
 * RyzomRenderingService - https://github.com/nimetu/rrs_server
 * Copyright (c) 2014 Meelis Mägi <nimetu@gmail.com>
 *
 * This file is part of RyzomRenderingService.
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#ifndef _TOOLS_H
#define	_TOOLS_H

#include "stdpch.h"
#include "main.h"

// my functions
void applyColorSlot(NL3D::UInstance inst, const sint skin, const sint userColor, const sint hairColor, const sint eyesColor);
float calcMorphValue(float target, float MTmin, float MTmax);
void animatePSM(NL3D::CParticleSystemModel *psm);
void animateTS(NL3D::CTransformShape *ts);
void fixImageBackground(NLMISC::CBitmap &btm, const NLMISC::CRGBA bgcolor, bool remove);
bool getBonePos(NL3D::USkeleton &_Skeleton, const std::string bone, NLMISC::CVector &pos);
const std::string getAnimation(const CGenderInfo *pGI, const std::vector<CItemSheet *> _Sheets, float &_CustomWeaponRadius);

#endif	/* _TOOLS_H */

