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
#include "main.h"
#include "misc.h"
#include "tools.h"
#include "renderer.h"
#include "visual_config.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;

CRenderer::CRenderer(CConfigFile ConfigFile) :
		_AnimationSet(NULL), _PlayList(NULL), _PlayListManager(NULL), _Skeleton(
		NULL), _HeightScale(1.f), _CustomScalePos(1.f), _HasFX(false), _CustomWeaponRadius(1.f) {

	//
	_ScreenWidth = ConfigFile.getVar("ScreenWidth").asInt();
	_ScreenHeight = ConfigFile.getVar("ScreenHeight").asInt();
	_ScreenDepth = ConfigFile.getVar("ScreenDepth").asInt();
	_ShowWindow = ConfigFile.getVar("ShowWindow").asBool();

	_AspectRatio = float(_ScreenWidth) / float(_ScreenHeight);
	_FoV = float(20.0 * Pi / 180.0);

	nldebug("creating driver");
	Driver = NL3D::UDriver::createDriver();
	nlassert(Driver);

	nldebug("initializing screen");

	NL3D::UDriver::CMode mode(_ScreenWidth, _ScreenHeight, _ScreenDepth, true, 0, -1);
	nldebug("setDisplay(%d, %d, %d, %s)", mode.Width, mode.Height, mode.Depth, _ShowWindow ? "show" : "hidden");
	Driver->setDisplay(mode, _ShowWindow, false); // not-resizeable

	if (_ShowWindow) {
		Driver->setWindowTitle(ucstring("RRS"));
		//Driver->setWindowPos(10, 10);
		Driver->showWindow(_ShowWindow);
	}

	nldebug("creating scene");
	Scene = Driver->createScene(false);
	Scene->setLayersRenderingOrder(true);

	nldebug("creating camera");
	Camera = Scene->getCam();
	Camera.setTransformMode(UTransformable::DirectMatrix);

	Camera.setPerspective(_FoV, _AspectRatio, 0.1f, 1000.0f);

	//Scene->enableLightingSystem(true);
	//Scene->enableShadowPolySmooth(true);
	Scene->setPolygonBalancingMode(UScene::PolygonBalancingClamp);
	Scene->setGroupLoadMaxPolygon("Fx", 100000);
	Scene->setMaxSkeletonsInNotCLodForm(100000);

	nldebug("creating playlist manager");
	_PlayListManager = Scene->createPlayListManager();

	nldebug("loading gabarit set");
	IProgressCallback callback;
	GabaritSet.loadGabarits(callback);

	nldebug("init done");

	GenericMat = Driver->createMaterial();
	if (GenericMat.empty()) {
		nlerror("init: Cannot create generic material");
	}

	// is skeleton already present - need to remove before render
	_Dirty = false;
	_Alive = true;

	_EmissivColor = CRGBA::White;

	// make room for visual slots
	_ListInstance.resize(SLOTTYPE::NB_SLOT);	// keeps UInstance's
	_Sheets.resize(SLOTTYPE::NB_SLOT);	// keeps CItemSheet's
	_FX.resize(SLOTTYPE::NB_SLOT);	// keeps FX

	Scene->animate(0);

	// without this, window is not displayed
	if (_ShowWindow) {
		Driver->EventServer.pump();
	}
}

CRenderer::~CRenderer() {
	clearScene();

	if (_PlayList != NULL) {
		_PlayListManager->deletePlayList(_PlayList);
		_PlayList = NULL;
	}

	if (_PlayListManager != NULL) {
		Scene->deletePlayListManager(_PlayListManager);
		_PlayListManager = NULL;
	}

	// FIXME: release sun/light

	Driver->deleteScene(Scene);
	Scene = NULL;

	Driver->release();
	delete Driver;
	Driver = NULL;
}

