#include "pch.h"
#include "status_window.h"
#include "service_manager.h"
#include "date_time_events_manager.h"
#include "action_list_exec_session.h"
#include "date_time_event.h"

// {967599e9-9808-49ea-b58b-d95ef843b157} 
static const GUID g_statusWindowSizeGUID = 
{ 0x967599e9, 0x9808, 0x49ea, { 0xb5, 0x8b, 0xd9, 0x5e, 0xf8, 0x43, 0xb1, 0x57 } };

static cfgWindowSize g_cfgStatusWindowSize(g_statusWindowSizeGUID);

// {367533c7-f887-4dc6-afa6-bbabf071d2a1} 
static const GUID g_statusWindowPosGUID = 
{ 0x367533c7, 0xf887, 0x4dc6, { 0xaf, 0xa6, 0xbb, 0xab, 0xf0, 0x71, 0xd2, 0xa1 } };

static cfgDialogPosition g_cfgStatusWindowPos(g_statusWindowPosGUID);

StatusWindow::StatusWindow(HWND parent, const boost::function<void ()>& onDestroyCallback) :
	m_onDestroyCallback(onDestroyCallback),
	m_sizeTracker(g_cfgStatusWindowSize), m_posTracker(g_cfgStatusWindowPos)
{
	ServiceManager::Instance().GetDateTimeEventsManager().ConnectPendingEventsUpdatedSlot(
		boost::bind(&StatusWindow::UpdatePendingEvents, this));

	ServiceManager::Instance().GetRootController().ConnectActionListExecSessionAddedSlot(
		boost::bind(&StatusWindow::OnActionListExecSessionAdded, this, _1));

	ServiceManager::Instance().GetRootController().ConnectActionListExecSessionUpdatedSlot(
		boost::bind(&StatusWindow::OnActionListExecSessionUpdated, this, _1));

	ServiceManager::Instance().GetRootController().ConnectActionListExecSessionRemovedSlot(
		boost::bind(&StatusWindow::OnActionListExecSessionRemoved, this, _1));

	static_api_ptr_t<message_loop> pMsgLoop;
	pMsgLoop->add_message_filter(this);

	Create(parent);
	ShowWindow(SW_SHOWNORMAL);
}

StatusWindow::~StatusWindow()
{
	static_api_ptr_t<message_loop> pMsgLoop;
	pMsgLoop->remove_message_filter(this);	
}

BOOL StatusWindow::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	m_activeSessionsModel =	ServiceManager::Instance().GetRootController().GetActionListExecSessions();

	DoDataExchange(DDX_LOAD);
	DlgResize_Init(true);
	cfg_window_placement_status_dlg.on_window_creation(m_hWnd, true);

	//dark mode
	AddDialog(m_hWnd);
	AddControls(m_hWnd);

	DWORD dwStyle = LVS_EX_LABELTIP | LVS_EX_DOUBLEBUFFER;

	m_dateTimeEvents.SubclassWindow(GetDlgItem(IDC_STATUS_DATE_TIME_EVENTS_LIST));
	m_dateTimeEvents.SetExtendedListViewStyle(dwStyle, dwStyle);
	m_dateTimeEvents.AddColumn(L"", 0);
	m_dateTimeEvents.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);

	m_activeSessions.SubclassWindow(GetDlgItem(IDC_ACT_SESSIONS_LIST));
	m_activeSessions.SetExtendedListViewStyle(dwStyle, dwStyle);
	m_activeSessions.AddColumn(L"", 0);
	m_activeSessions.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);

	::SetWindowTheme(m_dateTimeEvents.m_hWnd, L"explorer", 0);
	::SetWindowTheme(m_activeSessions.m_hWnd, L"explorer", 0);

	UpdatePendingEvents();
	UpdateActiveSessions();

	SetMsgHandled(false);
	return TRUE;
}

void StatusWindow::OnFinalMessage(HWND /*hWnd*/)
{
	m_onDestroyCallback();
	delete this;
}

void StatusWindow::OnClose()
{
	cfg_window_placement_status_dlg.on_window_destruction(m_hWnd);
	DestroyWindow();
}

void StatusWindow::Activate()
{
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE);
}

void StatusWindow::OnCloseButton(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	PostMessage(WM_CLOSE);
}

bool StatusWindow::pretranslate_message(MSG* p_msg)
{
	// Close window on ESC pressed.
	if ((p_msg->hwnd == m_hWnd || IsChild(p_msg->hwnd)) &&
		p_msg->message == WM_KEYDOWN && p_msg->wParam == VK_ESCAPE)
	{
		PostMessage(WM_CLOSE);
		return true;
	}

	return IsDialogMessage(p_msg) == TRUE;
}

void StatusWindow::UpdatePendingEvents()
{
	m_pendingEventsModel = ServiceManager::Instance().GetDateTimeEventsManager().GetPendingEvents();
	m_dateTimeEvents.SetItemCountEx(m_pendingEventsModel.size(), LVSICF_NOSCROLL);
}

void StatusWindow::OnActionListExecSessionAdded(ActionListExecSession* pSession)
{
	m_activeSessionsModel.push_back(pSession);
	UpdateActiveSessions();
}

void StatusWindow::OnActionListExecSessionUpdated(ActionListExecSession* pSession)
{
	UpdateActiveSessions();
}

void StatusWindow::OnActionListExecSessionRemoved(ActionListExecSession* pSession)
{
	m_activeSessionsModel.erase(std::find(m_activeSessionsModel.begin(), m_activeSessionsModel.end(), pSession));
	UpdateActiveSessions();
}

LRESULT StatusWindow::OnActSessionsListGetDispInfo(LPNMHDR pnmh)
{
	NMLVDISPINFO* pInfo = reinterpret_cast<NMLVDISPINFO*>(pnmh);

	if (pInfo->item.mask & LVIF_TEXT)
	{
		swprintf_s(pInfo->item.pszText, pInfo->item.cchTextMax, L"%s", m_activeSessionsModel[pInfo->item.iItem]->GetDescription().c_str());
	}

	return 0;
}

void StatusWindow::UpdateActiveSessions()
{
	m_activeSessions.SetItemCountEx(m_activeSessionsModel.size(), LVSICF_NOSCROLL);
}

LRESULT StatusWindow::OnActSessionsDblClick(LPNMHDR pnmh)
{
	NMITEMACTIVATE* pInfo = reinterpret_cast<NMITEMACTIVATE*>(pnmh);

	if (pInfo->iItem == -1)
		return 0;

	ActionListExecSession* pSession = m_activeSessionsModel[pInfo->iItem];
	ServiceManager::Instance().GetRootController().RemoveExecSession(pSession);

	return 0;
}

LRESULT StatusWindow::OnPendingEventsGetDispInfo(LPNMHDR pnmh)
{
	NMLVDISPINFO* pInfo = reinterpret_cast<NMLVDISPINFO*>(pnmh);

	if (pInfo->item.mask & LVIF_TEXT)
	{
		swprintf_s(pInfo->item.pszText, pInfo->item.cchTextMax, L"%s", m_pendingEventsModel[pInfo->item.iItem]->GetDescription().c_str());
	}

	return 0;
}

void StatusWindow::OnStopAllActionLists(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	ServiceManager::Instance().GetRootController().RemoveAllExecSessions();
}