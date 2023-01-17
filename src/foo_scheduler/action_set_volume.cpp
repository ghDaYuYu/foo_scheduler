#include "pch.h"
#include "action_set_volume.h"
#include "service_manager.h"
#include "combo_helpers.h"

//------------------------------------------------------------------------------
// ActionSetVolume
//------------------------------------------------------------------------------

ActionSetVolume::ActionSetVolume() :
	m_volume(0.0), m_useFade(false), m_fadeDuration(1), m_fadeDurationUnits(DurationUnits::seconds)
{

}

float ActionSetVolume::GetVolume() const
{
	return m_volume;
}

void ActionSetVolume::SetVolume(float val)
{
	m_volume = val;
}

bool ActionSetVolume::GetUseFade() const
{
	return m_useFade;
}

void ActionSetVolume::SetUseFade(bool val)
{
	m_useFade = val;
}

int ActionSetVolume::GetFadeDuration() const
{
	return m_fadeDuration;
}

void ActionSetVolume::SetFadeDuration(int val)
{
	m_fadeDuration = val;
}

DurationUnits::Type ActionSetVolume::GetFadeDurationUnits() const
{
	return m_fadeDurationUnits;
}

void ActionSetVolume::SetFadeDurationUnits(DurationUnits::Type val)
{
	m_fadeDurationUnits = val;
}

GUID ActionSetVolume::GetPrototypeGUID() const
{
	// {91dda9a1-b9b3-44c2-87a5-4abf4786c017} 
	static const GUID result = 
	{ 0x91dda9a1, 0xb9b3, 0x44c2, { 0x87, 0xa5, 0x4a, 0xbf, 0x47, 0x86, 0xc0, 0x17 } };

	return result;
}

int ActionSetVolume::GetPriority() const
{
	return 35;
}

std::wstring ActionSetVolume::GetName() const
{
	return L"Set volume";
}

IAction* ActionSetVolume::Clone() const
{
	return new ActionSetVolume(*this);
}

std::wstring ActionSetVolume::GetDescription() const
{
	std::wstring result;

	if (m_useFade)
	{
		boost::wformat fmt(L"Set volume to %1$.2f dB with fade during %2% %3%");
		fmt % m_volume % m_fadeDuration % DurationUnits::Label(m_fadeDurationUnits);
		result = fmt.str();
	}
	else
	{
		boost::wformat fmt(L"Set volume to %1$.2f dB");
		fmt % m_volume;
		result = fmt.str();
	}

	return result;
}

bool ActionSetVolume::HasConfigDialog() const
{
	return true;
}

bool ActionSetVolume::ShowConfigDialog(CWindow parent)
{
	ActionSetVolumeEditor dlg(*this);
	return dlg.DoModal(parent) == IDOK;
}

ActionExecSessionPtr ActionSetVolume::CreateExecSession() const
{
	return ActionExecSessionPtr(new ExecSession(*this));
}

void ActionSetVolume::LoadFromS11nBlock(const ActionS11nBlock& block)
{
	if (!block.setVolume.Exists())
		return;

	const ActionSetVolumeS11nBlock& b = block.setVolume.GetValue();

	b.volume.GetValueIfExists(m_volume);
	b.useFade.GetValueIfExists(m_useFade);
	b.fadeDuration.GetValueIfExists(m_fadeDuration);

	if (b.fadeDurationUnits.Exists())
		m_fadeDurationUnits = static_cast<DurationUnits::Type>(b.fadeDurationUnits.GetValue());
}

void ActionSetVolume::SaveToS11nBlock(ActionS11nBlock& block) const
{
	ActionSetVolumeS11nBlock b;

	b.volume.SetValue(m_volume);
	b.useFade.SetValue(m_useFade);

	if (m_useFade)
	{
		b.fadeDuration.SetValue(m_fadeDuration);
		b.fadeDurationUnits.SetValue(m_fadeDurationUnits);
	}

	block.setVolume.SetValue(b);
}

namespace
{
	const bool registered = ServiceManager::Instance().GetActionPrototypesManager().RegisterPrototype(
		new ActionSetVolume);
}

//------------------------------------------------------------------------------
// ActionSetVolume::ExecSession
//------------------------------------------------------------------------------

ActionSetVolume::ExecSession::ExecSession(const ActionSetVolume& action) :
	m_action(action), m_power(0.0f), m_powerStep(0.0f), m_timerID(TimersManager::invalidTimerID), m_secondsLeft(0)
{
}

ActionSetVolume::ExecSession::~ExecSession()
{
	if (m_timerID != TimersManager::invalidTimerID)
		ServiceManager::Instance().GetTimersManager().CloseTimer(m_timerID);
}

void ActionSetVolume::ExecSession::Run(const AsyncCall::CallbackPtr& completionCall)
{
	m_completionCall = completionCall;

	if (m_action.GetUseFade())
		RunWithFade();
	else
		RunWithoutFade();
}

const IAction* ActionSetVolume::ExecSession::GetParentAction() const
{
	return &m_action;
}

void ActionSetVolume::ExecSession::OnTimer()
{
	static_api_ptr_t<playback_control> pc;

	m_power += m_powerStep;

	pc->set_volume(20.0f * std::log10f(m_power));

	--m_secondsLeft;

    m_alesFuncs->UpdateDescription();

	if (m_secondsLeft > 0)
		return;

	ServiceManager::Instance().GetTimersManager().CloseTimer(m_timerID);
	m_timerID = TimersManager::invalidTimerID;

	AsyncCall::AsyncRunInMainThread(m_completionCall);
}

