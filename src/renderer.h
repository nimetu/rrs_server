/*
 * RyzomRenderingService - https://github.com/nimetu/rrs_server
 * Copyright (c) 2014 Meelis Mägi <nimetu@gmail.com>
 *
 * This file is part of RyzomRenderingService.
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#ifndef RENDERER_H
#define RENDERER_H

#include "stdpch.h"
#include "main.h"
#include "visual_config.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;

extern CColorSlotManager ColorSlotManager;

class CRenderer {
private:
	CVisualConfig _Config;

	uint32 _ScreenWidth;
	uint32 _ScreenHeight;
	uint32 _ScreenDepth;
	bool _ShowWindow;

	float _AspectRatio;
	float _FoV;

	bool _Dirty;
	bool _Alive;

	bool _HasFX;

public:
	//
	CRenderer(CConfigFile ConfigFile);
	virtual ~CRenderer();

	bool idle();
	bool renderInto(CBitmap &btm);
	bool setupScene(const CVisualConfig &Config);

	bool isDirty() const {
		return _Dirty;
	}

	bool isAlive() const {
		return _Alive;
	}

	void shutdown(std::string reason) {
		nlinfo("Shutdown: %s", reason.c_str());
		_Alive = false;
	}

private:
	UAnimationSet *_AnimationSet;
	UPlayList *_PlayList;
	UPlayListManager *_PlayListManager;

	USkeleton _Skeleton;

	float _HeightScale;
	float _CustomScalePos;
	float _CustomWeaponRadius;

	CRGBA _EmissivColor;

	std::vector<NL3D::UInstance> _ListInstance;
	std::vector<CItemSheet *> _Sheets;
	std::vector<NL3D::UParticleSystemInstance> _FX;

	typedef std::vector<NL3D::UInstance> TStaticFXs;
	TStaticFXs _StaticFXs;

private:

	void render();

	void clearScene();

	void clearSlot(const SLOTTYPE::EVisualSlot slot);

	void equipShape(const SLOTTYPE::EVisualSlot slot, const std::string &shapeName, const CItemSheet *item);
	void equipVS(const SLOTTYPE::EVisualSlot slot, const uint index, const uint color = -1);

	void setupCamera();
	void loadAnimation();

	CGenderInfo *getGenderInfo();

	void setupMorph(UInstance inst, const SPropVisualC vpc, const CGenderInfo *pGI);
	void setupGabarit(USkeleton skel, const uint sex, const EGSPD::CPeople::TPeople people, const SPropVisualC vpc, float *customScale, float *heightScale);
};

#endif
