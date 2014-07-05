/*
 * RyzomRenderingService - https://github.com/nimetu/rrs_server
 * Copyright (c) 2014 Meelis Mägi <nimetu@gmail.com>
 *
 * This file is part of RyzomRenderingService.
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#include "stdpch.h"
#include "tools.h"

//
void applyColorSlot(NL3D::UInstance inst, const sint skin, const sint userColor, const sint hairColor, const sint eyesColor) {
	CColorSlotManager::TIntCouple array[4];
	// Skin
	array[0].first = (uint) 0;
	array[0].second = (uint) skin;
	// User Color
	array[1].first = (uint) 1;
	array[1].second = (uint) userColor;
	// Hair Color
	array[2].first = (uint) 2;
	array[2].second = (uint) hairColor;
	// Eyes Color
	array[3].first = (uint) 3;
	array[3].second = (uint) eyesColor;
	nldebug("ColorSlotManager: {%d, %d, %d, %d} = %s", array[0].second, array[1].second, array[2].second, array[3].second, inst.getShapeName().c_str());
	ColorSlotManager.setInstanceSlot(inst, array, 4);
}

//
float calcMorphValue(float target, float MTmin, float MTmax) {
	float val = (float) (target) / 7.f * (MTmax - MTmin) + MTmin;
	NLMISC::clamp(val, MTmin, MTmax);

	return val;
}

//
void animatePSM(NL3D::CParticleSystemModel *psm) {
	if (psm) {
		psm->enableAutoGetEllapsedTime(false);
		psm->setEllapsedTime(psm->getEllapsedTime() + 5);
		for (int i = 0; i < 5; i++) {
			psm->traverseAnimDetail();
		}
	}
}

//
void animateTS(NL3D::CTransformShape *ts) {
	if (ts) {
		NL3D::CParticleSystemModel *psm = dynamic_cast<NL3D::CParticleSystemModel *>(ts);
		animatePSM(psm);
	}
}

//
void fixImageBackground(NLMISC::CBitmap &btm, const NLMISC::CRGBA bgcolor, bool remove) {
	uint32 w = btm.getWidth();
	uint32 h = btm.getHeight();
	uint8 *data = btm.getPixels().getPtr();

	// this fixes holes in 32bit final image where (semi-)transparent texture
	// does not reset final color alpha value
	// - all non-background color alpha value is set to 255
	for (uint32 y = 0; y < h; y++) {
		for (uint32 x = 0; x < w * 4; x += 4) {
			uint8 r = data[(h - y - 1) * w * 4 + x];
			uint8 g = data[(h - y - 1) * w * 4 + x + 1];
			uint8 b = data[(h - y - 1) * w * 4 + x + 2];
			uint8 a = data[(h - y - 1) * w * 4 + x + 3];
			// this will force all non-background colors to be fully visible,
			// but does not touch background color alpha if not requested.
			if (r == bgcolor.R && g == bgcolor.G && b == bgcolor.B) {
				if (remove) {
					a = 0; // pixel is completely transparent
				}
			} else {
				a = 255; // make pixel fully visible
			}
			data[(h - y - 1) * w * 4 + x + 3] = a;
		}
	}
}

//
bool getBonePos(NL3D::USkeleton &_Skeleton, const std::string bone, NLMISC::CVector &pos) {
	if (_Skeleton.empty()) {
		nlwarning("no skeleton");
		return false;
	}
	sint boneId = _Skeleton.getBoneIdByName(bone);
	if (boneId == -1) {
		nlwarning("bad bone name (%s)", bone.c_str());
		return false;
	}
	_Skeleton.forceComputeBone(boneId);
	NL3D::UBone rBone = _Skeleton.getBone(boneId);
	const NLMISC::CMatrix &rM = rBone.getLastWorldMatrixComputed();
	rM.getPos(pos);

	nldebug("getBonePos: (%s) at (%s)", bone.c_str(), pos.toString().c_str());
	return true;
}

//
const std::string getAnimation(const CGenderInfo *pGI, const std::vector<CItemSheet *> _Sheets, float &_CustomWeaponRadius) {
	std::string anim;

	_CustomWeaponRadius = 0;
	if (_Sheets[SLOTTYPE::RIGHT_HAND_SLOT]) {
		switch (_Sheets[SLOTTYPE::RIGHT_HAND_SLOT]->ItemType) {
		default:
			if (_Sheets[SLOTTYPE::LEFT_HAND_SLOT]) {
				// fall thru
				switch (_Sheets[SLOTTYPE::LEFT_HAND_SLOT]->ItemType) {
				case ITEM_TYPE::DAGGER:
					anim = "_ad_idle.anim";
					//_CustomScalePos += 0.05;
					break;
				case ITEM_TYPE::BUCKLER:
				case ITEM_TYPE::SHIELD:
					anim = "_ab_idle1_0.anim";
					//_CustomScalePos += 0.15;
					break;
				default:
					anim = "_a_idle.anim";
				}
			} else {
				anim = "_a_idle.anim";
			}
			break;
		case ITEM_TYPE::TWO_HAND_AXE:
		case ITEM_TYPE::TWO_HAND_MACE:
			anim = "_l2m_idle.anim";
			_CustomWeaponRadius = 0.1;
			break;
		case ITEM_TYPE::PIKE:
			anim = "_l2m_idle.anim";
			// over the shoulder
			//anim = "_l2m_idle1_1.anim";
			_CustomWeaponRadius = 0.5;
			break;
		case ITEM_TYPE::TWO_HAND_SWORD:
			anim = "_a2m_idle1_1.anim";
			//anim = "_a_idle.anim";
			break;
		case ITEM_TYPE::PISTOL:
		case ITEM_TYPE::BOWPISTOL:
			anim = "_pa_idle.anim";
			break;
		case ITEM_TYPE::AUTOLAUCH:
		case ITEM_TYPE::LAUNCHER:
		case ITEM_TYPE::RIFLE:
			anim = "_fu_idle.anim";
			if (_Sheets[SLOTTYPE::RIGHT_HAND_SLOT]->ItemType == ITEM_TYPE::RIFLE) {
				_CustomWeaponRadius = 0.7;
			} else {
				_CustomWeaponRadius = 0.4;
			}
			break;
		case ITEM_TYPE::BOWRIFLE:
			anim = "_fus_idle.anim";
			_CustomWeaponRadius = 0.6;
			break;
		case ITEM_TYPE::MAGICIAN_STAFF:
			//anim = "_idle2.anim";
			anim = "_idle.anim";
			break;
		}
	} else {
		if (_Sheets[SLOTTYPE::LEFT_HAND_SLOT]) {
			// fall thru
			switch (_Sheets[SLOTTYPE::LEFT_HAND_SLOT]->ItemType) {
			case ITEM_TYPE::DAGGER:
				anim = "_ad_idle.anim";
				break;
			case ITEM_TYPE::BUCKLER:
			case ITEM_TYPE::SHIELD:
				anim = "_ab_idle1_0.anim";
				break;
			default:
				anim = "_a_idle.anim";
			}
		} else {
			anim = "_idle.anim";
		}
	}

	anim = pGI->AnimSetBaseName + anim;

	nldebug("using anim %s", anim.c_str());
	return anim;
}