//-----------------------------------------------
bool CRenderer::idle() {
	if (!Driver->isActive()) {
		shutdown("shutting down because Driver is not active");
		return false;
	}

	if (_ShowWindow) {
		Driver->EventServer.pump();
		if (Driver->AsyncListener.isKeyPushed(KeyESCAPE)) {
			shutdown("shutting down because ESCAPE was pressed");
			return false;
		}

		if (Driver->AsyncListener.isKeyDown(KeyD)) {
			if (_Skeleton.empty()) {
				nlinfo("_Skeleton: empty");
			} else {
				nlinfo("_Skeleton: {%s}", _Skeleton.getShapeName().c_str());
				nlinfo(">> scale : {%s}", _Skeleton.getScale().asString().c_str());
			}
			nlinfo("_ListInstance:");
			for (uint i = 0; i < _ListInstance.size(); i++) {
				if (_ListInstance[i].empty()) {
					nlinfo("%d: empty", i);
				} else {
					nlinfo("%d: {%s}, {%s}", i, _ListInstance[i].getScale().asString().c_str(), _ListInstance[i].getShapeName().c_str());
				}
			}
			nlinfo("_Sheets:");
			for (uint i = 0; i < _Sheets.size(); i++) {
				if (_Sheets[i]) {
					nlinfo("%d: {%d}, {%s}, {%s}", i, _Sheets[i]->Family, _Sheets[i]->getAnimSet().c_str(), _Sheets[i]->getShape().c_str());
				} else {
					nlinfo("%d: empty", i);
				}
			}
			nlinfo("_FX:");
			for (uint i = 0; i < _FX.size(); i++) {
				if (_FX[i].empty()) {
					nlinfo("%d: empty", i);
				} else {
					nlinfo("%d: {%s}", i, _FX[i].getShapeName().c_str());
				}
			}

			nlinfo("_StaticFX:");
			for (TStaticFXs::iterator it = _StaticFXs.begin(); it != _StaticFXs.end(); ++it) {
				if (!it->empty()) {
					nlinfo("  : {%s}", it->getShapeName().c_str());
				}
			}
		}

		if (Driver->AsyncListener.isKeyDown(KeyP)) {
			CFrustum frustum = Camera.getFrustum();
			frustum.Perspective = !frustum.Perspective;
			Camera.setFrustum(frustum);
		}

		if (Driver->AsyncListener.isKeyDown(KeyR)) {
			render();
		}
	}

	return isAlive();
}

bool CRenderer::renderInto(CBitmap &btm) {
	if (!isAlive()) {
		nlinfo("Window has gone away, can't render");
		return false;
	}

	render();

	nldebug("Taking screenshot...");
	Driver->getBuffer(btm);

	fixImageBackground(btm, _Config.Background, RemoveBackground);

	return true;
}

