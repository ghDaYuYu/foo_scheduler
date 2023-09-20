#pragma once

using namespace cfg_var_legacy;

class PluginInitQuit : public initquit
{
public:
	virtual void on_init();
	virtual void on_quit();
};

class PluginConfiguration : public cfg_var
{
public:
	explicit PluginConfiguration(const GUID& p_guid);

private:
	virtual void get_data_raw(stream_writer* p_stream, abort_callback& p_abort);
	virtual void set_data_raw(stream_reader* p_stream, t_size p_sizehint, abort_callback& p_abort);
};

namespace MainMenuItems
{
	class SchedulerGroup : public mainmenu_group_impl
	{
	public:
		static const GUID m_guid;
		SchedulerGroup();
	};

	class SchedulerMainPopupMenu : public mainmenu_group_popup_impl
	{
	public:
		static const GUID m_guid;
		SchedulerMainPopupMenu();
	};

	class SchedulerMainPopupCommands : public mainmenu_commands
	{
	public:
		virtual t_uint32 get_command_count();
		virtual GUID     get_command(t_uint32 p_index);
		virtual void     get_name(t_uint32 p_index, pfc::string_base& p_out);
		virtual bool     get_description(t_uint32 p_index, pfc::string_base& p_out);
		virtual GUID     get_parent();
		virtual t_uint32 get_sort_priority();
		virtual void     execute(t_uint32 p_index, service_ptr_t<service_base> p_callback);

	private:
		enum EMenuItemIndex
		{
			miiPreferences = 0,
			miiStatusWindow,
			miiStopAllActionLists,

			numMenuItems
		};
	};

	class SchedulerActionListsGroup : public mainmenu_group_impl
	{
	public:
		static const GUID m_guid;
		SchedulerActionListsGroup();
	};

	class SchedulerActionListsCommands : public mainmenu_commands
	{
	public:
		virtual t_uint32 get_command_count();
		virtual GUID     get_command(t_uint32 p_index);
		virtual void     get_name(t_uint32 p_index, pfc::string_base& p_out);
		virtual bool     get_description(t_uint32 p_index, pfc::string_base& p_out);
		virtual GUID     get_parent();
		virtual t_uint32 get_sort_priority();
		virtual void     execute(t_uint32 p_index, service_ptr_t<service_base> p_callback);
	};

} // namespace MainMenuItems
