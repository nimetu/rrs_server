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
#include "renderer.h"
#include "visual_config.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

/**
 * globals
 */
NL3D::UDriver *Driver;
NL3D::UScene *Scene;
NL3D::UCamera Camera;
NL3D::UMaterial GenericMat;

// bind port
uint32 ListenPort = 25000;

// exit after # of requests
uint32 nbLimitRequests = 0;
uint32 nbRequests = 0;

// force background removal, else just reset alpha channel for no BG color
bool RemoveBackground = false;

//
std::string PackedSheetsPath = "";

//
CCallbackServer *Clients = 0;
CRenderer *Renderer = 0;

NLNET::TCallbackItem ClientCallbackArray[] = { { "RENDER", cbRender } };

void CRenderService::init() {
#ifndef NL_DEBUG
	INelContext::getInstance().getDebugLog()->removeDisplayer("DEFAULT_SD");
	//INelContext::getInstance().getInfoLog()->removeDisplayer("DEFAULT_SD");
	INelContext::getInstance().getWarningLog()->removeDisplayer("DEFAULT_SD");
#endif // NL_DEBUG

	/**
	 * config file
	 */
	ListenPort = 25000;
	nbLimitRequests = 0;
	RemoveBackground = false;

	PackedSheetsPath = CFile::getPath(CPath::lookup("race_stats.packed_sheets", false, true, true));
	if (!PackedSheetsPath.empty()) {
		nlinfo("Using default value for PackedSheetsPath: race_stats.packed_sheets found in (%s)", PackedSheetsPath.c_str());
	}

	CConfigFile::CVar *var = NULL;
	var = ConfigFile.getVarPtr("ListenPort");
	if (var != NULL && var->asInt() > 0) {
		ListenPort = var->asInt();
	}

	var = ConfigFile.getVarPtr("LimitRequest");
	if (var != NULL && var->asInt() > 0) {
		nbLimitRequests = var->asInt();
	}

	var = ConfigFile.getVarPtr("RemoveBackground");
	if (var != NULL) {
		RemoveBackground = var->asBool();
	}

	var = ConfigFile.getVarPtr("PackedSheetsPath");
	if (var != NULL) {
		PackedSheetsPath = var->asString();
	}

	CPath::remapExtension("dds", "tga", true);

	/**
	 * command line arguments
	 */
	// --dumpvs
	dumpVSIndex = false;
	if (haveLongArg("dumpvs")) {
		nlinfo("--dumpvs detected");
		dumpVSIndex = true;
	}

	// --listen=25000
	if (haveLongArg("listen")) {
		uint32 port;
		NLMISC::fromString(getLongArg("listen"), port);
		if (port <= 0) {
			nlinfo("invalid port %d, fallback to default %d", port, ListenPort);
		} else {
			ListenPort = port;
		}
		nlinfo("Listen on port %d", ListenPort);
	}

	// --limit=100
	if (haveLongArg("limit")) {
		uint32 limit;
		NLMISC::fromString(getLongArg("limit"), limit);
		if ((sint) limit < 0) {
			nlwarning("invalid request limit %d, fallback to default %d", limit, nbLimitRequests);
		} else {
			nbLimitRequests = limit;
		}
	}

	/**
	 * start
	 */
	nlinfo("Init CSheetId...");
	CSheetId::init(false);

	nlinfo("Loading sheets...");
	std::vector<std::string> exts;
	//userExtensions.push_back("creature");
	exts.push_back("race_stats");
	exts.push_back("sitem");
	exts.push_back("item");
	//exts.push_back("animation_fx");

	IProgressCallback callback;
	SheetMngr.setOutputDataPath(PackedSheetsPath);
	SheetMngr.loadAllSheet(callback, false, false, dumpVSIndex, false, &exts);

	if (dumpVSIndex) {
		nlinfo("vs_index.txt is created, exiting");
	} else {
		nldebug("Color slot manager");
		initColorSlotManager();

		// create rendering instance
		nldebug("Renderer init");
		Renderer = new CRenderer(IService::ConfigFile);

		// server where 'clients' are connecting
		nldebug("CCallbackServer init, bind to port %d", ListenPort);
		Clients = new CCallbackServer();
		nlassert(Clients != 0);

		Clients->addCallbackArray(ClientCallbackArray, sizeof(ClientCallbackArray) / sizeof(ClientCallbackArray[0]));
		Clients->init(ListenPort);
	}
}

bool CRenderService::update() {
	// if we only dump vs_index.txt
	if (dumpVSIndex) {
		return false;
	}

	Clients->update();

	bool stillAlive = Renderer->idle();
	if (!stillAlive) {
		nlinfo("Renderer went away");
		return false;
	}

	if (nbLimitRequests > 0 && nbRequests > nbLimitRequests) {
		nlinfo("Shutting down because request limit reached (%d)", nbLimitRequests);
		return false;
	}

	return true;
}

void CRenderService::release() {
	if (!dumpVSIndex) {
		nlinfo("CArmoryService::release()");
		delete Clients;
		Clients = NULL;

		delete Renderer;
		Renderer = NULL;
	}

	CVisualSlotManager::releaseInstance();
	CSheetId::uninit();
	SheetMngr.release();
}

void cbRender(CMessage &msgin, TSockId from, CCallbackNetBase &clientcb) {
	nbRequests++;
	nldebug(">> cbRender - %d / %d request, connection from %s", nbRequests, nbLimitRequests, from->asString().c_str());

	try {
		CVisualConfig visual;
		visual.serial(msgin);

		TTime timerSetupStart = NLMISC::CTime::getLocalTime();
		if (!Renderer->setupScene(visual)) {
			throw "Renderer:setupScene failed";
		}

		CBitmap btm;
		TTime timerRenderStart = NLMISC::CTime::getLocalTime();
		if (!Renderer->renderInto(btm)) {
			throw "Renderer: renderInto failed";
		}

		TTime timerPngStart = NLMISC::CTime::getLocalTime();
		CMessage msgout("PNG");
		btm.writePNG(msgout, 32);

		TTime timerSendStart = NLMISC::CTime::getLocalTime();
		Clients->send(msgout, from);

		TTime timerEnd = NLMISC::CTime::getLocalTime();

		nldebug("Timers: ");
		nldebug("   setup : %dms", timerRenderStart - timerSetupStart);
		nldebug("  render : %dms", timerPngStart - timerRenderStart);
		nldebug("writePNG : %dms", timerSendStart - timerPngStart);
		nldebug("----------------");
		nldebug("   total : %dms", timerEnd - timerSetupStart);
	} catch (const Exception & e) {
		CMessage msgout("ERROR");
		std::string msg = e.what();
		msgout.serial(msg);
		Clients->send(msgout, from);
		nlwarning("Packet decoding failed (%s)", msg.c_str());
	} catch (char const *e) {
		CMessage msgout("ERROR");
		std::string msg = e;
		msgout.serial(msg);
		Clients->send(msgout, from);
		nlwarning("Packet decoding failed (%s)", e);
	}
}

NLNET_SERVICE_MAIN(CRenderService, "RRS", "render_service", 0, EmptyCallbackArray, "", "");