bool CRenderer::setupScene(const CVisualConfig& cfg) {
	_Config = cfg;
	const CGenderInfo *pGI = getGenderInfo();

	// clear everything - background, slots, skeleton
	clearScene();

	TTime cp, pcp;
	TTime _start = NLMISC::CTime::getLocalTime();
	pcp = _start;

	//_Root = Scene->createTransform();
	if (pGI->Skelfilename.empty()) {
		nlinfo("Skeleton filename is empty, error");
		return false;
	}

	_Skeleton = Scene->createSkeleton(pGI->Skelfilename);
	_Skeleton.setTransformMode(UTransformable::RotQuat);
	_Skeleton.setPos(CVector(0, 0, 0));

	cp = NLMISC::CTime::getLocalTime();
	nldebug(">> createSkel: %dms", cp - pcp);
	pcp = cp;

	// equip items
	equipVS(SLOTTYPE::HEAD_SLOT, _Config.vpa.PropertySubData.HatModel, _Config.vpa.PropertySubData.HatColor);
	equipVS(SLOTTYPE::CHEST_SLOT, _Config.vpa.PropertySubData.JacketModel, _Config.vpa.PropertySubData.JacketColor);
	equipVS(SLOTTYPE::ARMS_SLOT, _Config.vpa.PropertySubData.ArmModel, _Config.vpa.PropertySubData.ArmColor);
	equipVS(SLOTTYPE::HANDS_SLOT, _Config.vpb.PropertySubData.HandsModel, _Config.vpb.PropertySubData.HandsColor);
	equipVS(SLOTTYPE::FEET_SLOT, _Config.vpb.PropertySubData.FeetModel, _Config.vpb.PropertySubData.FeetColor);
	equipVS(SLOTTYPE::LEGS_SLOT, _Config.vpa.PropertySubData.TrouserModel, _Config.vpa.PropertySubData.TrouserColor);

	if (!_Config.FaceShot && _Config.vpa.PropertySubData.WeaponRightHand > 0) {
		equipVS(SLOTTYPE::RIGHT_HAND_SLOT, _Config.vpa.PropertySubData.WeaponRightHand);
	}
	if (!_Config.FaceShot && _Config.vpa.PropertySubData.WeaponLeftHand > 0) {
		equipVS(SLOTTYPE::LEFT_HAND_SLOT, _Config.vpa.PropertySubData.WeaponLeftHand);
	}

	// build face if haircut is set
	if (_Sheets[SLOTTYPE::HEAD_SLOT] && _Sheets[SLOTTYPE::HEAD_SLOT]->Family != ITEMFAMILY::ARMOR) {
		sint idx = SheetMngr.getVSIndex(pGI->Items[SLOTTYPE::FACE_SLOT], SLOTTYPE::FACE_SLOT);
		if (idx != -1) {
			equipVS(SLOTTYPE::FACE_SLOT, idx);
			if (!_ListInstance[SLOTTYPE::FACE_SLOT].empty()) {
				makeUp(_ListInstance[SLOTTYPE::FACE_SLOT], _Config.vpc.PropertySubData.Tattoo);
			}
		} else {
			nlwarning("Unable to get FACE visual slot index");
		}
	}

	cp = NLMISC::CTime::getLocalTime();
	nldebug(">> setupItems: %dms", cp - pcp);
	pcp = cp;

	// load animation based equiped item
	loadAnimation();
	cp = NLMISC::CTime::getLocalTime();
	nldebug(">> loadAnim: %dms", cp - pcp);
	pcp = cp;

	// morph targets
	setupMorph(_ListInstance[SLOTTYPE::FACE_SLOT], _Config.vpc, getGenderInfo());

	cp = NLMISC::CTime::getLocalTime();
	nldebug(">> setupMorph: %dms", cp - pcp);
	pcp = cp;

	// skeleton size
	setupGabarit(_Skeleton, (uint) _Config.vpa.PropertySubData.Sex, _Config.Race, _Config.vpc, &_CustomScalePos, &_HeightScale);

	cp = NLMISC::CTime::getLocalTime();
	nldebug(">> setupGabarit: %dms", cp - pcp);
	pcp = cp;

	// mark scene as dirty for 'cronjob' cleanup task
	_Dirty = true;

	return true;
}

//-----------------------------------------------
void CRenderer::render() {
	// full detail
	Scene->setMaxSkeletonsInNotCLodForm(1000);
	Scene->setPolygonBalancingMode(UScene::PolygonBalancingOff);

	Scene->animate(NLMISC::CTime::getLocalTime());

	setupCamera();

	//drawBox(CVector(0.1, 0.5, 0), CVector(0.5,1,1), CRGBA::White);

	// initialize and calculate particle systems
	Scene->render();

	if (_HasFX) {
		for (uint slot = 0; slot < _FX.size(); slot++) {
			if (!_FX[slot].empty()) {
				animatePSM(_FX[slot].getObjectPtr());
			}
		}
		for (TStaticFXs::iterator it = _StaticFXs.begin(); it != _StaticFXs.end(); ++it) {
			if (!it->empty()) {
				animateTS(it->getObjectPtr());
			}
		}

		// render whole screen again with particle systems this time
		Driver->clearBuffers(_Config.Background);
		Driver->clearZBuffer();
		Scene->render();
	}

	if (_ShowWindow) {
		Driver->swapBuffers();
	}
}

