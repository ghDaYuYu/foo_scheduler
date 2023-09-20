#include "pch.h"
#include "foobar_services.h"
#include "service_manager.h"
#include "version.h"
#include "foobar_stream.h"

//------------------------------------------------------------------------------
// PluginInitQuit
//------------------------------------------------------------------------------

void PluginInitQuit::on_init()
{
	ServiceManager::Instance().GetRootController().Init();
}

void PluginInitQuit::on_quit()
{
	ServiceManager::Instance().GetRootController().Shutdown();
}

//------------------------------------------------------------------------------
// PluginConfiguration
//------------------------------------------------------------------------------

PluginConfiguration::PluginConfiguration(const GUID& p_guid) : cfg_var(p_guid)
{
}

void PluginConfiguration::get_data_raw(stream_writer* p_stream, abort_callback& p_abort)
{
	foobar_stream_writer stream(*p_stream, p_abort);

	t_uint16 cfgVersion = PLUGIN_CFG_GLOBAL_VERSION;
	stream << cfgVersion;

	ServiceManager::Instance().GetModel().Save(stream);
}

void PluginConfiguration::set_data_raw(stream_reader* p_stream, t_size p_sizehint, abort_callback& p_abort)
{
	foobar_stream_reader stream(*p_stream, p_abort);

	// Reading config version
	t_uint16 cfgVersion = 0;
	bool readFailed = false;

	try
	{
		stream >> cfgVersion;
	}
	catch (exception_io&)
	{
		readFailed = true;
	}

	if (readFailed || cfgVersion < PLUGIN_CFG_GLOBAL_V4_19)
	{
		console::info(COMPONENT_NAME " : the configuration format is of version 3.x. Upgrade is not supported. "
			"Resetting configuration.");
		return;
	}

	ServiceManager::Instance().GetModel().Load(stream);
}

//------------------------------------------------------------------------------
// MainMenuItems
//------------------------------------------------------------------------------

namespace MainMenuItems
{
	//------------------------------------------------------------------------------
	// SchedulerGroup
	//------------------------------------------------------------------------------

	// {C7B4F372-AFB9-4FB8-BE20-959B949E0646} mod guid
	const GUID SchedulerGroup::m_guid =
		{ 0xc7b4f372, 0xafb9, 0x4fb8, { 0xbe, 0x20, 0x95, 0x9b, 0x94, 0x9e, 0x6, 0x46 } };

	SchedulerGroup::SchedulerGroup() : mainmenu_group_impl(m_guid,
		mainmenu_groups::file_playlist, mainmenu_commands::sort_priority_last)
	{
	}

	//------------------------------------------------------------------------------
	// SchedulerMainPopupMenu
	//------------------------------------------------------------------------------

	// {A8F73E72-8C56-4E70-B770-B952A573ECDF} mod guid
	const GUID SchedulerMainPopupMenu::m_guid =
		{ 0xa8f73e72, 0x8c56, 0x4e70, { 0xb7, 0x70, 0xb9, 0x52, 0xa5, 0x73, 0xec, 0xdf } };

	SchedulerMainPopupMenu::SchedulerMainPopupMenu() : mainmenu_group_popup_impl(m_guid,
		SchedulerGroup::m_guid,	mainmenu_commands::sort_priority_base, PLUGIN_NAME)
	{
	}

	//------------------------------------------------------------------------------
	// SchedulerMainPopupCommands
	//------------------------------------------------------------------------------
	
	t_uint32 SchedulerMainPopupCommands::get_command_count()
	{
		return numMenuItems;
	}

	GUID SchedulerMainPopupCommands::get_command(t_uint32 p_index)
	{
		// {FFC07C68-8861-4883-BDBC-792C7B40EC88} mod guid
		static const GUID guidPreferences =
			{ 0xffc07c68, 0x8861, 0x4883, { 0xbd, 0xbc, 0x79, 0x2c, 0x7b, 0x40, 0xec, 0x88 } };

		// {84E35924-CAC4-4AF6-96E3-61570661EC41} mod guid
		static const GUID guidStatusWindow =
			{ 0x84e35924, 0xcac4, 0x4af6, { 0x96, 0xe3, 0x61, 0x57, 0x6, 0x61, 0xec, 0x41 } };

		// {D67D975E-05B8-499A-9B8D-75B9A8875A30} mod guid
		static const GUID guidStopAllActionLists =
			{ 0xd67d975e, 0x5b8, 0x499a, { 0x9b, 0x8d, 0x75, 0xb9, 0xa8, 0x87, 0x5a, 0x30 } };


		if (p_index == miiPreferences)
			return guidPreferences;
		else if (p_index == miiStatusWindow)
			return guidStatusWindow;
		else if (p_index == miiStopAllActionLists)
			return guidStopAllActionLists;

		return pfc::guid_null;
	}

