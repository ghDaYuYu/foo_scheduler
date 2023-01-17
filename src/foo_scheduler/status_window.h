#pragma once

#include "resource.h"
#include "header_static.h"

class ActionListExecSession;
class DateTimeEvent;

//------------------------------------------------------------------------------
// StatusWindow
//------------------------------------------------------------------------------

class ListViewWithResizableColumn : public CWindowImpl<ListViewWithResizableColumn, CListViewCtrl> {
private:
	BEGIN_MSG_MAP(ListViewWithResizableColumn)
		MSG_WM_SIZE(OnSize)
	END_MSG_MAP()

	void OnSize(UINT nType, CSize size)
	{
		DefWindowProc();
		SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
	}
};

class StatusWindow :
	public CDialogImpl<StatusWindow>,
	public CWinDataExchange<StatusWindow>,
	public CDialogResize<StatusWindow>,
	public message_filter,
	public boost::signals2::trackable,
	public fb2k::CDarkModeHooks
{
public:
	enum { IDD = IDD_STATUS_WINDOW };

	StatusWindow(HWND parent, const boost::function<void ()>& onDestroyCallback);

	void Activate();

private:
	// Private destructor to forbid creation on stack.
	~StatusWindow();

	void UpdatePendingEvents();
	void UpdateActiveSessions();

	void OnActionListExecSessionAdded(ActionListExecSession* pSession);
	void OnActionListExecSessionUpdated(ActionListExecSession* pSession);
	void OnActionListExecSessionRemoved(ActionListExecSession* pSession);
	
private:
	virtual bool pretranslate_message(MSG* p_msg);

private:
	BEGIN_MSG_MAP(StatusWindow)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)

		COMMAND_ID_HANDLER_EX(IDOK, OnCloseButton)
		COMMAND_ID_HANDLER_EX(IDC_BTN_STOP_ALL, OnStopAllActionLists)
		NOTIFY_HANDLER_EX(IDC_ACT_SESSIONS_LIST, LVN_GETDISPINFO, OnActSessionsListGetDispInfo)
		NOTIFY_HANDLER_EX(IDC_STATUS_DATE_TIME_EVENTS_LIST, LVN_GETDISPINFO, OnPendingEventsGetDispInfo)
		NOTIFY_HANDLER_EX(IDC_ACT_SESSIONS_LIST, NM_DBLCLK, OnActSessionsDblClick)
		REFLECT_NOTIFICATIONS()
		CHAIN_MSG_MAP(CDialogResize<StatusWindow>)
	END_MSG_MAP()

	BEGIN_DDX_MAP(StatusWindow)
		DDX_CONTROL(IDC_STATUS_DATE_TIME_EVENTS_HEADER, m_staticStatusDateTimeEventsHeader)
		DDX_CONTROL(IDC_ACT_SESSIONS_HEADER, m_staticActiveSessionsHeader)
	END_DDX_MAP()

	BEGIN_DLGRESIZE_MAP(StatusWindow)
		DLGRESIZE_CONTROL(IDC_STATUS_DATE_TIME_EVENTS_HEADER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_STATUS_DATE_TIME_EVENTS_LIST, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ACT_SESSIONS_HEADER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ACT_SESSIONS_LIST, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_BTN_STOP_ALL, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X | DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);
	void OnClose();
	void OnCloseButton(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnStopAllActionLists(UINT uNotifyCode, int nID, CWindow wndCtl);

	void OnDestroy();

	LRESULT OnActSessionsListGetDispInfo(LPNMHDR pnmh);
	LRESULT OnActSessionsDblClick(LPNMHDR pnmh);

	LRESULT OnPendingEventsGetDispInfo(LPNMHDR pnmh);

private:
	virtual void OnFinalMessage(HWND hWnd);

private:
	boost::function<void ()> m_onDestroyCallback;

	HeaderStatic m_staticStatusDateTimeEventsHeader;
	HeaderStatic m_staticActiveSessionsHeader;

	ListViewWithResizableColumn m_dateTimeEvents;
	ListViewWithResizableColumn m_activeSessions;

	std::vector<ActionListExecSession*> m_activeSessionsModel;
	std::vector<DateTimeEvent*> m_pendingEventsModel;
};