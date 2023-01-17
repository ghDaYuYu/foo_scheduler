#pragma once

#include "resource.h"
#include "action.h"
#include "popup_tooltip_message.h"

class ActionStartPlayback : public IAction
{
public:
	class ExecSession
		: public IActionExecSession
		, public boost::enable_shared_from_this<ExecSession>
		, public play_callback
	{
	public:
		explicit ExecSession(const ActionStartPlayback& action);

		virtual void Init(IActionListExecSessionFuncs& alesFuncs);
		virtual void Run(const AsyncCall::CallbackPtr& completionCall);
		virtual const IAction* GetParentAction() const;
		virtual bool GetCurrentStateDescription(std::wstring& descr) const;

	private: // play_callback
		virtual void on_playback_stop(playback_control::t_stop_reason p_reason) {}
		virtual void on_playback_pause(bool p_state) {}
		virtual void on_playback_starting(play_control::t_track_command p_command, bool p_paused) {}
		virtual void on_playback_new_track(metadb_handle_ptr p_track);
		virtual void on_playback_seek(double p_time) {}
		virtual void on_playback_edited(metadb_handle_ptr p_track) {}
		virtual void on_playback_dynamic_info(const file_info & p_info) {}
		virtual void on_playback_dynamic_info_track(const file_info & p_info) {}
		virtual void on_playback_time(double p_time) {}
		virtual void on_volume_change(float p_new_val) {}

	private:
		const ActionStartPlayback& m_action;
        IActionListExecSessionFuncs* m_alesFuncs = nullptr;
		AsyncCall::CallbackPtr m_completionCall;
	};

public:
	ActionStartPlayback();

	enum EStartType
	{
		startTypeOrdinary,
		startTypeFromTrack,
        startTypeFromSavedState
	};

	EStartType GetStartPlaybackType() const;
	void SetStartPlaybackType(EStartType startType);

	t_uint32 GetStartTrackNumber() const;
	void SetStartTrackNumber(t_uint32 track);

public:
	virtual GUID GetPrototypeGUID() const;
	virtual int GetPriority() const;
	virtual std::wstring GetName() const;
	virtual IAction* Clone() const;

	virtual std::wstring GetDescription() const;
	virtual bool HasConfigDialog() const;
	virtual bool ShowConfigDialog(CWindow parent);
	virtual ActionExecSessionPtr CreateExecSession() const;

	virtual void LoadFromS11nBlock(const ActionS11nBlock& block);
	virtual void SaveToS11nBlock(ActionS11nBlock& block) const;

private:
	ActionStartPlayback(const ActionStartPlayback& rhs);

private:
	EStartType m_playbackStartType;
	t_uint32 m_startTrackNumber;
};

//------------------------------------------------------------------------------
// ActionStartPlaybackEditor
//------------------------------------------------------------------------------

class ActionStartPlaybackEditor :
	public CDialogImpl<ActionStartPlaybackEditor>,
	public CWinDataExchange<ActionStartPlaybackEditor>,
	public fb2k::CDarkModeHooks
{
public:
	enum { IDD = IDD_ACTION_START_PLAYBACK_CONFIG };

	explicit ActionStartPlaybackEditor(ActionStartPlayback* pAction);

private:
	ActionStartPlayback* m_pAction;

	int m_playbackStartType;

private:
	BEGIN_MSG_MAP_EX(ActionStartPlaybackEditor)
		MSG_WM_INITDIALOG(OnInitDialog)

		COMMAND_ID_HANDLER_EX(IDC_BTN_PICK_CURRENT_TRACK, OnPickCurrentTrack)
		COMMAND_ID_HANDLER_EX(IDC_RADIO_ORDINARY_START, OnStartTypeSelected)
		COMMAND_ID_HANDLER_EX(IDC_RADIO_START_FROM_TRACK, OnStartTypeSelected)
        COMMAND_ID_HANDLER_EX(IDC_RADIO_START_FROM_SAVED_STATE, OnStartTypeSelected)

		COMMAND_ID_HANDLER_EX(IDOK,     OnCloseCmd)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCloseCmd)
		END_MSG_MAP()

		BEGIN_DDX_MAP(ActionStartPlaybackEditor)
			DDX_RADIO(IDC_RADIO_ORDINARY_START, m_playbackStartType)
		END_DDX_MAP()

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);

	void OnPickCurrentTrack(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnCloseCmd(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnStartTypeSelected(UINT uNotifyCode, int nID, CWindow wndCtl);

private:
	void UpdateControlsState();

private:
	PopupTooltipMessage m_popupTooltipMsg;
};
