#include "pch.h"
#include "version.h"
#include "foobar_services.h"
#include "pref_page.h"

DECLARE_COMPONENT_VERSION(PLUGIN_NAME, PLUGIN_VERSION, PLUGIN_ABOUT)
VALIDATE_COMPONENT_FILENAME(PLUGIN_DLLFILENAME)

// {0F2265AC-38A4-4192-AE8A-508969E49280} mod guid
static const GUID g_guidPluginCfg =
	{ 0xf2265ac, 0x38a4, 0x4192, { 0xae, 0x8a, 0x50, 0x89, 0x69, 0xe4, 0x92, 0x80 } };

static PluginConfiguration g_cfgPlugin(g_guidPluginCfg);

static initquit_factory_t<PluginInitQuit> foo_initquit;

static mainmenu_commands_factory_t<MainMenuItems::SchedulerGroup> foo_menu_SchedulerGroup;
static mainmenu_commands_factory_t<MainMenuItems::SchedulerMainPopupMenu> foo_menu_SchedulerMainPopupMenu;
static mainmenu_commands_factory_t<MainMenuItems::SchedulerMainPopupCommands> foo_menu_SchedulerMainPopupCommands;
static mainmenu_commands_factory_t<MainMenuItems::SchedulerActionListsGroup> foo_menu_SchedulerActionListsGroup;
static mainmenu_commands_factory_t<MainMenuItems::SchedulerActionListsCommands> foo_menu_SchedulerActionListsCommands;

static preferences_page_factory_t<PreferencesPageImpl> foo_preferences_page_myimpl_factory;