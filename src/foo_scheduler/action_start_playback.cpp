#include "pch.h"
#include "action_start_playback.h"
#include "service_manager.h"
#include "action_start_playback_s11n_block.h"
#include "action_save_playback_state.h"
#include "scope_exit_function.h"

//------------------------------------------------------------------------------
// ActionStartPlayback
//------------------------------------------------------------------------------

ActionStartPlayback::ActionStartPlayback() :
	m_playbackStartType(startTypeOrdinary), m_startTrackNumber(1)
{

}

ActionStartPlayback::ActionStartPlayback(const ActionStartPlayback& rhs) :
	m_playbackStartType(rhs.m_playbackStartType),
	m_startTrackNumber(rhs.m_startTrackNumber)
{

}

std::wstring ActionStartPlayback::GetDescription() const
{
	std::wstring strResult = L"Start playback";

    switch (m_playbackStartType)
    {
    case startTypeOrdinary:
        break;
    case startTypeFromTrack:
        strResult += L" from track #" + boost::lexical_cast<std::wstring>(m_startTrackNumber);
        break;
    case startTypeFromSavedState:
        strResult += L" from saved state";
        break;
    }

    return strResult;
}

IAction* ActionStartPlayback::Clone() const
{
	return new ActionStartPlayback(*this);
}

bool ActionStartPlayback::HasConfigDialog() const
{
	return true;
}

bool ActionStartPlayback::ShowConfigDialog(CWindow parent)
{
	ActionStartPlaybackEditor dlg(this);
	return dlg.DoModal(parent) == IDOK;
}

ActionExecSessionPtr ActionStartPlayback::CreateExecSession() const
{
	return ActionExecSessionPtr(new ExecSession(*this));
}

ActionStartPlayback::EStartType ActionStartPlayback::GetStartPlaybackType() const
{
	return m_playbackStartType;
}

void ActionStartPlayback::SetStartPlaybackType(EStartType startType)
{
	m_playbackStartType = startType;
}

t_uint32 ActionStartPlayback::GetStartTrackNumber() const
{
	return m_startTrackNumber;
}

void ActionStartPlayback::SetStartTrackNumber(t_uint32 track)
{
	m_startTrackNumber = track;
}

GUID ActionStartPlayback::GetPrototypeGUID() const
{
	// {31d42d43-9596-4a28-886f-e23d3cd9866d} 
	static const GUID result = 
	{ 0x31d42d43, 0x9596, 0x4a28, { 0x88, 0x6f, 0xe2, 0x3d, 0x3c, 0xd9, 0x86, 0x6d } };

	return result;
}

int ActionStartPlayback::GetPriority() const
{
	return 0;
}

std::wstring ActionStartPlayback::GetName() const
{
	return L"Start playback";
}

void ActionStartPlayback::LoadFromS11nBlock(const ActionS11nBlock& block)
{
	if (!block.startPlayback.Exists())
		return;

	const ActionStartPlaybackS11nBlock& b = block.startPlayback.GetValue();

	if (b.playbackStartType.Exists())
		m_playbackStartType = static_cast<EStartType>(b.playbackStartType.GetValue());

	b.startTrackNumber.GetValueIfExists(m_startTrackNumber);
}

void ActionStartPlayback::SaveToS11nBlock(ActionS11nBlock& block) const
{
	ActionStartPlaybackS11nBlock b;

	b.playbackStartType.SetValue(m_playbackStartType);

	if (m_playbackStartType == startTypeFromTrack)
		b.startTrackNumber.SetValue(m_startTrackNumber);

	block.startPlayback.SetValue(b);
}

namespace
{
	const bool registered = ServiceManager::Instance().GetActionPrototypesManager().RegisterPrototype(
		new ActionStartPlayback);
}

//------------------------------------------------------------------------------
// ActionStartPlayback::ExecSession
//------------------------------------------------------------------------------

ActionStartPlayback::ExecSession::ExecSession(const ActionStartPlayback& action) : m_action(action)
{
}

void ActionStartPlayback::ExecSession::Run(const AsyncCall::CallbackPtr& completionCall)
{
    static_api_ptr_t<playlist_manager> pm;

	if (m_action.GetStartPlaybackType() == startTypeFromTrack)
	{
		t_size playlist = pm->get_active_playlist();

		if (playlist == pfc::infinite_size)
			playlist = pm->get_playing_playlist();

		pm->queue_flush();
		pm->queue_add_item_playlist(playlist, m_action.GetStartTrackNumber() - 1);
	}
    else if (m_action.GetStartPlaybackType() == startTypeFromSavedState)
    {
        const boost::any playlistAny = m_alesFuncs->GetValue(ActionSavePlaybackState::GetPlaylistKey());
        const boost::any trackAny = m_alesFuncs->GetValue(ActionSavePlaybackState::GetTrackKey());

        const t_size* playlist = boost::any_cast<t_size>(&playlistAny);
        const t_size* track = boost::any_cast<t_size>(&trackAny);

        if (!playlist || !track)
            return;
       
        pm->queue_flush();
        pm->queue_add_item_playlist(*playlist, *track);
    }

    ServiceManager::Instance().GetPlayerEventsManager().BlockEvents(true);

    m_completionCall = completionCall;
    static_api_ptr_t<play_callback_manager>()->register_callback(this, flag_on_playback_new_track, false);   

    // Start playback in paused state.
	static_api_ptr_t<playback_control>()->start(playback_control::track_command_play, true);
}