void ActionSetVolume::ExecSession::Init(IActionListExecSessionFuncs& alesFuncs)
{
    m_alesFuncs = &alesFuncs;
}

bool ActionSetVolume::ExecSession::GetCurrentStateDescription(std::wstring& descr) const
{
	boost::posix_time::time_duration td = boost::posix_time::seconds(m_secondsLeft);
	descr = boost::posix_time::to_simple_wstring(td) + L" left";

	return true;
}

void ActionSetVolume::ExecSession::RunWithFade()
{
	boost::posix_time::time_duration fadeDuration;

	switch (m_action.GetFadeDurationUnits())
	{
	case DurationUnits::seconds:
		fadeDuration = boost::posix_time::seconds(m_action.GetFadeDuration());
		break;

	case DurationUnits::minutes:
		fadeDuration = boost::posix_time::minutes(m_action.GetFadeDuration());
		break;

	case DurationUnits::hours:
		fadeDuration = boost::posix_time::hours(m_action.GetFadeDuration());
		break;
	}

	static_api_ptr_t<playback_control> pc;
	m_secondsLeft = static_cast<int>(fadeDuration.total_seconds());

	float startPower = std::powf(10.0f, pc->get_volume() / 20.0f);
	float endPower = std::powf(10.0f, m_action.GetVolume() / 20.0f);

	m_power = startPower;
	m_powerStep = (endPower - startPower) / static_cast<float>(fadeDuration.total_seconds());

	m_timerID = ServiceManager::Instance().GetTimersManager().CreateTimer(
		boost::posix_time::second_clock::local_time() + boost::posix_time::seconds(1), boost::posix_time::seconds(1), false);

	AsyncCall::CallbackPtr pTimerCallback = AsyncCall::MakeCallback<ActionSetVolume::ExecSession>(shared_from_this(),
		boost::bind(&ActionSetVolume::ExecSession::OnTimer, this));

	ServiceManager::Instance().GetTimersManager().StartTimer(m_timerID, pTimerCallback);
}

void ActionSetVolume::ExecSession::RunWithoutFade()
{
	static_api_ptr_t<playback_control> pc;
	pc->set_volume(m_action.GetVolume());

	AsyncCall::AsyncRunInMainThread(m_completionCall);
}

//------------------------------------------------------------------------------
// ActionSetVolumeEditor
//------------------------------------------------------------------------------

ActionSetVolumeEditor::ActionSetVolumeEditor(ActionSetVolume& action) : m_action(action), m_volume(0.0)
{

}

BOOL ActionSetVolumeEditor::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	m_volume = m_action.GetVolume();
	SetVolumeLabel();

	m_checkUseFade = GetDlgItem(IDC_CHECK_USE_FADE);
	m_checkUseFade.SetCheck(m_action.GetUseFade());

	EnableFadeDurationControls(m_action.GetUseFade());

	std::vector<std::pair<std::wstring, int>> comboItems;

	for (int i = 0; i < DurationUnits::numTypes; ++i)
	{
		comboItems.push_back(std::make_pair(
			DurationUnits::Label(static_cast<DurationUnits::Type>(i)), i));
	}

	m_durationUnitsCombo = GetDlgItem(IDC_COMBO_DURATION_UNITS);
	ComboHelpers::InitCombo(m_durationUnitsCombo, comboItems, m_action.GetFadeDurationUnits());

	CUpDownCtrl spin = ::GetDlgItem(m_hWnd, IDC_SPIN_DURATION);

	spin.SetRange(1, ActionSetVolume::s_maxFadeDuration);
	spin.SetPos(m_action.GetFadeDuration());

	CenterWindow(GetParent());

	return TRUE;

}

void ActionSetVolumeEditor::EnableFadeDurationControls(bool enable)
{
	GetDlgItem(IDC_STATIC_DURATION).EnableWindow(enable);
	GetDlgItem(IDC_EDIT_DURATION).EnableWindow(enable);
	GetDlgItem(IDC_COMBO_DURATION_UNITS).EnableWindow(enable);
}

void ActionSetVolumeEditor::SetVolumeLabel()
{
	boost::wformat fmt(_T("%1$.2f dB"));
	fmt % m_volume;

	GetDlgItem(IDC_STATIC_VOLUME).SetWindowText(fmt.str().c_str());
}

void ActionSetVolumeEditor::OnCloseCmd(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (nID == IDOK)
	{
		m_action.SetVolume(m_volume);
		m_action.SetUseFade(m_checkUseFade.GetCheck() != 0);

		if (m_action.GetUseFade())
		{
			CString s;
			GetDlgItemText(IDC_EDIT_DURATION, s);

			int duration = 0;

			try
			{
				duration = boost::lexical_cast<int>(s.GetString());

				if (duration < 1)
					throw int();
			}
			catch (...)
			{
				m_popupTooltipMsg.Show(L"Invalid duration value.", GetDlgItem(IDC_EDIT_DURATION));
				return;
			}

			m_action.SetFadeDuration(duration);
			m_action.SetFadeDurationUnits(ComboHelpers::GetSelectedItem<DurationUnits::Type>(m_durationUnitsCombo));
		}
		else
		{
			m_action.SetFadeDuration(1);
			m_action.SetFadeDurationUnits(DurationUnits::seconds);
		}
	}

	EndDialog(nID);
}

void ActionSetVolumeEditor::OnPick(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	static_api_ptr_t<playback_control> pc;
	m_volume = pc->get_volume();

	SetVolumeLabel();
}

void ActionSetVolumeEditor::OnCheckUseFade(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	EnableFadeDurationControls(m_checkUseFade.GetCheck() != 0);
}