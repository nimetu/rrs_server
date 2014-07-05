/*
 * RyzomRenderingService - https://github.com/nimetu/rrs_server
 * Copyright (c) 2014 Meelis Mägi <nimetu@gmail.com>
 *
 * This file is part of RyzomRenderingService.
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#ifndef _MAIN_H
#define	_MAIN_H

#include "stdpch.h"

#define NB_MORPH_TARGETS 8

extern NL3D::UDriver *Driver;
extern NL3D::UScene *Scene;
extern NL3D::UCamera Camera;
extern NL3D::UMaterial GenericMat;

extern uint32 ListenPort;
extern uint32 nbLimitRequests;
extern uint32 nbRequests;
extern bool RemoveBackground;
extern std::string PackedSheetsPath;

class CRenderService: public NLNET::IService {
private:
	bool dumpVSIndex;

public:

	void init();
	bool update();
	void release();
};

void cbRender(NLNET::CMessage &msgin, NLNET::TSockId from, NLNET::CCallbackNetBase &clientcb);

#endif	/* _MAIN_H */