void ActionStartPlayback::ExecSession::on_playback_new_track(metadb_handle_ptr p_track)
{
	AsyncCall::AsyncRunInMainThread(AsyncCall::MakeCallback<ActionStartPlayback::ExecSession>(
		shared_from_this(),
		[this] (ActionStartPlayback::ExecSession*)
		{
			static_api_ptr_t<play_callback_manager>()->unregister_callback(this);
			
			static_api_ptr_t<playback_control> pc;

            if (m_action.GetStartPlaybackType() == startTypeFromSavedState)
            {
                const boost::any positionAny = m_alesFuncs->GetValue(ActionSavePlaybackState::GetPositionKey());
                if (const double* position = boost::any_cast<double>(&positionAny))
                {
                    pc->playback_seek(*position);
                }
            }

			// We started playback in paused state, see above.
			pc->pause(false);

			ServiceManager::Instance().GetPlayerEventsManager().BlockEvents(false);

			_ASSERTE(m_completionCall);
			AsyncCall::AsyncRunInMainThread(m_completionCall);
		}
	));
}

const IAction* ActionStartPlayback::ExecSession::GetParentAction() const
{
	return &m_action;
}

void ActionStartPlayback::ExecSession::Init(IActionListExecSessionFuncs& alesFuncs)
{
    m_alesFuncs = &alesFuncs;
}

bool ActionStartPlayback::ExecSession::GetCurrentStateDescription(std::wstring& /*descr*/) const
{
	return false;
}

//------------------------------------------------------------------------------
// CDlgActionSelectTrackConfig
//------------------------------------------------------------------------------

ActionStartPlaybackEditor::ActionStartPlaybackEditor(ActionStartPlayback* pAction) :
	m_pAction(pAction),
	m_playbackStartType(pAction->GetStartPlaybackType())
{
}

BOOL ActionStartPlaybackEditor::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	DoDataExchange(DDX_LOAD);

	SetDlgItemInt(IDC_EDIT_TRACK_NO, m_pAction->GetStartTrackNumber());

	UpdateControlsState();

	CenterWindow(GetParent());

	// dark mode
	m_dark.AddDialogWithControls(m_hWnd);

	return TRUE;
}

void ActionStartPlaybackEditor::OnCloseCmd(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (nID == IDOK)
	{
		if (!DoDataExchange(DDX_SAVE))
			return;

		ActionStartPlayback::EStartType startType = static_cast<ActionStartPlayback::EStartType>(m_playbackStartType);
		t_uint32 track = 1;

        if (startType == ActionStartPlayback::startTypeFromTrack)
		{
			CString s;
			GetDlgItemText(IDC_EDIT_TRACK_NO, s);

			try
			{
				track = boost::lexical_cast<t_uint32>(s.GetString());

				if (track < 1)
					throw int();
			}
			catch (...)
			{
				m_popupTooltipMsg.Show(L"Invalid track number.", GetDlgItem(IDC_EDIT_TRACK_NO));
				return;
			}
		}

		m_pAction->SetStartPlaybackType(startType);
		m_pAction->SetStartTrackNumber(track);
	}

	EndDialog(nID);
}

void ActionStartPlaybackEditor::OnPickCurrentTrack(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	static_api_ptr_t<playlist_manager> pm;

	t_size playlist = pm->get_active_playlist();
	
	if (playlist == pfc::infinite_size)
		playlist = pm->get_playing_playlist();

	t_size numTotalTracks = pm->playlist_get_item_count(playlist);
	
	if (numTotalTracks == 0)
		return;

	bit_array_bittable ba(numTotalTracks);
	pm->playlist_get_selection_mask(playlist, ba);

	t_uint32 item = static_cast<t_uint32>(ba.find_first(true, 0, numTotalTracks));

	if (item >= numTotalTracks)
		return;

	SetDlgItemInt(IDC_EDIT_TRACK_NO, item + 1, FALSE);

	DoDataExchange(DDX_LOAD);
}

void ActionStartPlaybackEditor::UpdateControlsState()
{
	bool enable = m_playbackStartType == ActionStartPlayback::startTypeFromTrack;
	GetDlgItem(IDC_EDIT_TRACK_NO).EnableWindow(enable);
	GetDlgItem(IDC_BTN_PICK_CURRENT_TRACK).EnableWindow(enable);
}

void ActionStartPlaybackEditor::OnStartTypeSelected(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	DoDataExchange(DDX_SAVE);
	UpdateControlsState();
}