void CRenderer::setupCamera() {
	CAABBox bbox;
	if (!_Skeleton.computeCurrentBBox(bbox, _PlayList, 0.01f, true, false)) {
		// center made up - half size from zorai male
		bbox.setHalfSize(CVector(0.304290, 0.490923, 1.271324));
	}

	CVector halfSize = bbox.getHalfSize();
	float radius = max(halfSize.x, halfSize.z);
	if (radius == 0.f) {
		radius = 1.f;
	}

	float dist;
	float camHeight;
	CVector pos;
	if (_Config.FaceShot && getBonePos(_Skeleton, "Bip01 Head", pos)) {
		camHeight = 0.10f;
		dist = 0.2;
		nldebug("Camera dist %f, radius %f", dist, radius);
		nldebug("Face shot, pos (%s) as center", pos.asString().c_str());
	} else {
		pos = _Skeleton.getPos();
		camHeight = 0;
		dist = (radius + _CustomWeaponRadius * _CustomScalePos) / tan(_FoV);
		nldebug("Camera dist %f, radius %f, CustomWeaponRadius: %f, CustomScalePos %f", dist, radius, _CustomWeaponRadius, _CustomScalePos);
		nldebug("Full shot, pos (%s) as center", pos.asString().c_str());
	}

	CVector eye(pos);
	eye.z += camHeight;

	CVector center(eye);

	CVector rot;
	rot.sphericToCartesian((dist + radius), _Config.Theta, 0.f/*_Config.Phi*/);
	eye += rot;

	Camera.lookAt(eye, center);

	nlinfo("Skel (%s)", pos.toString().c_str());
	nlinfo("Bbox min:%s max:%s center:%s", bbox.getMin().toString().c_str(), bbox.getMax().toString().c_str(), bbox.getCenter().toString().c_str());
	nlinfo("Cam (%s) (%s)", eye.toString().c_str(), center.toString().c_str());
}

void CRenderer::clearScene() {
	Driver->clearBuffers(_Config.Background);
	Driver->clearZBuffer();

	if (!_Dirty) {
		return;
	}

	for (TStaticFXs::iterator it = _StaticFXs.begin(); it != _StaticFXs.end(); ++it) {
		if (!it->empty()) {
			Scene->deleteInstance(*it);
		}
	}
	_StaticFXs.clear();

	clearSlot(SLOTTYPE::CHEST_SLOT);
	clearSlot(SLOTTYPE::LEGS_SLOT);
	clearSlot(SLOTTYPE::HEAD_SLOT);
	clearSlot(SLOTTYPE::ARMS_SLOT);
	clearSlot(SLOTTYPE::FACE_SLOT);
	clearSlot(SLOTTYPE::HANDS_SLOT);
	clearSlot(SLOTTYPE::FEET_SLOT);
	clearSlot(SLOTTYPE::RIGHT_HAND_SLOT);
	clearSlot(SLOTTYPE::LEFT_HAND_SLOT);

	Scene->deleteSkeleton(_Skeleton);
	_Skeleton = NULL;

	_Dirty = false;
	_HasFX = false;
}

void CRenderer::clearSlot(const SLOTTYPE::EVisualSlot slot) {
	if (!_ListInstance[slot].empty()) {
		nldebug("clear slot %d", slot);
		Scene->deleteInstance(_ListInstance[slot]);
	}

	_Sheets[slot] = NULL;

	if (!_FX[slot].empty()) {
		Scene->deleteInstance(_FX[slot]);
	}
}

