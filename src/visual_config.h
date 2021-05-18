/*
 * RyzomRenderingService - https://github.com/nimetu/rrs_server
 * Copyright (c) 2014 Meelis Mägi <nimetu@gmail.com>
 *
 * This file is part of RyzomRenderingService.
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#ifndef _VISUAL_CONFIG_H
#define	_VISUAL_CONFIG_H

#include "stdpch.h"

class CVisualConfig {
public:

	enum EPacketFormat {
		PKT_SHORT = 0, PKT_LONG = 1
	};
private:
	EPacketFormat pktFormat;
public:
	EGSPD::CPeople::TPeople Race;
	uint8 Age;

	SPropVisualA vpa; // sex, jacket, trouser, weapon(r+l), arm, hat
	SPropVisualB vpb; // name, Hands, Feet, RTrail, LTrail
	SPropVisualC vpc; // Morph, EyesColor, Tattoo, Gabarit

	// horizontal angle
	float Theta;
	// vertical angle
	float Phi;

	bool FaceShot;

	NLMISC::CRGBA Background;
	bool useFx;

	// not serialized, updated on serial in
	uint HairColor;

	void serial(NLMISC::IStream & f);
};

#endif	/* _VISUAL_CONFIG_H */
