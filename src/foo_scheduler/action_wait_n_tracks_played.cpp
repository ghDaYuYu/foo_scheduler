#include "pch.h"
#include "action_wait_n_tracks_played.h"
#include "service_manager.h"

ActionWaitNTracksPlayed::ActionWaitNTracksPlayed() : m_numTracks(1)
{
}

int ActionWaitNTracksPlayed::GetNumTracks() const
{
	return m_numTracks;
}

void ActionWaitNTracksPlayed::SetNumTracks(int numTracks)
{
	m_numTracks = numTracks;
}

GUID ActionWaitNTracksPlayed::GetPrototypeGUID() const
{
	// {19E9ABFA-E6A1-4E7A-84C2-2715C7775936}
	static const GUID result = 
	{ 0x19e9abfa, 0xe6a1, 0x4e7a, { 0x84, 0xc2, 0x27, 0x15, 0xc7, 0x77, 0x59, 0x36 } };

	return result;
}

int ActionWaitNTracksPlayed::GetPriority() const
{
	return 27;
}

std::wstring ActionWaitNTracksPlayed::GetName() const
{
	return L"Wait until N tracks played";
}

IAction* ActionWaitNTracksPlayed::Clone() const
{
	return new ActionWaitNTracksPlayed(*this);
}

std::wstring ActionWaitNTracksPlayed::GetDescription() const
{
	return boost::str(boost::wformat(L"Wait until %1% tracks played") % m_numTracks);
}

bool ActionWaitNTracksPlayed::HasConfigDialog() const
{
	return true;
}

bool ActionWaitNTracksPlayed::ShowConfigDialog(CWindow parent)
{
	ActionWaitNTracksEditor dlg(*this);
	return dlg.DoModal(parent) == IDOK;
}

ActionExecSessionPtr ActionWaitNTracksPlayed::CreateExecSession() const
{
	return ActionExecSessionPtr(new ExecSession(*this));
}

void ActionWaitNTracksPlayed::LoadFromS11nBlock(const ActionS11nBlock& block)
{
	if (!block.waitNTracksPlayed.Exists())
		return;

	const ActionWaitNTracksPlayedS11nBlock& b = block.waitNTracksPlayed.GetValue();
	b.numTracks.GetValueIfExists(m_numTracks);
}

void ActionWaitNTracksPlayed::SaveToS11nBlock(ActionS11nBlock& block) const
{
	ActionWaitNTracksPlayedS11nBlock b;
	b.numTracks.SetValue(m_numTracks);

	block.waitNTracksPlayed.SetValue(b);
}

namespace
{
	const bool registered = ServiceManager::Instance().GetActionPrototypesManager().RegisterPrototype(
		new ActionWaitNTracksPlayed);
}

//------------------------------------------------------------------------------
// ActionDelay::ExecSession
//------------------------------------------------------------------------------

ActionWaitNTracksPlayed::ExecSession::ExecSession(const ActionWaitNTracksPlayed& action)
	: m_action(action), m_tracksLeft(action.GetNumTracks())
	, m_subscribedToPlayerEvents(false)
	, m_ignoreNextTrackEvent(false)
{
}

ActionWaitNTracksPlayed::ExecSession::~ExecSession()
{
	UnsubscribeFromPlayerEvents();
}

void ActionWaitNTracksPlayed::ExecSession::Run(const AsyncCall::CallbackPtr& completionCall)
{
	m_completionCall = completionCall;
	SubscribeToPlayerEvents();
}

const IAction* ActionWaitNTracksPlayed::ExecSession::GetParentAction() const
{
	return &m_action;
}

void ActionWaitNTracksPlayed::ExecSession::Init(IActionListExecSessionFuncs& alesFuncs)
{
    m_alesFuncs = &alesFuncs;
}

bool ActionWaitNTracksPlayed::ExecSession::GetCurrentStateDescription(std::wstring& descr) const
{
	descr = boost::str(boost::wformat(L"%1% tracks left") % m_tracksLeft);
	return true;
}

void ActionWaitNTracksPlayed::ExecSession::on_playback_new_track(metadb_handle_ptr p_track)
{
	if (m_ignoreNextTrackEvent)
	{
		m_ignoreNextTrackEvent = false;
		return;
	}

	--m_tracksLeft;

	if (m_tracksLeft == 0)
	{
		// Do not unregister from player events in on_playback_, so need async call.
		AsyncCall::CallbackPtr completionCall = AsyncCall::MakeCallback<ExecSession>(
			shared_from_this(), boost::mem_fn(&ExecSession::OnSessionCompleted));

		AsyncCall::AsyncRunInMainThread(completionCall);
	}

    m_alesFuncs->UpdateDescription();
}

void ActionWaitNTracksPlayed::ExecSession::OnSessionCompleted()
{
	UnsubscribeFromPlayerEvents();
	AsyncCall::AsyncRunInMainThread(m_completionCall);
}

void ActionWaitNTracksPlayed::ExecSession::UnsubscribeFromPlayerEvents()
{
	if (!m_subscribedToPlayerEvents)
		return;

	static_api_ptr_t<play_callback_manager>()->unregister_callback(this);
	m_subscribedToPlayerEvents = false;
}

void ActionWaitNTracksPlayed::ExecSession::SubscribeToPlayerEvents()
{
	static_api_ptr_t<play_callback_manager>()->register_callback(this,
		flag_on_playback_starting | flag_on_playback_stop | flag_on_playback_new_track, false);
	m_subscribedToPlayerEvents = true;
}

void ActionWaitNTracksPlayed::ExecSession::on_playback_starting(play_control::t_track_command p_command, bool p_paused)
{
	m_ignoreNextTrackEvent = true;
}

void ActionWaitNTracksPlayed::ExecSession::on_playback_stop(playback_control::t_stop_reason p_reason)
{
	m_ignoreNextTrackEvent = true;
}

//------------------------------------------------------------------------------
// ActionWaitNTracksEditor
//------------------------------------------------------------------------------

ActionWaitNTracksEditor::ActionWaitNTracksEditor(ActionWaitNTracksPlayed& action) : m_action(action)
{

}

BOOL ActionWaitNTracksEditor::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	SetDlgItemInt(IDC_EDIT_N_TRACKS, m_action.GetNumTracks());
	CenterWindow(GetParent());

	// dark mode
	AddDialog(m_hWnd);
	AddControls(m_hWnd);

	return TRUE;
}

void ActionWaitNTracksEditor::OnCloseCmd(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (nID == IDOK)
	{
		int numTracks = GetDlgItemInt(IDC_EDIT_N_TRACKS, 0, 0);

		if (numTracks < 1)
		{
			m_popupTooltipMsg.Show(L"Invalid value.", GetDlgItem(IDC_EDIT_N_TRACKS));
			return;
		}

		m_action.SetNumTracks(numTracks);
	}

	EndDialog(nID);
}