void CRenderer::equipShape(const SLOTTYPE::EVisualSlot slot, const std::string &shapeName, const CItemSheet *item) {
	// FIXME: load FX
	if (shapeName.empty()) {
		return;
	}

	// check for conflicting gloves-amps items
	if (item) {
		if (slot == SLOTTYPE::HANDS_SLOT && item->Family == ITEMFAMILY::ARMOR) {
			// probably gloves, see if there is amps already added
			if (_Sheets[SLOTTYPE::RIGHT_HAND_SLOT] && _Sheets[SLOTTYPE::RIGHT_HAND_SLOT]->ItemType == ITEM_TYPE::MAGICIAN_STAFF) {
				return;
			}
		} else if (slot == SLOTTYPE::RIGHT_HAND_SLOT && item->ItemType == ITEM_TYPE::MAGICIAN_STAFF) {
			// we should clear gloves before equipping amps
			clearSlot(SLOTTYPE::HANDS_SLOT);
		}
	}

	UInstance inst = Scene->createInstance(shapeName);
	if (inst.empty()) {
		return;
	}

	_ListInstance[slot] = inst;

	// bind shape
	sint stickID = -1;
	std::string boneName;
	if (item) {
		switch (slot) {
		case SLOTTYPE::RIGHT_HAND_SLOT:
			if (item->ItemType != ITEM_TYPE::MAGICIAN_STAFF) {
				boneName = "box_arme";
			}
			break;
		case SLOTTYPE::LEFT_HAND_SLOT:
			if (item->getAnimSet() == "s") {
				boneName = "Box_bouclier";
			} else {
				boneName = "box_arme_gauche";
			}
			break;
		default:
			//boneName = ClientSheetsStrings.get(item->IdBindPoint); ///item->getBindPoint();
			boneName = "";
			break;
		}
	}

	if (boneName.empty()) {
		_Skeleton.bindSkin(_ListInstance[slot]);
	} else {
		stickID = _Skeleton.getBoneIdByName(boneName);
		nldebug(">> slot (%d) stick {%d}, {%s}", slot, stickID, boneName.c_str());
		if (stickID != -1) {
			_Skeleton.stickObject(_ListInstance[slot], stickID);
		}
	}

	if (_Config.useFx && item->HasFx) {
		uint numStaticFX = item->FX.getNumStaticFX();
		nldebug(">> StaticFX count: %d", numStaticFX);
		if (numStaticFX > 0) {
			_HasFX = true;
			_StaticFXs.reserve(numStaticFX);
			for (uint k = 0; k < numStaticFX; k++) {
				string boneName = item->FX.getStaticFXBone(k);
				string name = item->FX.getStaticFXName(k);
				if (!boneName.empty() && !name.empty()) {
					nldebug(">> StaticFX (%d): bone {%s}, name {%s}", k, boneName.c_str(), name.c_str());
					sint boneID = _Skeleton.getBoneIdByName(boneName);
					if (boneID != -1) {
						NL3D::UInstance fx = Scene->createInstance(name);
						if (fx.empty()) {
							nlwarning("_FX (%s) file not found", name.c_str());
							continue;
						}
						fx.setTransformMode(UTransform::DirectMatrix);
						CMatrix mat;
						mat.setPos(item->FX.getStaticFXOffset(k));
						fx.setMatrix(mat);

						_Skeleton.stickObject(fx, boneID);
						_Skeleton.forceComputeBone(boneID);

						_StaticFXs.push_back(fx);
						nldebug(">> added: {%s}", name.c_str());
					} else {
						nldebug(">> invalid bone name (%s)", boneName.c_str());
					}
				}
			}
		}

		{ // FIXME: outpost amps have static fx and advantage fx
			std::string shapeName = item->FX.getAdvantageFX();
			nldebug(">> Item has advantageFX (slot:%d, shape:%s)", slot, shapeName.c_str());
			if (!shapeName.empty()) {
				NL3D::UInstance fx = Scene->createInstance(shapeName);
				if (fx.empty())
					return;
				_FX[slot].cast(fx);
				if (_FX[slot].empty()) {
					nldebug(">> FX (%s) not found", shapeName.c_str());
					Scene->deleteInstance(fx);
					return;
				}
				_HasFX = true;
				CMatrix mat = _ListInstance[slot].getMatrix();
				mat.invert();
				mat *= _FX[slot].getMatrix();
				_FX[slot].setTransformMode(UTransformable::DirectMatrix);
				_FX[slot].setMatrix(mat);
				_FX[slot].forceDisplayBBox(true);
				_FX[slot].parent(_ListInstance[slot]);
				nldebug(">> advantage FX (%s) added", shapeName.c_str());
			}
		}
	}
}

