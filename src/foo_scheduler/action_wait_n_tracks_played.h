#pragma once

#include "resource.h"
#include "action.h"
#include "popup_tooltip_message.h"

//------------------------------------------------------------------------------
// ActionWaitNTracksPlayed
//------------------------------------------------------------------------------

class ActionWaitNTracksPlayed : public IAction
{
public:
	class ExecSession
		: public IActionExecSession
		, public boost::enable_shared_from_this<ExecSession>
		, public play_callback
	{
	public:
		explicit ExecSession(const ActionWaitNTracksPlayed& action);
		~ExecSession();

		virtual void Init(IActionListExecSessionFuncs& alesFuncs);
		virtual void Run(const AsyncCall::CallbackPtr& completionCall);

		void SubscribeToPlayerEvents();
		virtual const IAction* GetParentAction() const;
		virtual bool GetCurrentStateDescription(std::wstring& descr) const;

	private: // play_callback
		virtual void on_playback_stop(playback_control::t_stop_reason p_reason);
		virtual void on_playback_pause(bool p_state) {}
		virtual void on_playback_starting(play_control::t_track_command p_command, bool p_paused);
		virtual void on_playback_new_track(metadb_handle_ptr p_track);
		virtual void on_playback_seek(double p_time) {}
		virtual void on_playback_edited(metadb_handle_ptr p_track) {}
		virtual void on_playback_dynamic_info(const file_info & p_info) {}
		virtual void on_playback_dynamic_info_track(const file_info & p_info) {}
		virtual void on_playback_time(double p_time) {}
		virtual void on_volume_change(float p_new_val) {}

	private:
		void OnSessionCompleted();
		void UnsubscribeFromPlayerEvents();

	private:
		const ActionWaitNTracksPlayed& m_action;
		AsyncCall::CallbackPtr m_completionCall;
        IActionListExecSessionFuncs* m_alesFuncs = nullptr;

		int m_tracksLeft;
		bool m_subscribedToPlayerEvents;
		bool m_ignoreNextTrackEvent;
	};

	ActionWaitNTracksPlayed();

	int GetNumTracks() const;
	void SetNumTracks(int numTracks);

	static const int s_maxNumTracks = 1000000;

public: // IAction
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
	int m_numTracks;
};

//------------------------------------------------------------------------------
// ActionDelayEditor
//------------------------------------------------------------------------------

class ActionWaitNTracksEditor : public CDialogImpl<ActionWaitNTracksEditor>
{
public:
	enum { IDD = IDD_ACTION_WAIT_N_TRACKS_CONFIG };

	explicit ActionWaitNTracksEditor(ActionWaitNTracksPlayed& action);

private:
	BEGIN_MSG_MAP_EX(ActionWaitNTracksEditor)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDC_CHECK_NTRACKS_EOF, OnCheckEOF)
		COMMAND_ID_HANDLER_EX(IDOK,     OnCloseCmd)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);
	void OnCheckEOF(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnCloseCmd(UINT uNotifyCode, int nID, CWindow wndCtl);

	void enable_controls(bool eof);

	PopupTooltipMessage m_popupTooltipMsg;
	CButton m_checkEOF;
	int m_bk_numtracks = 1;

private:
	ActionWaitNTracksPlayed& m_action;
	fb2k::CDarkModeHooks m_dark;
};

