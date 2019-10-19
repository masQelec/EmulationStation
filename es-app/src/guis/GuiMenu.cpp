#include "guis/GuiMenu.h"

#include "components/OptionListComponent.h"
#include "components/SliderComponent.h"
#include "components/SwitchComponent.h"
#include "guis/GuiCollectionSystemsOptions.h"
#include "guis/GuiDetectDevice.h"
#include "guis/GuiGeneralScreensaverOptions.h"
#include "guis/GuiMsgBox.h"
#include "guis/GuiScraperStart.h"
#include "guis/GuiSettings.h"
#include "views/UIModeController.h"
#include "views/ViewController.h"
#include "CollectionSystemManager.h"
#include "EmulationStation.h"
#include "Scripting.h"
#include "SystemData.h"
#include "VolumeControl.h"
#include <SDL_events.h>
#include <algorithm>
#include "platform.h"
#include "utils/FileSystemUtil.h" /* < emuelec */

GuiMenu::GuiMenu(Window* window) : GuiComponent(window), mMenu(window, "MAIN MENU"), mVersion(window)
{
	bool isFullUI = UIModeController::getInstance()->isUIModeFull();

	if (isFullUI)
	    addEntry("EmuELEC", 0x777777FF, true, [this] { openEmuELECSettings(); }); /* < emuelec */
	
	if (isFullUI)
		addEntry("RASPADOR", 0x777777FF, true, [this] { openScraperSettings(); });

	addEntry("AJUSTES DE SONIDO", 0x777777FF, true, [this] { openSoundSettings(); });

	if (isFullUI)
		addEntry("AJUSTES DE INTERFAZ", 0x777777FF, true, [this] { openUISettings(); });

	if (isFullUI)
		addEntry("AJUSTES DE COLECCIÓN DE JUEGO", 0x777777FF, true, [this] { openCollectionSystemSettings(); });

	if (isFullUI)
		addEntry("OTROS AJUSTES", 0x777777FF, true, [this] { openOtherSettings(); });

	if (isFullUI)
		addEntry("CONFIGURAR ENTRADA", 0x777777FF, true, [this] { openConfigInput(); });

	addEntry("QUITAR", 0x777777FF, true, [this] {openQuitMenu(); });

	addChild(&mMenu);
	addVersionInfo();
	setSize(mMenu.getSize());
	setPosition((Renderer::getScreenWidth() - mSize.x()) / 2, Renderer::getScreenHeight() * 0.15f);
}
/* < emuelec */
void GuiMenu::openEmuELECSettings()
{
	auto s = new GuiSettings(mWindow, "EmuELEC Settings");
	Window* window = mWindow;
	std::string a;
	    auto bgm_enabled = std::make_shared<SwitchComponent>(mWindow);
		bgm_enabled->setState(Settings::getInstance()->getBool("BGM"));
		s->addWithLabel("ACTIVAR BGM", bgm_enabled);
		s->addSaveFunc([bgm_enabled] {
			if (bgm_enabled->getState() == false) {
				runSystemCommand("bash /storage/.emulationstation/scripts/bgm.sh stop"); 
				} else { 
				runSystemCommand("bash /storage/.emulationstation/scripts/bgm.sh start");
			}
                Settings::getInstance()->setBool("BGM", bgm_enabled->getState());
			});

		auto emuelec_bgm_boot_def = std::make_shared< OptionListComponent<std::string> >(mWindow, "INICIAR BGM EN EL ARRANQUE", false);
		std::vector<std::string> bgmboot;
		bgmboot.push_back("Yes");
		bgmboot.push_back("No");
		for (auto it = bgmboot.cbegin(); it != bgmboot.cend(); it++)
		emuelec_bgm_boot_def->add(*it, *it, Settings::getInstance()->getString("EmuELEC_BGM_BOOT") == *it);
		s->addWithLabel("START BGM AT BOOT", emuelec_bgm_boot_def);
		s->addSaveFunc([emuelec_bgm_boot_def] {
			/*runSystemCommand("echo "+emuelec_bgm_boot_def->getSelected()+" > /storage/.config/def_fe");*/
			if (Settings::getInstance()->getString("EmuELEC_BGM_BOOT") != emuelec_bgm_boot_def->getSelected())
				Settings::getInstance()->setString("EmuELEC_BGM_BOOT",  emuelec_bgm_boot_def->getSelected());
		});
		
       auto bezels_enabled = std::make_shared<SwitchComponent>(mWindow);
		bezels_enabled->setState(Settings::getInstance()->getBool("EmuELEC_BEZELS"));
		s->addWithLabel("ACTIVAR RA BEZELS", bezels_enabled);
		s->addSaveFunc([bezels_enabled] {
			/* if (bezels_enabled->getState() == false) {
				runSystemCommand("/emuelec/scripts/enable.sh bezels disable"); 
				} else { 
				runSystemCommand("/emuelec/scripts/enable.sh bezels enable"); 
			} */
                Settings::getInstance()->setBool("EmuELEC_BEZELS", bezels_enabled->getState());
			});	
       
       auto splash_enabled = std::make_shared<SwitchComponent>(mWindow);
		splash_enabled->setState(Settings::getInstance()->getBool("EmuELEC_SPLASH"));
		s->addWithLabel("ACTIVAR RA SPLASH", splash_enabled);
		s->addSaveFunc([splash_enabled] {
		/*	if (splash_enabled->getState() == false) {
				runSystemCommand("/emuelec/scripts/enable.sh splash disable"); 
				} else { 
				runSystemCommand("/emuelec/scripts/enable.sh splash enable"); 
			} */
                Settings::getInstance()->setBool("EmuELEC_SPLASH", splash_enabled->getState());
			});

	ComponentListRow row;
	
	row.addElement(std::make_shared<TextComponent>(mWindow, "                                   OPCIONES DEL EMULADOR", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	s->addRow(row);
	row.elements.clear();
	
	std::vector<std::string> emuchoices;
	
	/* CHOICE */
	auto emuelec_amiga_def = std::make_shared< OptionListComponent<std::string> >(mWindow, "AMIGA", false);
         emuchoices.clear();  
    for(std::stringstream ss(getShOutput(R"(~/.emulationstation/scripts/getcores.sh amiga)")); getline(ss, a, ','); )
        emuchoices.push_back(a);
    
		for (auto it = emuchoices.cbegin(); it != emuchoices.cend(); it++) {
		emuelec_amiga_def->add(*it, *it, Settings::getInstance()->getString("EmuELEC_AMIGA_CORE") == *it); }
		s->addWithLabel("AMIGA", emuelec_amiga_def);
		s->addSaveFunc([emuelec_amiga_def] {
			if (Settings::getInstance()->getString("EmuELEC_AMIGA_CORE") != emuelec_amiga_def->getSelected())
				Settings::getInstance()->setString("EmuELEC_AMIGA_CORE", emuelec_amiga_def->getSelected());
		});
	/* END CHOICE */
	/* CHOICE */
	auto emuelec_arcade_def = std::make_shared< OptionListComponent<std::string> >(mWindow, "ARCADE", false);
         emuchoices.clear();  
    for(std::stringstream ss(getShOutput(R"(~/.emulationstation/scripts/getcores.sh arcade)")); getline(ss, a, ','); )
        emuchoices.push_back(a);
    
		for (auto it = emuchoices.cbegin(); it != emuchoices.cend(); it++) {
		emuelec_arcade_def->add(*it, *it, Settings::getInstance()->getString("EmuELEC_ARCADE_CORE") == *it); }
		s->addWithLabel("ARCADE", emuelec_arcade_def);
		s->addSaveFunc([emuelec_arcade_def] {
			if (Settings::getInstance()->getString("EmuELEC_ARCADE_CORE") != emuelec_arcade_def->getSelected())
				Settings::getInstance()->setString("EmuELEC_ARCADE_CORE", emuelec_arcade_def->getSelected());
		});
	/* END CHOICE */
	/* CHOICE */
	auto emuelec_dosbox_def = std::make_shared< OptionListComponent<std::string> >(mWindow, "DOSBOX", false);
         emuchoices.clear();  
    for(std::stringstream ss(getShOutput(R"(~/.emulationstation/scripts/getcores.sh dosbox)")); getline(ss, a, ','); )
        emuchoices.push_back(a);
    
		for (auto it = emuchoices.cbegin(); it != emuchoices.cend(); it++) {
		emuelec_dosbox_def->add(*it, *it, Settings::getInstance()->getString("EmuELEC_DOSBOX_CORE") == *it); }
		s->addWithLabel("DOSBOX", emuelec_dosbox_def);
		s->addSaveFunc([emuelec_dosbox_def] {
			if (Settings::getInstance()->getString("EmuELEC_DOSBOX_CORE") != emuelec_dosbox_def->getSelected())
				Settings::getInstance()->setString("EmuELEC_DOSBOX_CORE", emuelec_dosbox_def->getSelected());
		});
	/* END CHOICE */
	/* CHOICE */
	auto emuelec_hatari_def = std::make_shared< OptionListComponent<std::string> >(mWindow, "HATARI", false);
         emuchoices.clear();  
    for(std::stringstream ss(getShOutput(R"(~/.emulationstation/scripts/getcores.sh hatari)")); getline(ss, a, ','); )
        emuchoices.push_back(a);
    
		for (auto it = emuchoices.cbegin(); it != emuchoices.cend(); it++) {
		emuelec_hatari_def->add(*it, *it, Settings::getInstance()->getString("EmuELEC_HATARI_CORE") == *it); }
		s->addWithLabel("HATARI", emuelec_hatari_def);
		s->addSaveFunc([emuelec_hatari_def] {
			if (Settings::getInstance()->getString("EmuELEC_HATARI_CORE") != emuelec_hatari_def->getSelected())
				Settings::getInstance()->setString("EmuELEC_HATARI_CORE", emuelec_hatari_def->getSelected());
		});
	/* END CHOICE */
	/* CHOICE */
	auto emuelec_mame_def = std::make_shared< OptionListComponent<std::string> >(mWindow, "MAME", false);
         emuchoices.clear();  
    for(std::stringstream ss(getShOutput(R"(~/.emulationstation/scripts/getcores.sh mame)")); getline(ss, a, ','); )
        emuchoices.push_back(a);
    
		for (auto it = emuchoices.cbegin(); it != emuchoices.cend(); it++) {
		emuelec_mame_def->add(*it, *it, Settings::getInstance()->getString("EmuELEC_MAME_CORE") == *it); }
		s->addWithLabel("MAME", emuelec_mame_def);
		s->addSaveFunc([emuelec_mame_def] {
			if (Settings::getInstance()->getString("EmuELEC_MAME_CORE") != emuelec_mame_def->getSelected())
				Settings::getInstance()->setString("EmuELEC_MAME_CORE", emuelec_mame_def->getSelected());
		});
	/* END CHOICE */
	/* CHOICE */
	auto emuelec_fbn_def = std::make_shared< OptionListComponent<std::string> >(mWindow, "NEO-GEO", false);
         emuchoices.clear(); 
    for(std::stringstream ss(getShOutput(R"(~/.emulationstation/scripts/getcores.sh fbn)")); getline(ss, a, ','); ) 
        emuchoices.push_back(a);
    
	    for (auto it = emuchoices.cbegin(); it != emuchoices.cend(); it++) {
		emuelec_fbn_def->add(*it, *it, Settings::getInstance()->getString("EmuELEC_FBN_CORE") == *it); }
		s->addWithLabel("NEO-GEO", emuelec_fbn_def);
		s->addSaveFunc([emuelec_fbn_def] {
			if (Settings::getInstance()->getString("EmuELEC_FBN_CORE") != emuelec_fbn_def->getSelected())
				Settings::getInstance()->setString("EmuELEC_FBN_CORE", emuelec_fbn_def->getSelected());
		});
	/* END CHOICE */
	/* CHOICE */
	auto emuelec_neocd_def = std::make_shared< OptionListComponent<std::string> >(mWindow, "NEO-GEO CD", false);
         emuchoices.clear();  
    for(std::stringstream ss(getShOutput(R"(~/.emulationstation/scripts/getcores.sh neocd)")); getline(ss, a, ','); )
        emuchoices.push_back(a);
    
		for (auto it = emuchoices.cbegin(); it != emuchoices.cend(); it++) {
		emuelec_neocd_def->add(*it, *it, Settings::getInstance()->getString("EmuELEC_NEOCD_CORE") == *it); }
		s->addWithLabel("NEO-GEO CD", emuelec_neocd_def);
		s->addSaveFunc([emuelec_neocd_def] {
			if (Settings::getInstance()->getString("EmuELEC_NEOCD_CORE") != emuelec_neocd_def->getSelected())
				Settings::getInstance()->setString("EmuELEC_NEOCD_CORE", emuelec_neocd_def->getSelected());
		});
	/* END CHOICE */
	/* CHOICE */
	auto emuelec_gba_def = std::make_shared< OptionListComponent<std::string> >(mWindow, "NINTENDO GBA", false);
         emuchoices.clear(); 
    for(std::stringstream ss(getShOutput(R"(~/.emulationstation/scripts/getcores.sh gba)")); getline(ss, a, ','); ) 
        emuchoices.push_back(a);
    
	    for (auto it = emuchoices.cbegin(); it != emuchoices.cend(); it++) {
		emuelec_gba_def->add(*it, *it, Settings::getInstance()->getString("EmuELEC_GBA_CORE") == *it); }
		s->addWithLabel("NINTENDO GBA", emuelec_gba_def);
		s->addSaveFunc([emuelec_gba_def] {
			if (Settings::getInstance()->getString("EmuELEC_GBA_CORE") != emuelec_gba_def->getSelected())
				Settings::getInstance()->setString("EmuELEC_GBA_CORE", emuelec_gba_def->getSelected());
		});
	/* END CHOICE */
	/* CHOICE */
	auto emuelec_gbc_def = std::make_shared< OptionListComponent<std::string> >(mWindow, "NINTENDO GB/GBC", false);
         emuchoices.clear(); 
    for(std::stringstream ss(getShOutput(R"(~/.emulationstation/scripts/getcores.sh gbc)")); getline(ss, a, ','); ) 
        emuchoices.push_back(a);
    
	    for (auto it = emuchoices.cbegin(); it != emuchoices.cend(); it++) {
		emuelec_gbc_def->add(*it, *it, Settings::getInstance()->getString("EmuELEC_GBC_CORE") == *it); }
		s->addWithLabel("NINTENDO GB/GBC", emuelec_gbc_def);
		s->addSaveFunc([emuelec_gbc_def] {
			if (Settings::getInstance()->getString("EmuELEC_GBC_CORE") != emuelec_gbc_def->getSelected())
				Settings::getInstance()->setString("EmuELEC_GBC_CORE", emuelec_gbc_def->getSelected());
		});
	/* END CHOICE */
	/* CHOICE */
	auto emuelec_n64_def = std::make_shared< OptionListComponent<std::string> >(mWindow, "NINTENDO N64", false);
         emuchoices.clear(); 
    for(std::stringstream ss(getShOutput(R"(~/.emulationstation/scripts/getcores.sh n64)")); getline(ss, a, ','); ) 
        emuchoices.push_back(a);
    
	    for (auto it = emuchoices.cbegin(); it != emuchoices.cend(); it++) {
		emuelec_n64_def->add(*it, *it, Settings::getInstance()->getString("EmuELEC_N64_CORE") == *it); }
		s->addWithLabel("NINTENDO N64", emuelec_n64_def);
		s->addSaveFunc([emuelec_n64_def] {
			if (Settings::getInstance()->getString("EmuELEC_N64_CORE") != emuelec_n64_def->getSelected())
				Settings::getInstance()->setString("EmuELEC_N64_CORE", emuelec_n64_def->getSelected());
		});
	/* END CHOICE */
	/* CHOICE */
	auto emuelec_nes_def = std::make_shared< OptionListComponent<std::string> >(mWindow, "NINTENDO NES/FC/FDS", false);
         emuchoices.clear(); 
    for(std::stringstream ss(getShOutput(R"(~/.emulationstation/scripts/getcores.sh nes)")); getline(ss, a, ','); ) 
        emuchoices.push_back(a);
    
	    for (auto it = emuchoices.cbegin(); it != emuchoices.cend(); it++) {
		emuelec_nes_def->add(*it, *it, Settings::getInstance()->getString("EmuELEC_NES_CORE") == *it); }
		s->addWithLabel("NINTENDO NES/FC/FDS", emuelec_nes_def);
		s->addSaveFunc([emuelec_nes_def] {
			if (Settings::getInstance()->getString("EmuELEC_NES_CORE") != emuelec_nes_def->getSelected())
				Settings::getInstance()->setString("EmuELEC_NES_CORE", emuelec_nes_def->getSelected());
		});
	/* END CHOICE */
	/* CHOICE */
	auto emuelec_snes_def = std::make_shared< OptionListComponent<std::string> >(mWindow, "NINTENDO SNES", false);
         emuchoices.clear(); 
    for(std::stringstream ss(getShOutput(R"(~/.emulationstation/scripts/getcores.sh snes)")); getline(ss, a, ','); ) 
        emuchoices.push_back(a);
    
	    for (auto it = emuchoices.cbegin(); it != emuchoices.cend(); it++) {
		emuelec_snes_def->add(*it, *it, Settings::getInstance()->getString("EmuELEC_SNES_CORE") == *it); }
		s->addWithLabel("NINTENDO SNES", emuelec_snes_def);
		s->addSaveFunc([emuelec_snes_def] {
			if (Settings::getInstance()->getString("EmuELEC_SNES_CORE") != emuelec_snes_def->getSelected())
				Settings::getInstance()->setString("EmuELEC_SNES_CORE", emuelec_snes_def->getSelected());
		});
	/* END CHOICE */
	/* CHOICE */
	auto emuelec_scummvm_def = std::make_shared< OptionListComponent<std::string> >(mWindow, "Scummvm", false);
         emuchoices.clear();  
    for(std::stringstream ss(getShOutput(R"(~/.emulationstation/scripts/getcores.sh scummvm)")); getline(ss, a, ','); )
        emuchoices.push_back(a);
    
		for (auto it = emuchoices.cbegin(); it != emuchoices.cend(); it++) {
		emuelec_scummvm_def->add(*it, *it, Settings::getInstance()->getString("EmuELEC_SCUMMVM_CORE") == *it); }
		s->addWithLabel("Scummvm", emuelec_scummvm_def);
		s->addSaveFunc([emuelec_scummvm_def] {
			if (Settings::getInstance()->getString("EmuELEC_SCUMMVM_CORE") != emuelec_scummvm_def->getSelected())
				Settings::getInstance()->setString("EmuELEC_SCUMMVM_CORE", emuelec_scummvm_def->getSelected());
		});
	/* END CHOICE */
	/* CHOICE */
	auto emuelec_dreamcast_def = std::make_shared< OptionListComponent<std::string> >(mWindow, "SEGA DREAMCAST", false);
         emuchoices.clear();  
    for(std::stringstream ss(getShOutput(R"(~/.emulationstation/scripts/getcores.sh dreamcast)")); getline(ss, a, ','); )
        emuchoices.push_back(a);
    
		for (auto it = emuchoices.cbegin(); it != emuchoices.cend(); it++) {
		emuelec_dreamcast_def->add(*it, *it, Settings::getInstance()->getString("EmuELEC_REICAST_CORE") == *it); }
		s->addWithLabel("SEGA DREAMCAST", emuelec_dreamcast_def);
		s->addSaveFunc([emuelec_dreamcast_def] {
			if (Settings::getInstance()->getString("EmuELEC_REICAST_CORE") != emuelec_dreamcast_def->getSelected())
				Settings::getInstance()->setString("EmuELEC_REICAST_CORE", emuelec_dreamcast_def->getSelected());
		});
	/* END CHOICE */
	/* CHOICE */
	auto emuelec_gen_def = std::make_shared< OptionListComponent<std::string> >(mWindow, "SEGA GEN/MD/CD", false);
         emuchoices.clear(); 
    for(std::stringstream ss(getShOutput(R"(~/.emulationstation/scripts/getcores.sh genesis)")); getline(ss, a, ','); ) 
        emuchoices.push_back(a);
    
	    for (auto it = emuchoices.cbegin(); it != emuchoices.cend(); it++) {
		emuelec_gen_def->add(*it, *it, Settings::getInstance()->getString("EmuELEC_GEN_CORE") == *it); }
		s->addWithLabel("SEGA GEN/MD/CD", emuelec_gen_def);
		s->addSaveFunc([emuelec_gen_def] {
			if (Settings::getInstance()->getString("EmuELEC_GEN_CORE") != emuelec_gen_def->getSelected())
				Settings::getInstance()->setString("EmuELEC_GEN_CORE", emuelec_gen_def->getSelected());
		});
	/* END CHOICE */
	/* CHOICE */
	auto emuelec_sms_def = std::make_shared< OptionListComponent<std::string> >(mWindow, "SEGA MS", false);
         emuchoices.clear();  
    for(std::stringstream ss(getShOutput(R"(~/.emulationstation/scripts/getcores.sh sms)")); getline(ss, a, ','); )
        emuchoices.push_back(a);
    
		for (auto it = emuchoices.cbegin(); it != emuchoices.cend(); it++) {
		emuelec_sms_def->add(*it, *it, Settings::getInstance()->getString("EmuELEC_SMS_CORE") == *it); }
		s->addWithLabel("SEGA MS", emuelec_sms_def);
		s->addSaveFunc([emuelec_sms_def] {
			if (Settings::getInstance()->getString("EmuELEC_SMS_CORE") != emuelec_sms_def->getSelected())
				Settings::getInstance()->setString("EmuELEC_SMS_CORE", emuelec_sms_def->getSelected());
		});
	/* END CHOICE */
	/* CHOICE */
	auto emuelec_psp_def = std::make_shared< OptionListComponent<std::string> >(mWindow, "SONY PSP", false);
         emuchoices.clear(); 
    for(std::stringstream ss(getShOutput(R"(~/.emulationstation/scripts/getcores.sh psp)")); getline(ss, a, ','); ) 
        emuchoices.push_back(a);
    
	    for (auto it = emuchoices.cbegin(); it != emuchoices.cend(); it++) {
		emuelec_psp_def->add(*it, *it, Settings::getInstance()->getString("EmuELEC_PSP_CORE") == *it); }
		s->addWithLabel("SONY PSP", emuelec_psp_def);
		s->addSaveFunc([emuelec_psp_def] {
			if (Settings::getInstance()->getString("EmuELEC_PSP_CORE") != emuelec_psp_def->getSelected())
				Settings::getInstance()->setString("EmuELEC_PSP_CORE", emuelec_psp_def->getSelected());
		});
	/* END CHOICE */
   
   mWindow->pushGui(s);

}
/*  emuelec >*/

void GuiMenu::openScraperSettings()
{
	auto s = new GuiSettings(mWindow, "SCRAPER");

	// scrape from
	auto scraper_list = std::make_shared< OptionListComponent< std::string > >(mWindow, "RASPADO DE", false);
	std::vector<std::string> scrapers = getScraperList();

	// Select either the first entry of the one read from the settings, just in case the scraper from settings has vanished.
	for(auto it = scrapers.cbegin(); it != scrapers.cend(); it++)
		scraper_list->add(*it, *it, *it == Settings::getInstance()->getString("Scraper"));

	s->addWithLabel("RASPADO DE", scraper_list);
	s->addSaveFunc([scraper_list] { Settings::getInstance()->setString("Scraper", scraper_list->getSelected()); });

	// scrape ratings
	auto scrape_ratings = std::make_shared<SwitchComponent>(mWindow);
	scrape_ratings->setState(Settings::getInstance()->getBool("ScrapeRatings"));
	s->addWithLabel("RASPADO DE CLASIFICACIONES", scrape_ratings);
	s->addSaveFunc([scrape_ratings] { Settings::getInstance()->setBool("ScrapeRatings", scrape_ratings->getState()); });

	// scrape now
	ComponentListRow row;
	auto openScrapeNow = [this] { mWindow->pushGui(new GuiScraperStart(mWindow)); };
	std::function<void()> openAndSave = openScrapeNow;
	openAndSave = [s, openAndSave] { s->save(); openAndSave(); };
	row.makeAcceptInputHandler(openAndSave);

	auto scrape_now = std::make_shared<TextComponent>(mWindow, "RASPAR AHORA", Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
	auto bracket = makeArrow(mWindow);
	row.addElement(scrape_now, true);
	row.addElement(bracket, false);
	s->addRow(row);

	mWindow->pushGui(s);
}

void GuiMenu::openSoundSettings()
{
	auto s = new GuiSettings(mWindow, "AJUSTES DE SONIDO");

	// volume
	auto volume = std::make_shared<SliderComponent>(mWindow, 0.f, 100.f, 1.f, "%");
	volume->setValue((float)VolumeControl::getInstance()->getVolume());
	s->addWithLabel("VOLUMEN DEL SISTEMA", volume);
	s->addSaveFunc([volume] { VolumeControl::getInstance()->setVolume((int)Math::round(volume->getValue())); });

	if (UIModeController::getInstance()->isUIModeFull())
	{
#if defined(__linux__)
		// audio card
		auto audio_card = std::make_shared< OptionListComponent<std::string> >(mWindow, "TARJETA DE SONIDO", false);
		std::vector<std::string> audio_cards;
	#ifdef _RPI_
		// RPi Specific  Audio Cards
		audio_cards.push_back("local");
		audio_cards.push_back("hdmi");
		audio_cards.push_back("both");
	#endif
		audio_cards.push_back("default");
		audio_cards.push_back("sysdefault");
		audio_cards.push_back("dmix");
		audio_cards.push_back("hw");
		audio_cards.push_back("plughw");
		audio_cards.push_back("null");
		if (Settings::getInstance()->getString("AudioCard") != "") {
			if(std::find(audio_cards.begin(), audio_cards.end(), Settings::getInstance()->getString("AudioCard")) == audio_cards.end()) {
				audio_cards.push_back(Settings::getInstance()->getString("AudioCard"));
			}
		}
		for(auto ac = audio_cards.cbegin(); ac != audio_cards.cend(); ac++)
			audio_card->add(*ac, *ac, Settings::getInstance()->getString("AudioCard") == *ac);
		s->addWithLabel("AUDIO CARD", audio_card);
		s->addSaveFunc([audio_card] {
			Settings::getInstance()->setString("AudioCard", audio_card->getSelected());
			VolumeControl::getInstance()->deinit();
			VolumeControl::getInstance()->init();
		});

		// volume control device
		auto vol_dev = std::make_shared< OptionListComponent<std::string> >(mWindow, "DISPOSITIVO DE SONIDO", false);
		std::vector<std::string> transitions;
		transitions.push_back("PCM");
		transitions.push_back("Speaker");
		transitions.push_back("Master");
		transitions.push_back("Digital");
		transitions.push_back("Analogue");
		if (Settings::getInstance()->getString("AudioDevice") != "") {
			if(std::find(transitions.begin(), transitions.end(), Settings::getInstance()->getString("AudioDevice")) == transitions.end()) {
				transitions.push_back(Settings::getInstance()->getString("AudioDevice"));
			}
		}
		for(auto it = transitions.cbegin(); it != transitions.cend(); it++)
			vol_dev->add(*it, *it, Settings::getInstance()->getString("AudioDevice") == *it);
		s->addWithLabel("AUDIO DEVICE", vol_dev);
		s->addSaveFunc([vol_dev] {
			Settings::getInstance()->setString("AudioDevice", vol_dev->getSelected());
			VolumeControl::getInstance()->deinit();
			VolumeControl::getInstance()->init();
		});
#endif

		// disable sounds
		auto sounds_enabled = std::make_shared<SwitchComponent>(mWindow);
		sounds_enabled->setState(Settings::getInstance()->getBool("EnableSounds"));
		s->addWithLabel("ACTIVAR SONIDOS EN LA NAVEGACIÓN", sounds_enabled);
		s->addSaveFunc([sounds_enabled] {
			if (sounds_enabled->getState()
				&& !Settings::getInstance()->getBool("EnableSounds")
				&& PowerSaver::getMode() == PowerSaver::INSTANT)
			{
				Settings::getInstance()->setString("PowerSaverMode", "default");
				PowerSaver::init();
			}
			Settings::getInstance()->setBool("EnableSounds", sounds_enabled->getState());
		});

		auto video_audio = std::make_shared<SwitchComponent>(mWindow);
		video_audio->setState(Settings::getInstance()->getBool("VideoAudio"));
		s->addWithLabel("ACTIVAR SONIDO EN EL VIDEO", video_audio);
		s->addSaveFunc([video_audio] { Settings::getInstance()->setBool("VideoAudio", video_audio->getState()); });

#ifdef _RPI_
		// OMX player Audio Device
		auto omx_audio_dev = std::make_shared< OptionListComponent<std::string> >(mWindow, "DISPOSITIVO DE SONIDO OMX PLAYER", false);
		std::vector<std::string> omx_cards;
		// RPi Specific  Audio Cards
		omx_cards.push_back("local");
		omx_cards.push_back("hdmi");
		omx_cards.push_back("both");
		omx_cards.push_back("alsa:hw:0,0");
		omx_cards.push_back("alsa:hw:1,0");
		if (Settings::getInstance()->getString("OMXAudioDev") != "") {
			if (std::find(omx_cards.begin(), omx_cards.end(), Settings::getInstance()->getString("OMXAudioDev")) == omx_cards.end()) {
				omx_cards.push_back(Settings::getInstance()->getString("OMXAudioDev"));
			}
		}
		for (auto it = omx_cards.cbegin(); it != omx_cards.cend(); it++)
			omx_audio_dev->add(*it, *it, Settings::getInstance()->getString("OMXAudioDev") == *it);
		s->addWithLabel("OMX PLAYER AUDIO DEVICE", omx_audio_dev);
		s->addSaveFunc([omx_audio_dev] {
			if (Settings::getInstance()->getString("OMXAudioDev") != omx_audio_dev->getSelected())
				Settings::getInstance()->setString("OMXAudioDev", omx_audio_dev->getSelected());
		});
#endif
	}

	mWindow->pushGui(s);

}

void GuiMenu::openUISettings()
{
	auto s = new GuiSettings(mWindow, "AJUSTES DE INTERFAZ");

	//UI mode
	auto UImodeSelection = std::make_shared< OptionListComponent<std::string> >(mWindow, "MODO INTERFAZ", false);
	std::vector<std::string> UImodes = UIModeController::getInstance()->getUIModes();
	for (auto it = UImodes.cbegin(); it != UImodes.cend(); it++)
		UImodeSelection->add(*it, *it, Settings::getInstance()->getString("UIMode") == *it);
	s->addWithLabel("MODO INTERFAZ", UImodeSelection);
	Window* window = mWindow;
	s->addSaveFunc([ UImodeSelection, window]
	{
		std::string selectedMode = UImodeSelection->getSelected();
		if (selectedMode != "Full")
		{
			std::string msg = "Está cambiando la IU a un modo restringido:\n" + selectedMode + "\n";
			msg += "Esto ocultará la mayoría de las opciones de menú para evitar cambios en el sistema.\n";
			msg += "Para desbloquear y volver a la IU completa, ingrese este código: \n";
			msg += "\"" + UIModeController::getInstance()->getFormattedPassKeyStr() + "\"\n\n";
			msg += "¿Quieres proceder?";
			window->pushGui(new GuiMsgBox(window, msg,
				"SI", [selectedMode] {
					LOG(LogDebug) << "Setting UI mode to " << selectedMode;
					Settings::getInstance()->setString("UIMode", selectedMode);
					Settings::getInstance()->saveFile();
			}, "NO",nullptr));
		}
	});

	// screensaver
	ComponentListRow screensaver_row;
	screensaver_row.elements.clear();
	screensaver_row.addElement(std::make_shared<TextComponent>(mWindow, "AJUSTES DE PANTALLA", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	screensaver_row.addElement(makeArrow(mWindow), false);
	screensaver_row.makeAcceptInputHandler(std::bind(&GuiMenu::openScreensaverOptions, this));
	s->addRow(screensaver_row);

	// quick system select (left/right in game list view)
	auto quick_sys_select = std::make_shared<SwitchComponent>(mWindow);
	quick_sys_select->setState(Settings::getInstance()->getBool("QuickSystemSelect"));
	s->addWithLabel("SELECCIÓN RÁPIDA DEL SISTEMA", quick_sys_select);
	s->addSaveFunc([quick_sys_select] { Settings::getInstance()->setBool("QuickSystemSelect", quick_sys_select->getState()); });

	// carousel transition option
	auto move_carousel = std::make_shared<SwitchComponent>(mWindow);
	move_carousel->setState(Settings::getInstance()->getBool("MoveCarousel"));
	s->addWithLabel("TRANSICIONES DE CARRUSEL", move_carousel);
	s->addSaveFunc([move_carousel] {
		if (move_carousel->getState()
			&& !Settings::getInstance()->getBool("MoveCarousel")
			&& PowerSaver::getMode() == PowerSaver::INSTANT)
		{
			Settings::getInstance()->setString("PowerSaverMode", "default");
			PowerSaver::init();
		}
		Settings::getInstance()->setBool("MoveCarousel", move_carousel->getState());
	});

	// transition style
	auto transition_style = std::make_shared< OptionListComponent<std::string> >(mWindow, "ESTILO DE TRANSICIÓN", false);
	std::vector<std::string> transitions;
	transitions.push_back("fade");
	transitions.push_back("slide");
	transitions.push_back("instant");
	for(auto it = transitions.cbegin(); it != transitions.cend(); it++)
		transition_style->add(*it, *it, Settings::getInstance()->getString("TransitionStyle") == *it);
	s->addWithLabel("TRANSITION STYLE", transition_style);
	s->addSaveFunc([transition_style] {
		if (Settings::getInstance()->getString("TransitionStyle") == "instant"
			&& transition_style->getSelected() != "instant"
			&& PowerSaver::getMode() == PowerSaver::INSTANT)
		{
			Settings::getInstance()->setString("PowerSaverMode", "default");
			PowerSaver::init();
		}
		Settings::getInstance()->setString("TransitionStyle", transition_style->getSelected());
	});

	// theme set
	auto themeSets = ThemeData::getThemeSets();

	if(!themeSets.empty())
	{
		std::map<std::string, ThemeSet>::const_iterator selectedSet = themeSets.find(Settings::getInstance()->getString("ThemeSet"));
		if(selectedSet == themeSets.cend())
			selectedSet = themeSets.cbegin();

		auto theme_set = std::make_shared< OptionListComponent<std::string> >(mWindow, "ESTABLECER TEMA", false);
		for(auto it = themeSets.cbegin(); it != themeSets.cend(); it++)
			theme_set->add(it->first, it->first, it == selectedSet);
		s->addWithLabel("ESTABLECER TEMA", theme_set);

		Window* window = mWindow;
		s->addSaveFunc([window, theme_set]
		{
			bool needReload = false;
			std::string oldTheme = Settings::getInstance()->getString("ThemeSet");
			if(oldTheme != theme_set->getSelected())
				needReload = true;

			Settings::getInstance()->setString("ThemeSet", theme_set->getSelected());

			if(needReload)
			{
				Scripting::fireEvent("theme-changed", theme_set->getSelected(), oldTheme);
				CollectionSystemManager::get()->updateSystemsList();
				ViewController::get()->goToStart();
				ViewController::get()->reloadAll(); // TODO - replace this with some sort of signal-based implementation
			}
		});
	}

	// GameList view style
	auto gamelist_style = std::make_shared< OptionListComponent<std::string> >(mWindow, "ESTILO DE LA LISTA DE JUEGOS", false);
	std::vector<std::string> styles;
	styles.push_back("automatic");
	styles.push_back("basic");
	styles.push_back("detailed");
	styles.push_back("video");
	styles.push_back("grid");

	for (auto it = styles.cbegin(); it != styles.cend(); it++)
		gamelist_style->add(*it, *it, Settings::getInstance()->getString("GamelistViewStyle") == *it);
	s->addWithLabel("ESTILO DE LA LISTA DE JUEGOS", gamelist_style);
	s->addSaveFunc([gamelist_style] {
		bool needReload = false;
		if (Settings::getInstance()->getString("GamelistViewStyle") != gamelist_style->getSelected())
			needReload = true;
		Settings::getInstance()->setString("GamelistViewStyle", gamelist_style->getSelected());
		if (needReload)
			ViewController::get()->reloadAll();
	});

	// Optionally start in selected system
	auto systemfocus_list = std::make_shared< OptionListComponent<std::string> >(mWindow, "INICIAR EN EL SISTEMA", false);
	systemfocus_list->add("NINGUNO", "", Settings::getInstance()->getString("StartupSystem") == "");
	for (auto it = SystemData::sSystemVector.cbegin(); it != SystemData::sSystemVector.cend(); it++)
	{
		if ("retropie" != (*it)->getName())
		{
			systemfocus_list->add((*it)->getName(), (*it)->getName(), Settings::getInstance()->getString("StartupSystem") == (*it)->getName());
		}
	}
	s->addWithLabel("INICIAR EN EL SISTEMA", systemfocus_list);
	s->addSaveFunc([systemfocus_list] {
		Settings::getInstance()->setString("StartupSystem", systemfocus_list->getSelected());
	});

	// show help
	auto show_help = std::make_shared<SwitchComponent>(mWindow);
	show_help->setState(Settings::getInstance()->getBool("ShowHelpPrompts"));
	s->addWithLabel("AYUDA EN PANTALLA", show_help);
	s->addSaveFunc([show_help] { Settings::getInstance()->setBool("ShowHelpPrompts", show_help->getState()); });

	// enable filters (ForceDisableFilters)
	auto enable_filter = std::make_shared<SwitchComponent>(mWindow);
	enable_filter->setState(!Settings::getInstance()->getBool("ForceDisableFilters"));
	s->addWithLabel("ENABLE FILTERS", enable_filter);
	s->addSaveFunc([enable_filter] {
		bool filter_is_enabled = !Settings::getInstance()->getBool("ForceDisableFilters");
		Settings::getInstance()->setBool("ForceDisableFilters", !enable_filter->getState());
		if (enable_filter->getState() != filter_is_enabled) ViewController::get()->ReloadAndGoToStart();
	});

	mWindow->pushGui(s);

}

void GuiMenu::openOtherSettings()
{
	auto s = new GuiSettings(mWindow, "OTROS AJUSTES");

	// maximum vram
	auto max_vram = std::make_shared<SliderComponent>(mWindow, 0.f, 1000.f, 10.f, "Mb");
	max_vram->setValue((float)(Settings::getInstance()->getInt("MaxVRAM")));
	s->addWithLabel("LÍMITE VRAM", max_vram);
	s->addSaveFunc([max_vram] { Settings::getInstance()->setInt("MaxVRAM", (int)Math::round(max_vram->getValue())); });

	// power saver
	auto power_saver = std::make_shared< OptionListComponent<std::string> >(mWindow, "MODOS DE AHORRO DE ENERGÍA", false);
	std::vector<std::string> modes;
	modes.push_back("disabled");
	modes.push_back("default");
	modes.push_back("enhanced");
	modes.push_back("instant");
	for (auto it = modes.cbegin(); it != modes.cend(); it++)
		power_saver->add(*it, *it, Settings::getInstance()->getString("PowerSaverMode") == *it);
	s->addWithLabel("POWER SAVER MODES", power_saver);
	s->addSaveFunc([this, power_saver] {
		if (Settings::getInstance()->getString("PowerSaverMode") != "instant" && power_saver->getSelected() == "instant") {
			Settings::getInstance()->setString("TransitionStyle", "instant");
			Settings::getInstance()->setBool("MoveCarousel", false);
			Settings::getInstance()->setBool("EnableSounds", false);
		}
		Settings::getInstance()->setString("PowerSaverMode", power_saver->getSelected());
		PowerSaver::init();
	});

	// gamelists
	auto save_gamelists = std::make_shared<SwitchComponent>(mWindow);
	save_gamelists->setState(Settings::getInstance()->getBool("SaveGamelistsOnExit"));
	s->addWithLabel("SALVAR METADATOS AL SALIR", save_gamelists);
	s->addSaveFunc([save_gamelists] { Settings::getInstance()->setBool("SaveGamelistsOnExit", save_gamelists->getState()); });

	auto parse_gamelists = std::make_shared<SwitchComponent>(mWindow);
	parse_gamelists->setState(Settings::getInstance()->getBool("ParseGamelistOnly"));
	s->addWithLabel("ANALIZAR LISTAS DE JUEGO SOLAMENTE", parse_gamelists);
	s->addSaveFunc([parse_gamelists] { Settings::getInstance()->setBool("ParseGamelistOnly", parse_gamelists->getState()); });

	auto local_art = std::make_shared<SwitchComponent>(mWindow);
	local_art->setState(Settings::getInstance()->getBool("LocalArt"));
	s->addWithLabel("BUSCAR ARTE LOCAL", local_art);
	s->addSaveFunc([local_art] { Settings::getInstance()->setBool("LocalArt", local_art->getState()); });

	// hidden files
	auto hidden_files = std::make_shared<SwitchComponent>(mWindow);
	hidden_files->setState(Settings::getInstance()->getBool("ShowHiddenFiles"));
	s->addWithLabel("MOSTRAR ARCHIVOS OCULTOS", hidden_files);
	s->addSaveFunc([hidden_files] { Settings::getInstance()->setBool("ShowHiddenFiles", hidden_files->getState()); });

#ifdef _RPI_
	// Video Player - VideoOmxPlayer
	auto omx_player = std::make_shared<SwitchComponent>(mWindow);
	omx_player->setState(Settings::getInstance()->getBool("VideoOmxPlayer"));
	s->addWithLabel("USAR REPRODUCTOR OMX (HW ACELERADO)", omx_player);
	s->addSaveFunc([omx_player]
	{
		// need to reload all views to re-create the right video components
		bool needReload = false;
		if(Settings::getInstance()->getBool("VideoOmxPlayer") != omx_player->getState())
			needReload = true;

		Settings::getInstance()->setBool("VideoOmxPlayer", omx_player->getState());

		if(needReload)
			ViewController::get()->reloadAll();
	});

#endif

	// framerate
	auto framerate = std::make_shared<SwitchComponent>(mWindow);
	framerate->setState(Settings::getInstance()->getBool("DrawFramerate"));
	s->addWithLabel("MOSTRAR FPS", framerate);
	s->addSaveFunc([framerate] { Settings::getInstance()->setBool("DrawFramerate", framerate->getState()); });


	mWindow->pushGui(s);

}

void GuiMenu::openConfigInput()
{
	Window* window = mWindow;
	window->pushGui(new GuiMsgBox(window, "¿ESTÁ SEGURO QUE DESEA CONFIGURAR LA ENTRADA?", "SI",
		[window] {
		window->pushGui(new GuiDetectDevice(window, false, nullptr));
	}, "NO", nullptr)
	);

}
/* < emuelec */
void GuiMenu::openQuitMenu()
{
	auto s = new GuiSettings(mWindow, "QUITAR");

	Window* window = mWindow;

	ComponentListRow row;
	row.elements.clear();
	row.makeAcceptInputHandler([window] {
		window->pushGui(new GuiMsgBox(window, "¿VOLVER A KODI?", "SI",
			[] {
			Scripting::fireEvent("quit");
			quitES();
		}, "NO", nullptr));
	});
	row.addElement(std::make_shared<TextComponent>(window, "VOLVER A KODI", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	s->addRow(row);

	mWindow->pushGui(s);
}
/*  emuelec > */

void GuiMenu::addVersionInfo()
{
	std::string  buildDate = (Settings::getInstance()->getBool("Debug") ? std::string( "   (" + Utils::String::toUpper(PROGRAM_BUILT_STRING) + ")") : (""));

	mVersion.setFont(Font::get(FONT_SIZE_SMALL));
	mVersion.setColor(0x5E5E5EFF);
	mVersion.setText("EMULATIONSTATION V" + Utils::String::toUpper(PROGRAM_VERSION_STRING) + buildDate +" emuELEC-Addon v" + getShOutput(R"(cat /storage/.config/EE_VERSION)") + " IP:" + getShOutput(R"(/storage/.emulationstation/scripts/ip.sh)")); /* < emuelec */
	mVersion.setHorizontalAlignment(ALIGN_CENTER);
	addChild(&mVersion);
}

void GuiMenu::openScreensaverOptions() {
	mWindow->pushGui(new GuiGeneralScreensaverOptions(mWindow, "AJUSTES DE PANTALLA"));
}

void GuiMenu::openCollectionSystemSettings() {
	mWindow->pushGui(new GuiCollectionSystemsOptions(mWindow));
}

void GuiMenu::onSizeChanged()
{
	mVersion.setSize(mSize.x(), 0);
	mVersion.setPosition(0, mSize.y() - mVersion.getSize().y());
}

void GuiMenu::addEntry(const char* name, unsigned int color, bool add_arrow, const std::function<void()>& func)
{
	std::shared_ptr<Font> font = Font::get(FONT_SIZE_MEDIUM);

	// populate the list
	ComponentListRow row;
	row.addElement(std::make_shared<TextComponent>(mWindow, name, font, color), true);

	if(add_arrow)
	{
		std::shared_ptr<ImageComponent> bracket = makeArrow(mWindow);
		row.addElement(bracket, false);
	}

	row.makeAcceptInputHandler(func);

	mMenu.addRow(row);
}

bool GuiMenu::input(InputConfig* config, Input input)
{
	if(GuiComponent::input(config, input))
		return true;

	if((config->isMappedTo("b", input) || config->isMappedTo("start", input)) && input.value != 0)
	{
		delete this;
		return true;
	}

	return false;
}

HelpStyle GuiMenu::getHelpStyle()
{
	HelpStyle style = HelpStyle();
	style.applyTheme(ViewController::get()->getState().getSystem()->getTheme(), "system");
	return style;
}

std::vector<HelpPrompt> GuiMenu::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt("up/down", "choose"));
	prompts.push_back(HelpPrompt("a", "select"));
	prompts.push_back(HelpPrompt("start", "close"));
	return prompts;
}