void CRenderer::equipVS(const SLOTTYPE::EVisualSlot slot, const uint index, const uint color) {
	const CGenderInfo *pGI = getGenderInfo();

	_Sheets[slot] = SheetMngr.getItem(slot, index);

	if (_Sheets[slot]) {
		const CItemSheet *item = _Sheets[slot];
		nldebug(">> equip: (%d,%d) - %s / %s", slot, index, item->getShape().c_str(), item->getShapeFemale().c_str());
		if (_Config.vpa.PropertySubData.Sex == GSGENDER::female) {
			equipShape(slot, item->getShapeFemale(), item);
		} else {
			equipShape(slot, item->getShape(), item);
		}
	}

	if (_ListInstance[slot].empty()) {
		return;
	}

	if (!_Sheets[slot]) {
		nlwarning("Sheets[slot] is empty (slot=%d)", slot);
	}

	uint texIndex;
	if (slot == SLOTTYPE::FACE_SLOT) {
		// for face, select requested age as there is only single (C1) texture set in sheets
		texIndex = _Config.Age;
	} else {
		texIndex = (uint) _Sheets[slot]->MapVariant;
	}
	nldebug(">> equip: selecting texture set: %d", texIndex);
	_ListInstance[slot].selectTextureSet(texIndex);
	nldebug(">> item (%d/%d) %s", slot, index, _Sheets[slot]->Id.toString().c_str());

	// test to see if we have haircut
	if (slot == SLOTTYPE::HEAD_SLOT && _Sheets[slot]->Family != ITEMFAMILY::ARMOR) {
		applyColorSlot(_ListInstance[slot], _Config.Race, 0, color, _Config.vpc.PropertySubData.EyesColor);
	} else {
		if (_Sheets[slot]->Color == -1) {
			// user color
			applyColorSlot(_ListInstance[slot], _Config.Race, color, _Config.HairColor, _Config.vpc.PropertySubData.EyesColor);
		} else if (_Sheets[slot]->Color != -2) {
			// hardcoded item color
			applyColorSlot(_ListInstance[slot], _Config.Race, _Sheets[slot]->Color, _Config.HairColor, _Config.vpc.PropertySubData.EyesColor);
		} else {
			// default color
			applyColorSlot(_ListInstance[slot], _Config.Race, 0, _Config.HairColor, _Config.vpc.PropertySubData.EyesColor);
		}
	}

	if (_Sheets[slot]->HasFx) {
		//nldebug("FIXME: (%d, %d) item has FX", slot, index);
	}

	// light up each instance
	setEmissive(_ListInstance[slot], _EmissivColor);
}

void CRenderer::loadAnimation() {
	const CGenderInfo *pGI = getGenderInfo();
	std::string anim = getAnimation(pGI, _Sheets, _CustomWeaponRadius);

	if (_PlayList != NULL) {
		_PlayListManager->deletePlayList(_PlayList);
		_PlayList = NULL;
	}
	if (_AnimationSet != NULL) {
		Driver->deleteAnimationSet(_AnimationSet);
		_AnimationSet = NULL;
	}

	_AnimationSet = Driver->createAnimationSet();
	_PlayList = _PlayListManager->createPlayList(_AnimationSet);
	uint animID = _AnimationSet->addAnimation(anim.c_str(), anim.c_str());
	_AnimationSet->build();

	_PlayList->registerTransform(_Skeleton);
	_PlayList->setAnimation(0, animID);
	_PlayList->setTimeOrigin(0, 0);
	_PlayList->setStartWeight(0, 1.0f, 0);
	_PlayList->setEndWeight(0, 1.0f, 0);
	_PlayList->setWeightSmoothness(0, 1.0f);
	_PlayList->setWrapMode(0, UPlayList::Clamp);
	_PlayListManager->animate(0.0f);
}

