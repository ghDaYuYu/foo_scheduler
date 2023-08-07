#include "pch.h"
#include "helpers/WindowPositionUtils.h"
#include "status_window.h"
#include "service_manager.h"
#include "date_time_events_manager.h"
#include "action_list_exec_session.h"
#include "date_time_event.h"

// {3F43374C-066A-4AC9-B5A5-EE71B23A7381}
static const GUID guid_cfg_dialog_position_status_dlg = { 0x3f43374c, 0x66a, 0x4ac9, { 0xb5, 0xa5, 0xee, 0x71, 0xb2, 0x3a, 0x73, 0x81 } };
static cfgDialogPosition cfg_dialog_position_status_dlg(guid_cfg_dialog_position_status_dlg);

StatusWindow::StatusWindow(HWND parent, const boost::function<void ()>& onDestroyCallback) :
	m_onDestroyCallback(onDestroyCallback)
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
	cfg_dialog_position_status_dlg.AddWindow(m_hWnd);
	DlgResize_Init(true);

	//dark mode
	m_dark.AddDialog(m_hWnd);
	m_dark.AddControls(m_hWnd);

	DWORD dwStyle = LVS_EX_LABELTIP | LVS_EX_DOUBLEBUFFER;

	m_dateTimeEvents.SubclassWindow(GetDlgItem(IDC_STATUS_DATE_TIME_EVENTS_LIST));
	m_dateTimeEvents.SetExtendedListViewStyle(dwStyle, dwStyle);
	m_dateTimeEvents.AddColumn(L"", 0);
	m_dateTimeEvents.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);

	m_activeSessions.SubclassWindow(GetDlgItem(IDC_ACT_SESSIONS_LIST));
	m_activeSessions.SetExtendedListViewStyle(dwStyle, dwStyle);
	m_activeSessions.AddColumn(L"", 0);
	m_activeSessions.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);

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
	cfg_dialog_position_status_dlg.RemoveWindow(m_hWnd);
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
	bool bfix_refresh = m_dateTimeEvents.GetItemCount() && m_dateTimeEvents.GetItemCount() == m_pendingEventsModel.size();
	m_dateTimeEvents.SetItemCountEx(static_cast<int>(m_pendingEventsModel.size()), LVSICF_NOSCROLL);
	if (bfix_refresh) {
		m_dateTimeEvents.RedrawItems((int)m_pendingEventsModel.size() - 1, (int)m_pendingEventsModel.size() - 1);
	}
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

	return TRUE;
}

void StatusWindow::UpdateActiveSessions()
{
	bool bfix_refresh = m_activeSessions.GetItemCount() && m_activeSessions.GetItemCount() == m_activeSessionsModel.size();
	m_activeSessions.SetItemCountEx(static_cast<int>(m_activeSessionsModel.size()), LVSICF_NOSCROLL);
	if (bfix_refresh) {
		m_activeSessions.RedrawItems((int)m_activeSessionsModel.size() - 1, (int)m_activeSessionsModel.size() - 1);
	}
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

	return TRUE;
}

void StatusWindow::OnStopAllActionLists(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	ServiceManager::Instance().GetRootController().RemoveAllExecSessions();
}