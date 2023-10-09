#include "pch.h"
#include "action_delay.h"
#include "service_manager.h"
#include "combo_helpers.h"

ActionDelay::ActionDelay() : m_delay(1), m_delayUnits(DurationUnits::seconds)
{

}

int ActionDelay::GetDelay() const
{
	return m_delay;
}

void ActionDelay::SetDelay(int delay)
{
	m_delay = delay;
}

GUID ActionDelay::GetPrototypeGUID() const
{
	// {cd95ae2a-1ca9-41af-9aa9-a850f771285e} 
	static const GUID result = 
	{ 0xcd95ae2a, 0x1ca9, 0x41af, { 0x9a, 0xa9, 0xa8, 0x50, 0xf7, 0x71, 0x28, 0x5e } };

	return result;
}

int ActionDelay::GetPriority() const
{
	return 50;
}

std::wstring ActionDelay::GetName() const
{
	return L"Delay";
}

IAction* ActionDelay::Clone() const
{
	return new ActionDelay(*this);
}

std::wstring ActionDelay::GetDescription() const
{
	return boost::str(boost::wformat(L"%1% %2% delay") % m_delay % DurationUnits::Label(m_delayUnits));
}

bool ActionDelay::HasConfigDialog() const
{
	return true;
}

bool ActionDelay::ShowConfigDialog(CWindow parent)
{
	ActionDelayEditor dlg(*this);
	return dlg.DoModal(parent) == IDOK;
}

ActionExecSessionPtr ActionDelay::CreateExecSession() const
{
	return ActionExecSessionPtr(new ExecSession(*this));
}

void ActionDelay::LoadFromS11nBlock(const ActionS11nBlock& block)
{
	if (!block.delay.Exists())
		return;

	const ActionDelayS11nBlock& b = block.delay.GetValue();
	b.delay.GetValueIfExists(m_delay);

	if (b.delayUnits.Exists())
		m_delayUnits = static_cast<DurationUnits::Type>(b.delayUnits.GetValue());
}

void ActionDelay::SaveToS11nBlock(ActionS11nBlock& block) const
{
	ActionDelayS11nBlock b;
	b.delay.SetValue(m_delay);
	b.delayUnits.SetValue(m_delayUnits);

	block.delay.SetValue(b);
}

DurationUnits::Type ActionDelay::GetDelayUnits() const
{
	return m_delayUnits;
}

void ActionDelay::SetDelayUnits(DurationUnits::Type delayUnits)
{
	m_delayUnits = delayUnits;
}

namespace
{
	const bool registered = ServiceManager::Instance().GetActionPrototypesManager().RegisterPrototype(
		new ActionDelay);
}

//------------------------------------------------------------------------------
// ActionDelay::ExecSession
//------------------------------------------------------------------------------

ActionDelay::ExecSession::ExecSession(const ActionDelay& action) :
	m_action(action), m_timerID(TimersManager::invalidTimerID)
{
}

ActionDelay::ExecSession::~ExecSession()
{
	if (m_timerID != TimersManager::invalidTimerID)
		ServiceManager::Instance().GetTimersManager().CloseTimer(m_timerID);
}

void ActionDelay::ExecSession::Run(const AsyncCall::CallbackPtr& completionCall)
{
	boost::posix_time::time_duration duration;

	switch (m_action.GetDelayUnits())
	{
	case DurationUnits::seconds:
		duration = boost::posix_time::seconds(m_action.GetDelay());
		break;

	case DurationUnits::minutes:
		duration = boost::posix_time::minutes(m_action.GetDelay());
		break;

	case DurationUnits::hours:
		duration = boost::posix_time::hours(m_action.GetDelay());
		break;
	}

	m_secondsLeft = static_cast<int>(duration.total_seconds());

	m_timerID = ServiceManager::Instance().GetTimersManager().CreateTimer(
		boost::posix_time::second_clock::local_time() + boost::posix_time::seconds(1), boost::posix_time::seconds(1), false);

	AsyncCall::CallbackPtr pTimerCallback = 
		AsyncCall::MakeCallback<ActionDelay::ExecSession>(shared_from_this(),
			boost::bind(&ActionDelay::ExecSession::OnTimer, this));

	m_completionCall = completionCall;

	ServiceManager::Instance().GetTimersManager().StartTimer(m_timerID, pTimerCallback);
}

const IAction* ActionDelay::ExecSession::GetParentAction() const
{
	return &m_action;
}

void ActionDelay::ExecSession::OnTimer()
{
	--m_secondsLeft;

    m_alesFuncs->UpdateDescription();

	if (m_secondsLeft > 0)
		return;

	ServiceManager::Instance().GetTimersManager().CloseTimer(m_timerID);
	m_timerID = TimersManager::invalidTimerID;

	AsyncCall::AsyncRunInMainThread(m_completionCall);
}

void ActionDelay::ExecSession::Init(IActionListExecSessionFuncs& alesFuncs)
{
    m_alesFuncs = &alesFuncs;
}

bool ActionDelay::ExecSession::GetCurrentStateDescription(std::wstring& descr) const
{
	boost::posix_time::time_duration td = boost::posix_time::seconds(m_secondsLeft);
	descr = boost::posix_time::to_simple_wstring(td) + L" left";

	return true;
}

//------------------------------------------------------------------------------
// ActionDelayEditor
//------------------------------------------------------------------------------

ActionDelayEditor::ActionDelayEditor(ActionDelay& action) : m_action(action)
{

}

BOOL ActionDelayEditor::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	std::vector<std::pair<std::wstring, int>> comboItems;

	for (int i = 0; i < DurationUnits::numTypes; ++i)
	{
		comboItems.push_back(std::make_pair(
			DurationUnits::Label(static_cast<DurationUnits::Type>(i)), i));
	}

	m_delayUnitsCombo = GetDlgItem(IDC_COMBO_DELAY_UNITS);
	ComboHelpers::InitCombo(m_delayUnitsCombo, comboItems, m_action.GetDelayUnits());

	CUpDownCtrl spin = ::GetDlgItem(m_hWnd,IDC_SPIN_DELAY);

	spin.SetRange(1, ActionDelay::s_maxDelay);
	spin.SetPos(m_action.GetDelay());

	CenterWindow(GetParent());

	// dark mode
	m_dark.AddDialogWithControls(m_hWnd);

	return TRUE;
}

void ActionDelayEditor::OnCloseCmd(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (nID == IDOK)
	{
		CString s;
		GetDlgItemText(IDC_EDIT_DELAY, s);

		int delay = 0;

		try
		{
			delay = boost::lexical_cast<int>(s.GetString());

			if (delay < 1)
				throw int();
		}
		catch (...)
		{
			m_popupTooltipMsg.Show(L"Invalid delay value.", GetDlgItem(IDC_EDIT_DELAY));
			return;
		}

		m_action.SetDelay(delay);
		m_action.SetDelayUnits(ComboHelpers::GetSelectedItem<DurationUnits::Type>(m_delayUnitsCombo));
	}

	EndDialog(nID);
}