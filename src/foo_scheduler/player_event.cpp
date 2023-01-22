#include "pch.h"
#include "player_event.h"
#include "pref_page_model.h"
#include "service_manager.h"
#include "combo_helpers.h"

//------------------------------------------------------------------------------
// PlayerEvent
//------------------------------------------------------------------------------

PlayerEvent::PlayerEvent() : m_type(PlayerEventType::onPlaybackStart), m_finalAction(finalActionReenable)
{
}

PlayerEvent::PlayerEvent(const PlayerEvent& rhs) : Event(rhs),
	m_type(rhs.m_type),	m_finalAction(rhs.m_finalAction), m_stopReasons(rhs.m_stopReasons)
{
}

std::wstring PlayerEvent::GetName() const
{
	return L"player event";
}

std::wstring PlayerEvent::GetDescription() const
{
	std::wstring result = PlayerEventType::Label(m_type);
	
	if (m_type == PlayerEventType::onPlaybackStop)
	{
		result += L" / ";

		for (std::size_t i = 0; i < m_stopReasons.size(); ++i)
		{
			result += boost::to_lower_copy(PlayerEventStopReason::Label(m_stopReasons[i]));

			if (i != m_stopReasons.size() - 1)
				result += L", ";
		}
	}

	switch (m_finalAction)
	{
	case finalActionRemove:
		result += L" [remove event]";
		break;

	case finalActionDisable:
		result += L" [disable event]";
		break;

	case finalActionReenable:
		result += L" [re-enable event]";
		break;
	}
	
	return result;
}

bool PlayerEvent::ShowConfigDialog(CWindow parent, PrefPageModel* pPrefPageModel)
{
	PlayerEventEditor dlg(this, pPrefPageModel);
	return dlg.DoModal(parent) == IDOK;
}

void PlayerEvent::OnSignal()
{
	switch (m_finalAction)
	{
	case finalActionRemove:
		ServiceManager::Instance().GetModel().RemoveEvent(this);
		return;

	case finalActionDisable:
		Enable(false);
		ServiceManager::Instance().GetModel().UpdateEvent(this);
		break;

	case finalActionReenable:
		break;
	}
}

std::unique_ptr<Event> PlayerEvent::Clone() const
{
    return std::unique_ptr<Event>(new PlayerEvent(*this));
}

PlayerEventType::Type PlayerEvent::GetType() const
{
	return m_type;
}

void PlayerEvent::SetType(PlayerEventType::Type type)
{
	m_type = type;
}

PlayerEvent::EFinalAction PlayerEvent::GetFinalAction() const
{
	return m_finalAction;
}

void PlayerEvent::SetFinalAction(EFinalAction finalAction)
{
	m_finalAction = finalAction;
}

void PlayerEvent::LoadFromS11nBlock(const EventS11nBlock& block)
{
	if (!block.playerEvent.Exists())
		return;

	const PlayerEventS11nBlock& b = block.playerEvent.GetValue();

	if (b.type.Exists())
		m_type = static_cast<PlayerEventType::Type>(b.type.GetValue());

	if (b.finalAction.Exists())
		m_finalAction = static_cast<EFinalAction>(b.finalAction.GetValue());

	if (m_type == PlayerEventType::onPlaybackStop)
	{
		if (b.stopReasons.Exists())
		{
			for (int i = 0; i < b.stopReasons.GetSize(); ++i)
			{
				int stopReason = b.stopReasons.GetAt(i);

				// This may happen due to removal of unnecessary reason.
				if (stopReason >= PlayerEventStopReason::numReasons)
					stopReason = 0;

				m_stopReasons.push_back(static_cast<PlayerEventStopReason::Type>(stopReason));
			}
		}
	
		if (m_stopReasons.empty())
		{
			// To keep compatibility during version upgrade from 4.01 to 4.02.
			m_stopReasons.push_back(PlayerEventStopReason::eof);
		}
	}
}

