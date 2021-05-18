/*
 * RyzomRenderingService - https://github.com/nimetu/bmsite_rrs_cpp
 * Copyright (c) 2014 Meelis Mägi <nimetu@gmail.com>
 *
 * This file is part of RyzomRenderingService.
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#include "stdpch.h"
#include "visual_config.h"

void CVisualConfig::serial(NLMISC::IStream & f) {
	if (!f.isReading()) {
		throw "This is only for reading";
	}

	uint8 ver;
	f.serial(ver);
	nldebug("CVisualConfig::serial version %d", ver);

	if (ver < 2) {
		pktFormat = (EPacketFormat) ver;
	} else {
		f.serialShortEnum(pktFormat);
	}

	f.serialShortEnum(Race);

	if (ver >= 2) {
		f.serial(Age);
		if (Age > 2){
			Age = 2;
		}
	} else {
		Age = 0;
	}

	if (pktFormat == PKT_SHORT) {
		nldebug(">> short packet");
		vpa.serial(f);
		vpb.serial(f);
		vpc.serial(f);
	} else {
		nldebug(">> long packet");
		// they say bitfields are compiler and platform specific - this is mainly for php
		uint32 b;
		f.serial(b);
		vpa.PropertySubData.Sex = b;
		f.serial(b);
		vpa.PropertySubData.JacketModel = b;
		f.serial(b);
		vpa.PropertySubData.JacketColor = b;
		f.serial(b);
		vpa.PropertySubData.TrouserModel = b;
		f.serial(b);
		vpa.PropertySubData.TrouserColor = b;
		f.serial(b);
		vpa.PropertySubData.WeaponRightHand = b;
		f.serial(b);
		vpa.PropertySubData.WeaponLeftHand = b;
		f.serial(b);
		vpa.PropertySubData.ArmModel = b;
		f.serial(b);
		vpa.PropertySubData.ArmColor = b;
		f.serial(b);
		vpa.PropertySubData.HatModel = b;
		f.serial(b);
		vpa.PropertySubData.HatColor = b;

		f.serial(b);
		vpb.PropertySubData.Name = b;
		f.serial(b);
		vpb.PropertySubData.HandsModel = b;
		f.serial(b);
		vpb.PropertySubData.HandsColor = b;
		f.serial(b);
		vpb.PropertySubData.FeetModel = b;
		f.serial(b);
		vpb.PropertySubData.FeetColor = b;
		f.serial(b);
		vpb.PropertySubData.RTrail = b;
		f.serial(b);
		vpb.PropertySubData.LTrail = b;

		f.serial(b);
		vpc.PropertySubData.MorphTarget1 = b;
		f.serial(b);
		vpc.PropertySubData.MorphTarget2 = b;
		f.serial(b);
		vpc.PropertySubData.MorphTarget3 = b;
		f.serial(b);
		vpc.PropertySubData.MorphTarget4 = b;
		f.serial(b);
		vpc.PropertySubData.MorphTarget5 = b;
		f.serial(b);
		vpc.PropertySubData.MorphTarget6 = b;
		f.serial(b);
		vpc.PropertySubData.MorphTarget7 = b;
		f.serial(b);
		vpc.PropertySubData.MorphTarget8 = b;
		f.serial(b);
		vpc.PropertySubData.EyesColor = b;
		f.serial(b);
		vpc.PropertySubData.Tattoo = b;
		f.serial(b);
		vpc.PropertySubData.CharacterHeight = b;
		f.serial(b);
		vpc.PropertySubData.TorsoWidth = b;
		f.serial(b);
		vpc.PropertySubData.ArmsWidth = b;
		f.serial(b);
		vpc.PropertySubData.LegsWidth = b;
		f.serial(b);
		vpc.PropertySubData.BreastSize = b;
	}

	// hair color can only come from HEAD_SLOT, so use it and clamp it
	HairColor = vpa.PropertySubData.HatColor;
	NLMISC::clamp(HairColor, (uint) 1, (uint) 6); // H1..H6

	sint32 angle;
	f.serial(angle);
	// convert angle from CCW to CW (clockwise)
	angle = (270 - angle);
	if (angle < 0) {
		angle += 360;
	}
	Theta = angle * 3.14 / 180;
	f.serial(angle);
	Phi = angle * 3.14 / 180;
	f.serial(FaceShot);
	Background.serial(f);
	if (ver >= 2) {
		f.serial(useFx);
	} else {
		useFx = Background.A == 255;
	}

	nldebug(">> serial in");
	nldebug(">> race: %s", EGSPD::CPeople::toString(Race).c_str());
	nldebug(">> age: %d", Age);
	nldebug("VPA: %" NL_I64 "u : Chest(%d,%d) Legs(%d,%d) Arms(%d,%d) Hat(%d,%d) RH(%d) LH(%d)", vpa.PropertyA,
	(uint) vpa.PropertySubData.JacketModel, (uint) vpa.PropertySubData.JacketColor,
	(uint) vpa.PropertySubData.TrouserModel, (uint) vpa.PropertySubData.TrouserColor,
	(uint) vpa.PropertySubData.ArmModel, (uint) vpa.PropertySubData.ArmColor,
	(uint) vpa.PropertySubData.HatModel, (uint) vpa.PropertySubData.HatColor,
	(uint) vpa.PropertySubData.WeaponRightHand,
	(uint) vpa.PropertySubData.WeaponLeftHand);
	nldebug("VPB: %" NL_I64 "u : Hands(%d,%d) Feet(%d,%d).", vpb.PropertyB,
	(uint) vpb.PropertySubData.HandsModel, (uint) vpb.PropertySubData.HandsColor,
	(uint) vpb.PropertySubData.FeetModel, (uint) vpb.PropertySubData.FeetColor);
	nldebug("VPC: %" NL_I64 "u : EyesColor(%d) Tattoo(%d).", vpc.PropertyC,
	(uint) vpc.PropertySubData.EyesColor, (uint) vpc.PropertySubData.Tattoo);
	nldebug("VPC: morph(%d,%d,%d,%d,%d,%d,%d,%d), gabarit(%d,%d,%d,%d,%d)",
	(uint) vpc.PropertySubData.MorphTarget1,
	(uint) vpc.PropertySubData.MorphTarget2,
	(uint) vpc.PropertySubData.MorphTarget3,
	(uint) vpc.PropertySubData.MorphTarget4,
	(uint) vpc.PropertySubData.MorphTarget5,
	(uint) vpc.PropertySubData.MorphTarget6,
	(uint) vpc.PropertySubData.MorphTarget7,
	(uint) vpc.PropertySubData.MorphTarget8,
	(uint) vpc.PropertySubData.CharacterHeight,
	(uint) vpc.PropertySubData.TorsoWidth,
	(uint) vpc.PropertySubData.ArmsWidth,
	(uint) vpc.PropertySubData.LegsWidth,
	(uint) vpc.PropertySubData.BreastSize);
	nldebug(">> angle = %f, %f", Theta, Phi);
	nldebug(">> face shot: %s", FaceShot ? "yes" : "no");
	nldebug(">> use fx: %s", useFx ? "yes" : "no");
}
