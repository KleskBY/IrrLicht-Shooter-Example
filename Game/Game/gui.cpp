#include "gui.h"
#include "main.h"

void RenderGui()
{
	IGUIEnvironment* gui = device->getGUIEnvironment();
	ITexture* bgTexture = driver->getTexture("background.jpg");
	IGUISkin* skin = gui->getSkin();
	IGUIFont* font = gui->getFont("data/gfx/myfont.xml");
	if (font) skin->setFont(font);
	device->getCursorControl()->setVisible(true);
	device->getCursorControl()->setActiveIcon(irr::gui::ECI_NORMAL);  // Use normal free cursor
	int x = 50, y = 100, h = 35;
	gui->addButton(rect<s32>(x, y, x + 200, y + h), 0, GUI_ID_NEW_GAME, L"New Game");
	y += h + 10;
	gui->addButton(rect<s32>(x, y, x + 200, y + h), 0, GUI_ID_LOAD_GAME, L"Load Game");
	y += h + 10;
	gui->addButton(rect<s32>(x, y, x + 200, y + h), 0, GUI_ID_FIND_SERVERS, L"Find Servers");
	y += h + 10;
	gui->addButton(rect<s32>(x, y, x + 200, y + h), 0, GUI_ID_CREATE_SERVER, L"Create Server");
	y += h + 10;
	gui->addButton(rect<s32>(x, y, x + 200, y + h), 0, GUI_ID_CHANGE_GAME, L"Change Game");
	y += h + 10;
	gui->addButton(rect<s32>(x, y, x + 200, y + h), 0, GUI_ID_OPTIONS, L"Options");
	y += h + 10;
	gui->addButton(rect<s32>(x, y, x + 200, y + h), 0, GUI_ID_QUIT, L"Quit");
	while (device->run() && driver) 
	{
		driver->beginScene(true, true, SColor(255, 0, 0, 0));
		if (bgTexture) driver->draw2DImage(bgTexture, position2d<s32>(0, 0), rect<s32>(0, 0, bgTexture->getOriginalSize().Width, bgTexture->getOriginalSize().Height), 0, 0, true);
		gui->drawAll();
		driver->endScene();
	}
}