	void SchedulerMainPopupCommands::get_name(t_uint32 p_index, pfc::string_base& p_out)
	{
		if (p_index == miiPreferences)
			p_out = "Preferences";
		else if (p_index == miiStatusWindow)
			p_out = "Status window";
		else if (p_index == miiStopAllActionLists)
			p_out = "Stop all action lists";
	}

	bool SchedulerMainPopupCommands::get_description(t_uint32 p_index, pfc::string_base& p_out)
	{
		if (p_index == miiPreferences)
			p_out = "Opens scheduler preferences page.";
		else if (p_index == miiStatusWindow)
			p_out = "Opens status window.";
		else if (p_index == miiStopAllActionLists)
			p_out = "Stops all action lists.";
		else
			return false;

		return true;
	}

	GUID SchedulerMainPopupCommands::get_parent()
	{
		return SchedulerMainPopupMenu::m_guid;
	}

	t_uint32 SchedulerMainPopupCommands::get_sort_priority()
	{
		return mainmenu_commands::sort_priority_base + 1;
	}

	void SchedulerMainPopupCommands::execute(t_uint32 p_index, service_ptr_t<service_base> p_callback)
	{
		if (p_index == miiPreferences)
			ServiceManager::Instance().GetRootController().ShowPreferencesPage();
		else if (p_index == miiStatusWindow)
			ServiceManager::Instance().GetRootController().ShowStatusWindow();
		else if (p_index == miiStopAllActionLists)
			ServiceManager::Instance().GetRootController().RemoveAllExecSessions();
	}

	//------------------------------------------------------------------------------
	// SchedulerActionListsGroup
	//------------------------------------------------------------------------------

	// {A080066A-34C8-4AD8-963A-FB2181FDEC04} mod guid
	const GUID SchedulerActionListsGroup::m_guid =
		{ 0xa080066a, 0x34c8, 0x4ad8, { 0x96, 0x3a, 0xfb, 0x21, 0x81, 0xfd, 0xec, 0x4 } };

	SchedulerActionListsGroup::SchedulerActionListsGroup() : mainmenu_group_impl(m_guid,
		SchedulerMainPopupMenu::m_guid, mainmenu_commands::sort_priority_last)
	{
	}

	//------------------------------------------------------------------------------
	// SchedulerActionListsCommands
	//------------------------------------------------------------------------------

	t_uint32 SchedulerActionListsCommands::get_command_count()
	{
		return static_cast<t_uint32>(ServiceManager::Instance().GetMenuItemEventsManager().GetNumEvents());
	}

	GUID SchedulerActionListsCommands::get_command(t_uint32 p_index)
	{
		return ServiceManager::Instance().GetMenuItemEventsManager().GetEvent(p_index)->GetGUID();
	}

	void SchedulerActionListsCommands::get_name(t_uint32 p_index, pfc::string_base& p_out)
	{
		MenuItemEvent* pEvent = ServiceManager::Instance().GetMenuItemEventsManager().GetEvent(p_index);
		p_out = pfc::stringcvt::string_utf8_from_wide(pEvent->GetMenuItemName().c_str());
	}

	bool SchedulerActionListsCommands::get_description(t_uint32 p_index, pfc::string_base& p_out)
	{
		get_name(p_index, p_out);
		return true;
	}

	GUID SchedulerActionListsCommands::get_parent()
	{
		return SchedulerActionListsGroup::m_guid;
	}

	t_uint32 SchedulerActionListsCommands::get_sort_priority()
	{
		return mainmenu_commands::sort_priority_base + 1;
	}

	void SchedulerActionListsCommands::execute(t_uint32 p_index, service_ptr_t<service_base> p_callback)
	{
		ServiceManager::Instance().GetMenuItemEventsManager().EmitEvent(p_index);
	}
}