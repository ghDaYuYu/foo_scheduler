#pragma once

#include "resource.h"
#include "header_static.h"
#include "event_list_window.h"
#include "action_tree_window.h"
#include "pref_page_model.h"

class PreferencesPage :
	public CDialogImpl<PreferencesPage>,
	public CWinDataExchange<PreferencesPage>,
	public preferences_page_instance
{
public:
	// Constructor - invoked by preferences_page_impl helpers - don't do Create() in here,
	// preferences_page_impl does this for us.
	PreferencesPage(preferences_page_callback::ptr callback);
	~PreferencesPage();

	// dialog resource ID
	enum { IDD = IDD_PREF_PAGE };

	// preferences_page_instance methods (not all of them - get_wnd() is supplied by preferences_page_impl helpers)
	virtual t_uint32 get_state();
	virtual void apply();
	virtual void reset();

private:
	BEGIN_MSG_MAP(PreferencesPage)
		MSG_WM_INITDIALOG(OnInitDialog)

		COMMAND_ID_HANDLER_EX(IDC_BTN_ADD_EVENT, OnBtnAddEvent)
		COMMAND_ID_HANDLER_EX(IDC_BTN_ADD_ACTION_LIST, OnBtnAddActionList)
		COMMAND_ID_HANDLER_EX(IDC_BTN_SHOW_STATUS_WINDOW, OnBtnShowStatusWindow)
		COMMAND_ID_HANDLER_EX(IDC_ENABLED_CHECK, OnEnableScheduler)
		CHAIN_MSG_MAP_MEMBER(m_actionTree)
	END_MSG_MAP()

	BEGIN_DDX_MAP(PreferencesPage)
		DDX_CONTROL(IDC_EVENTS_LIST_HEADER, m_staticStatusDateTimeEventsHeader)
		DDX_CONTROL(IDC_STATIC_ACTION_LIST_HEADER, m_staticActiveSessionsHeader)
		DDX_CONTROL(IDC_STATIC_STATUS_HEADER, m_staticStatusHeader)
		DDX_CONTROL(IDC_ACTION_LIST_TREE, m_actionTree)
	END_DDX_MAP()

private:
	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);

	void OnBtnAddEvent(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnBtnAddActionList(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnBtnShowStatusWindow(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnEnableScheduler(UINT uNotifyCode, int nID, CWindow wndCtl);

	void OnChanged();

private:
	const preferences_page_callback::ptr m_callback;

	fb2k::CDarkModeHooks m_dark;

	HeaderStatic m_staticStatusDateTimeEventsHeader;
	HeaderStatic m_staticActiveSessionsHeader;
	HeaderStatic m_staticStatusHeader;

	EventListWindow m_eventList;
	ActionTreeWindow m_actionTree;

	boost::scoped_ptr<PrefPageModel> m_pModel;

	bool m_changed;
};

class PreferencesPageImpl : public preferences_page_impl<PreferencesPage>
{
public:
	static const GUID m_guid;

	virtual const char* get_name();
	virtual GUID get_guid();
	virtual GUID get_parent_guid();
};