void PlayerEvent::SaveToS11nBlock(EventS11nBlock& block) const
{
	PlayerEventS11nBlock b;

	b.type.SetValue(m_type);
	b.finalAction.SetValue(m_finalAction);

	if (m_type == PlayerEventType::onPlaybackStop)
	{
		for (std::size_t i = 0; i < m_stopReasons.size(); ++i)
			b.stopReasons.Add(m_stopReasons[i]);
	}

	block.playerEvent.SetValue(b);
}

void PlayerEvent::ApplyVisitor(IEventVisitor& visitor)
{
    visitor.Visit(*this);
}

GUID PlayerEvent::GetPrototypeGUID() const
{
	// {3a25dc7e-e051-42c1-921a-7e1d4c7ee416} 
	static const GUID result = 
	{ 0x3a25dc7e, 0xe051, 0x42c1, { 0x92, 0x1a, 0x7e, 0x1d, 0x4c, 0x7e, 0xe4, 0x16 } };

	return result;
}

int PlayerEvent::GetPriority() const
{
	return 10;
}

const PlayerEvent::StopReasons& PlayerEvent::GetStopReasons() const
{
	_ASSERTE(m_type == PlayerEventType::onPlaybackStop);
	return m_stopReasons;
}

void PlayerEvent::SetStopReasons(const StopReasons& stopReasons)
{
	m_stopReasons = stopReasons;
}

std::unique_ptr<Event> PlayerEvent::CreateFromPrototype() const
{
	std::unique_ptr<Event>pClone(new PlayerEvent(*this));
	pClone->NewEventGUID();
	return pClone;
}

namespace
{
	const bool registered = ServiceManager::Instance().GetEventPrototypesManager().RegisterPrototype(
		new PlayerEvent);
}

//------------------------------------------------------------------------------
// PlayerEventEditor
//------------------------------------------------------------------------------

PlayerEventEditor::PlayerEventEditor(PlayerEvent* pEvent, PrefPageModel* pPrefPageModel) :
	m_pEvent(pEvent), m_pPrefPageModel(pPrefPageModel),
	m_stopReasonsDy(0), m_stopReasonsControlVisible(true)
{

}

BOOL PlayerEventEditor::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	m_eventType = GetDlgItem(IDC_COMBO_EVENT);
	m_finalAction = GetDlgItem(IDC_COMBO_FINAL_ACTION);
	
	std::vector<std::pair<std::wstring, int>> comboItems;

	// dark mode
	AddDialog(m_hWnd);
	AddControls(m_hWnd);

	for (int i = PlayerEventType::onPlaybackStart; i < PlayerEventType::numEvents; ++i)
		comboItems.push_back(std::make_pair(PlayerEventType::Label(static_cast<PlayerEventType::Type>(i)), i));

	ComboHelpers::InitCombo(m_eventType, comboItems, m_pEvent->GetType());

	ComboHelpers::InitCombo(m_finalAction,
		boost::assign::list_of<std::pair<std::wstring, int> >
		(L"Re-enable event", PlayerEvent::finalActionReenable)
		(L"Disable event", PlayerEvent::finalActionDisable)
		(L"Remove event", PlayerEvent::finalActionRemove),
		m_pEvent->GetFinalAction()
	);

	CreateStopReasonsControl();
	UpdateStopReasonsControlVisibility();

	CenterWindow(GetParent());

	return TRUE;
}

void PlayerEventEditor::OnClose(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (nID == IDOK)
	{
		PlayerEventType::Type type = ComboHelpers::GetSelectedItem<PlayerEventType::Type>(m_eventType);

		if (type == PlayerEventType::onPlaybackStop)
		{
			PlayerEvent::StopReasons stopReasons;

			for (int i = PlayerEventStopReason::user; i < PlayerEventStopReason::numReasons; ++i)
			{
				if (m_stopReasons.GetCheckState(i))
					stopReasons.push_back(static_cast<PlayerEventStopReason::Type>(i));
			}

			if (stopReasons.empty())
			{
				m_popupTooltipMsg.Show(L"Specify at least one reason.", m_stopReasons);
				return;
			}

			m_pEvent->SetStopReasons(stopReasons);
		}
		else
			m_pEvent->SetStopReasons(PlayerEvent::StopReasons());

		m_pEvent->SetType(type);
		m_pEvent->SetFinalAction(ComboHelpers::GetSelectedItem<PlayerEvent::EFinalAction>(m_finalAction));
	}

	EndDialog(nID);
}

