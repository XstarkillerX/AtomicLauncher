#include <dlfcn.h>
#include <jni.h>
#include <unistd.h>
#include <sys/mman.h>
#include <substrate.h>
#include <dobby_public.h>
#include <dl_internal.h>
#include <android/log.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include "shared.h"
#include "mcpe.h"
#include "debugmenu/DebugMenu.h"

const std::string ALVERSION = "1.03";

Debug::DebugMenu debugMenu;

static void Gui$render_hook(Gui* gui, float a, bool b, int c, int d) {
	Gui$render_real(gui, a, b, c, d);
	
	if(debug) {
		int pxY = 1;
		font->drawTransformed(debugMenu.getVersionString(), 1, pxY, Color::WHITE, 0, font->getLineLength(debugMenu->getVersionString()), false, 0.8f);
		font->drawTransformed(debugMenu.getCoordsString(), 1, pxY += 8, Color::WHITE, 0, font->getLineLength(debugMenu->getCoordsString()), false, 0.8f);
		font->drawTransformed(debugMenu.getBlockCoordsString(), 1, pxY += 8, Color::WHITE, 0, font->getLineLength(debugMenu->getBlockCoordsString()), false, 0.8f);
		font->drawTransformed(debugMenu.getChunkCoordsString(), 1, pxY += 8, Color::WHITE, 0, font->getLineLength(debugMenu->getChunkCoordsString()), false, 0.8f);
		font->drawTransformed(debugMenu.getFacingString(), 1, pxY += 8, Color::WHITE, 0, font->getLineLength(debugMenu->getFacingString()), false, 0.8f);
		font->drawTransformed(debugMenu.getBiomeString(), 1, pxY += 8, Color::WHITE, 0, font->getLineLength(debugMenugetBiomeString()),  false, 0.8f);
		//font->drawTransformed(debugMenu.getLightString(), 1, pxY += 8, Color::WHITE, 0, font->getLineLength(debugMenu->getLightString()), false, 0.8f);
	}
	if(!debugButton) {
		debugButton = new Button(0, "F3", false);
		debugButton->x = AppPlatform::getScreenWidth() / AppPlatform::getPixelsPerMillimeter() * 3 + 25;
		debugButton ->y = 1;
		debugButton->width = 18;
		debugButton ->height = 18;
	}
	debugButton->render(minecraftClient, 0, 0);
}

static void (*mouseDown_real)(int, int, int);
static void mouseDown_hook(int idk, int x, int y) {
	mouseDown_real(idk, x, y);
	if(debugButton && debugButton->isInside(x / 4, y / 4)) debug = !debug;
}

static void (*MinecraftClient$leaveGame_real)(MinecraftClient*, bool, bool);
static void MinecraftClient$leaveGame_hook(MinecraftClient* mc, bool a, bool b) {
	MinecraftClient$leaveGame_real(mc, a, b);
	debugButton = NULL;
}

static void (*Font$Font_real)(Font*, Options*, std::string const&, Textures*);
static void Font$Font_hook(Font* fnt, Options* op, std::string const& str, Textures* tex) {
	Font$Font_real(fnt, op, str, tex);
	font = fnt;
}

static void (*Level$onSourceCreated_real)(Level*, TileSource*);
static void Level$onSourceCreated_hook(Level* lvl, TileSource* ts) {
	Level$onSourceCreated_real(lvl, ts);
	debugMenu = Debug::DebugMenu(lvl, ts);
}

static void (*Gui$Gui_real)(Gui*, MinecraftClient*);
static void Gui$Gui_hook(Gui* gui, MinecraftClient* mc) {
	Gui$Gui_real(gui, mc);
	minecraftClient = mc;debu
}

static void(*CreativeInventoryScreen$populateItem_real)(Item*, int, int);
static void CreativeInventoryScreen$populateItem_hook(Item* item, int count, int damage) {
	CreativeInventoryScreen$populateItem_real(item, count, damage);
	if(item == Item::items[383] && damage == 40) {
		CreativeInventoryScreen$populateItem_real(Item::items[383], 1, 41);
		CreativeInventoryScreen$populateItem_real(Item::items[383], 1, 42);
	}
}

static std::string(*Common$getGameVersionString_real)();
static std::string Common$getGameVersionString_hook(){
	return "AtomicLauncher v" + ALVERSION;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
	void *handle = dlopen("libminecraftpe.so", RTLD_LAZY);
	
	void* Font$$Font = dlsym(handle, "_ZN4FontC1EP7OptionsRKSsP8Textures");
	void* Gui$$Gui = dlsym(handle, "_ZN3GuiC2ER15MinecraftClient");
	MSHookFunction(Font$$Font, (void*) &Font$Font_hook, (void**) &Font$Font_real);
	MSHookFunction(Gui$$Gui, (void*) &Gui$Gui_hook, (void**) &Gui$Gui_real);
	MSHookFunction((void*) &mouseDown, (void*) &mouseDown_hook, (void**) &mouseDown_real);
	MSHookFunction((void*) &Gui::render, (void*) &Gui$render_hook, (void**) &Gui$render_real);
	MSHookFunction((void*) &Common::getGameVersionString, (void*) &Common$getGameVersionString_hook, (void*) Common$getGameVersionString_real);
	MSHookFunction((void*) &Level::onSourceCreated, (void*) &Level$onSourceCreated_hook, (void**) &Level$onSourceCreated_real);
	//MSHookFunction((void*) CreativeInventoryScreen::populateItem, (void*) &CreativeInventoryScreen$populateItem_hook, (void**) &CreativeInventoryScreen$populateItem_real);
	
	return JNI_VERSION_1_2;
}