void CRenderer::setupMorph(UInstance inst, const SPropVisualC vpc, const CGenderInfo *pGI) {
	if (!inst.empty()) {
		inst.setBlendShapeFactor("visage_000", calcMorphValue(vpc.PropertySubData.MorphTarget1, pGI->BlendShapeMin[0], pGI->BlendShapeMax[0]), true);
		inst.setBlendShapeFactor("visage_001", calcMorphValue(vpc.PropertySubData.MorphTarget2, pGI->BlendShapeMin[1], pGI->BlendShapeMax[1]), true);
		inst.setBlendShapeFactor("visage_002", calcMorphValue(vpc.PropertySubData.MorphTarget3, pGI->BlendShapeMin[2], pGI->BlendShapeMax[2]), true);
		inst.setBlendShapeFactor("visage_003", calcMorphValue(vpc.PropertySubData.MorphTarget4, pGI->BlendShapeMin[3], pGI->BlendShapeMax[3]), true);
		inst.setBlendShapeFactor("visage_004", calcMorphValue(vpc.PropertySubData.MorphTarget5, pGI->BlendShapeMin[4], pGI->BlendShapeMax[4]), true);
		inst.setBlendShapeFactor("visage_005", calcMorphValue(vpc.PropertySubData.MorphTarget6, pGI->BlendShapeMin[5], pGI->BlendShapeMax[5]), true);
		inst.setBlendShapeFactor("visage_006", calcMorphValue(vpc.PropertySubData.MorphTarget7, pGI->BlendShapeMin[6], pGI->BlendShapeMax[6]), true);
		inst.setBlendShapeFactor("visage_007", calcMorphValue(vpc.PropertySubData.MorphTarget8, pGI->BlendShapeMin[7], pGI->BlendShapeMax[7]), true);
	}
}

void CRenderer::setupGabarit(USkeleton skel, const uint sex, const EGSPD::CPeople::TPeople people, const SPropVisualC vpc, float *customScale, float *heightScale) {
	float characterHeight = (float) ((sint8) (vpc.PropertySubData.CharacterHeight) - 7) / 7.f;
	float torsoWidth = (float) ((sint8) (vpc.PropertySubData.TorsoWidth) - 7) / 7.f;
	float armsWidth = (float) ((sint8) (vpc.PropertySubData.ArmsWidth) - 7) / 7.f;
	float legsWidth = (float) ((sint8) (vpc.PropertySubData.LegsWidth) - 7) / 7.f;
	float breastSize = (float) ((sint8) (vpc.PropertySubData.BreastSize) - 7) / 7.f;

	GabaritSet.applyGabarit(skel, sex, people, characterHeight, torsoWidth, armsWidth, legsWidth, breastSize, heightScale);
	float refHeightScale = GabaritSet.getRefHeightScale(sex, people);
	*customScale = 1.f;
	if (refHeightScale != 0.f)
		*customScale = *heightScale / refHeightScale;
	nldebug("setupGabarit: customScale %f, heightScale %f, refHeightScale %f", customScale, heightScale, refHeightScale);
}

CGenderInfo *CRenderer::getGenderInfo() {
	bool bMale = _Config.vpa.PropertySubData.Sex == GSGENDER::male;

	CSheetId RSid;
	switch (_Config.Race) {
	default:
	case EGSPD::CPeople::Fyros:
		RSid = CSheetId("fyros.race_stats");
		break;
	case EGSPD::CPeople::Matis:
		RSid = CSheetId("matis.race_stats");
		break;
	case EGSPD::CPeople::Tryker:
		RSid = CSheetId("tryker.race_stats");
		break;
	case EGSPD::CPeople::Zorai:
		RSid = CSheetId("zorai.race_stats");
		break;
	}

	CRaceStatsSheet *pRSS = dynamic_cast<CRaceStatsSheet*>(SheetMngr.get(RSid));
	if (pRSS == NULL) {
		nlwarning("Cannot find sheet for people:%d male;%d", _Config.Race, bMale);
		return NULL;
	}

	CGenderInfo *pGI;
	if (bMale) {
		pGI = &pRSS->GenderInfos[0];
	} else {
		pGI = &pRSS->GenderInfos[1];
	}

	return pGI;
}