void PlayerEventEditor::CreateStopReasonsControl()
{
	m_stopReasons.SubclassWindow(GetDlgItem(IDC_LIST_STOP_REASON));

	DWORD dwStyle = LVS_EX_LABELTIP | LVS_EX_DOUBLEBUFFER;
	m_stopReasons.SetExtendedListViewStyle(dwStyle, dwStyle);

	::SetWindowTheme(m_stopReasons.m_hWnd, L"explorer", 0);

	CRect rect;
	m_stopReasons.GetClientRect(rect);

	m_stopReasons.AddColumn(L"", 0);
	m_stopReasons.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);

	for (int i = PlayerEventStopReason::user; i < PlayerEventStopReason::numReasons; ++i)
	{
		m_stopReasons.AddItem(i, 0,
			PlayerEventStopReason::Label(static_cast<PlayerEventStopReason::Type>(i)).c_str());
	}

	CRect stopReasonsRect;
	m_stopReasons.GetWindowRect(stopReasonsRect);
	ScreenToClient(stopReasonsRect);

	CRect finalActionRect;
	m_finalAction.GetWindowRect(finalActionRect);
	ScreenToClient(finalActionRect);

	m_stopReasonsDy = finalActionRect.top - stopReasonsRect.top;

	if (m_pEvent->GetType() == PlayerEventType::onPlaybackStop)
	{
		PlayerEvent::StopReasons stopReasons = m_pEvent->GetStopReasons();

		for (int i = PlayerEventStopReason::user; i < PlayerEventStopReason::numReasons; ++i)
		{
			auto it = std::find(stopReasons.cbegin(), stopReasons.cend(), static_cast<PlayerEventStopReason::Type>(i));

			m_stopReasons.SetCheckState(i, it != stopReasons.cend());
		}
	}
}

void PlayerEventEditor::UpdateStopReasonsControlVisibility()
{
	PlayerEventType::Type type = ComboHelpers::GetSelectedItem<PlayerEventType::Type>(m_eventType);

	bool show = type == PlayerEventType::onPlaybackStop;

	if (m_stopReasonsControlVisible == show)
		return;

	const int dy = m_stopReasonsControlVisible ? -m_stopReasonsDy : m_stopReasonsDy;
	m_stopReasonsControlVisible = show;

	GetDlgItem(IDC_STATIC_REASON).ShowWindow(show);
	m_stopReasons.ShowWindow(show);

	std::vector<CWindow> windowsToMove;

	windowsToMove.push_back(GetDlgItem(IDC_STATIC_FINAL_ACTION));
	windowsToMove.push_back(GetDlgItem(IDC_COMBO_FINAL_ACTION));
	windowsToMove.push_back(GetDlgItem(IDOK));
	windowsToMove.push_back(GetDlgItem(IDCANCEL));

	for (std::size_t i = 0; i < windowsToMove.size(); ++i)
	{
		CWindow wnd = windowsToMove[i];

		CRect rect;
		wnd.GetWindowRect(rect);
		ScreenToClient(rect);

		rect.OffsetRect(0, dy);

		wnd.MoveWindow(rect);
	}

	CRect dlgRect;
	GetWindowRect(dlgRect);
	dlgRect.bottom += dy;

	SetWindowPos(NULL, 0, 0, dlgRect.Width(), dlgRect.Height(), SWP_NOZORDER | SWP_NOMOVE);
}

void PlayerEventEditor::OnEventTypeSelChange(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	m_popupTooltipMsg.CleanUp();
	UpdateStopReasonsControlVisibility();
}