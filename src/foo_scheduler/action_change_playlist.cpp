#include "pch.h"
#include "action_change_playlist.h"
#include "action_change_playlist_s11n_block.h"
#include "service_manager.h"
#include "scope_exit_function.h"

ActionChangePlaylist::ActionChangePlaylist() : m_changeType(ctSelectPlaylist)
{
}

GUID ActionChangePlaylist::GetPrototypeGUID() const
{
	// {6a66bb98-4c5b-4ddd-b208-6b5799952707} 
	static const GUID result = 
	{ 0x6a66bb98, 0x4c5b, 0x4ddd, { 0xb2, 0x08, 0x6b, 0x57, 0x99, 0x95, 0x27, 0x07 } };

	return result;
}

int ActionChangePlaylist::GetPriority() const
{
	return 30;
}

std::wstring ActionChangePlaylist::GetName() const
{
	return L"Change playlist";
}

IAction* ActionChangePlaylist::Clone() const
{
	return new ActionChangePlaylist(*this);
}

std::wstring ActionChangePlaylist::GetDescription() const
{
	switch (m_changeType)
	{
	case ctSelectPlaylist:
		return std::wstring(L"Set active playlist \"") + m_playlist + L"\"";

	case ctNextPlaylist:
		return L"Next playlist";

	case ctPrevPlaylist:
		return L"Previous playlist";
	}

	_ASSERTE(false);
	return std::wstring();
}

bool ActionChangePlaylist::HasConfigDialog() const
{
	return true;
}

bool ActionChangePlaylist::ShowConfigDialog(CWindow parent)
{
	ActionChangePlaylistEditor dlg(*this);
	return dlg.DoModal(parent) == IDOK;
}

ActionExecSessionPtr ActionChangePlaylist::CreateExecSession() const
{
	return ActionExecSessionPtr(new ExecSession(*this));
}

void ActionChangePlaylist::LoadFromS11nBlock(const ActionS11nBlock& block)
{
	if (!block.changePlaylist.Exists())
		return;

	const ActionChangePlaylistS11nBlock& b = block.changePlaylist.GetValue();

	if (b.changeType.Exists())
		m_changeType = static_cast<EChangeType>(b.changeType.GetValue());

	if (b.playlist.Exists())
		m_playlist = pfc::stringcvt::string_wide_from_utf8(b.playlist.GetValue()).get_ptr();
}

void ActionChangePlaylist::SaveToS11nBlock(ActionS11nBlock& block) const
{
	ActionChangePlaylistS11nBlock b;

	b.changeType.SetValue(m_changeType);

	if (m_changeType == ctSelectPlaylist)
		b.playlist.SetValue(pfc::stringcvt::string_utf8_from_wide(m_playlist.c_str()).toString());

	block.changePlaylist.SetValue(b);
}

ActionChangePlaylist::EChangeType ActionChangePlaylist::GetChangeType() const
{
	return m_changeType;
}

void ActionChangePlaylist::SetChangeType(EChangeType type)
{
	m_changeType = type;
}

std::wstring ActionChangePlaylist::GetPlaylist() const
{
	return m_playlist;
}

void ActionChangePlaylist::SetPlaylist(const std::wstring& playlist)
{
	m_playlist = playlist;
}

namespace
{
	const bool registered = ServiceManager::Instance().GetActionPrototypesManager().RegisterPrototype(
		new ActionChangePlaylist);
}

//------------------------------------------------------------------------------
// ActionChangePlaylist::ExecSession
//------------------------------------------------------------------------------

ActionChangePlaylist::ExecSession::ExecSession(const ActionChangePlaylist& action) : m_action(action)
{

}

void ActionChangePlaylist::ExecSession::Run(const AsyncCall::CallbackPtr& completionCall)
{
	ScopeExitFunction scopeExit(boost::bind(&AsyncCall::AsyncRunInMainThread, completionCall));

	static_api_ptr_t<playlist_manager> pm;

	t_size playlistIndex = pfc::infinite_size;

	switch (m_action.GetChangeType())
	{
	case ActionChangePlaylist::ctSelectPlaylist:
		{
			pfc::string8 utf8playlist = pfc::stringcvt::string_utf8_from_wide(m_action.GetPlaylist().c_str()).toString();
			
			playlistIndex = pm->find_playlist(utf8playlist, utf8playlist.length());

			if (playlistIndex == pfc::infinite_size)
				return;
		}
		break;

	case ActionChangePlaylist::ctNextPlaylist:
	case ActionChangePlaylist::ctPrevPlaylist:
		{
			t_size plCount = pm->get_playlist_count();

			if (plCount == 0)
				return;

			playlistIndex = pm->get_active_playlist();

			if (playlistIndex == pfc::infinite_size)
				playlistIndex = pm->get_playing_playlist();

			if (m_action.GetChangeType() == ActionChangePlaylist::ctNextPlaylist)
			{
				if (playlistIndex == plCount - 1)
					playlistIndex = 0;
				else
					++playlistIndex;
			}
			else
			{
				if (playlistIndex == 0)
					playlistIndex = plCount - 1;
				else
					--playlistIndex;
			}
		}
		break;
	}

	pm->queue_flush();
	pm->set_active_playlist(playlistIndex);
	pm->reset_playing_playlist();
}

const IAction* ActionChangePlaylist::ExecSession::GetParentAction() const
{
	return &m_action;
}

void ActionChangePlaylist::ExecSession::Init(IActionListExecSessionFuncs&)
{
}

bool ActionChangePlaylist::ExecSession::GetCurrentStateDescription(std::wstring& /*descr*/) const
{
	return false;
}

//------------------------------------------------------------------------------
// ActionChangePlaylistEditor
//------------------------------------------------------------------------------

ActionChangePlaylistEditor::ActionChangePlaylistEditor(ActionChangePlaylist& action) :
	m_action(action), m_changeType(action.GetChangeType()), m_playlist(action.GetPlaylist().c_str())
{
}

BOOL ActionChangePlaylistEditor::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	InitPlaylistCombo();

	DoDataExchange(DDX_LOAD);
	UpdateControlsState();
	CenterWindow(GetParent());

	// dark mode
	AddDialog(m_hWnd);
	AddControls(m_hWnd);

	return TRUE;
}

void ActionChangePlaylistEditor::OnCloseCmd(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (nID == IDOK)
	{
		if (!DoDataExchange(DDX_SAVE))
			return;

		m_action.SetChangeType(static_cast<ActionChangePlaylist::EChangeType>(m_changeType));
		m_action.SetPlaylist(m_changeType == ActionChangePlaylist::ctSelectPlaylist ? m_playlist.GetString() : std::wstring());
	}

	EndDialog(nID);
}

void ActionChangePlaylistEditor::OnChangeTypeSelected(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	DoDataExchange(DDX_SAVE);
	UpdateControlsState();
}

void ActionChangePlaylistEditor::UpdateControlsState()
{
	m_comboPlaylist.EnableWindow(m_changeType == ActionChangePlaylist::ctSelectPlaylist);
}

void ActionChangePlaylistEditor::InitPlaylistCombo()
{
	m_comboPlaylist = GetDlgItem(IDC_COMBO_PLAYLIST);

	static_api_ptr_t<playlist_manager> pm;

	for (t_size i = 0; i < pm->get_playlist_count(); ++i)
	{
		pfc::string8 s;
		pm->playlist_get_name(i, s);

		pfc::stringcvt::string_os_from_utf8 strPlayList(s);

		m_comboPlaylist.AddString(strPlayList);
	}
}