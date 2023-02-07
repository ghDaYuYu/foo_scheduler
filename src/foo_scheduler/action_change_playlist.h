#pragma once

#include "resource.h"
#include "action.h"

//------------------------------------------------------------------------------
// ActionChangePlaylist
//------------------------------------------------------------------------------

class ActionChangePlaylist : public IAction
{
public:
	class ExecSession : public IActionExecSession
	{
	public:
		explicit ExecSession(const ActionChangePlaylist& action);

		virtual void Init(IActionListExecSessionFuncs& alesFuncs);
		virtual void Run(const AsyncCall::CallbackPtr& completionCall);
		virtual const IAction* GetParentAction() const;
		virtual bool GetCurrentStateDescription(std::wstring& descr) const;

	private:
		const ActionChangePlaylist& m_action;
	};

	ActionChangePlaylist();

	enum EChangeType
	{
		ctSelectPlaylist = 0,
		ctNextPlaylist,
		ctPrevPlaylist,
		ctActivePlaylist,
	};

	EChangeType GetChangeType() const;
	void SetChangeType(EChangeType type);

	std::wstring GetPlaylist() const;
	void SetPlaylist(const std::wstring& playlist);

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
	EChangeType m_changeType;
	std::wstring m_playlist; // Used only when m_changeType == ctSelectPlaylist
};

//------------------------------------------------------------------------------
// ActionChangePlaylistEditor
//------------------------------------------------------------------------------

class ActionChangePlaylistEditor :
	public CDialogImpl<ActionChangePlaylistEditor>,
	public CWinDataExchange<ActionChangePlaylistEditor>,
	public fb2k::CDarkModeHooks
{
public:
	enum { IDD = IDD_ACTION_CHANGE_PLAYLIST_CONFIG };

	explicit ActionChangePlaylistEditor(ActionChangePlaylist& action);

private:
	BEGIN_MSG_MAP_EX(ActionChangePlaylistEditor)
		MSG_WM_INITDIALOG(OnInitDialog)

		COMMAND_ID_HANDLER_EX(IDC_RADIO_SELECT_PLAYLIST, OnChangeTypeSelected)
		COMMAND_ID_HANDLER_EX(IDC_RADIO_NEXT_PLAYLIST, OnChangeTypeSelected)
		COMMAND_ID_HANDLER_EX(IDC_RADIO_PREV_PLAYLIST, OnChangeTypeSelected)
		COMMAND_ID_HANDLER_EX(IDC_RADIO_ACTIVE_PLAYLIST, OnChangeTypeSelected)

		COMMAND_ID_HANDLER_EX(IDOK,     OnCloseCmd)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	BEGIN_DDX_MAP(ActionChangePlaylistEditor)
		DDX_TEXT(IDC_COMBO_PLAYLIST, m_playlist)
		DDX_RADIO(IDC_RADIO_SELECT_PLAYLIST, m_changeType)
	END_DDX_MAP()

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);

	void OnCloseCmd(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnChangeTypeSelected(UINT uNotifyCode, int nID, CWindow wndCtl);

private:
	void UpdateControlsState();
	void InitPlaylistCombo();

private:
	ActionChangePlaylist& m_action;

	int m_changeType;
	CString m_playlist;

	CComboBox m_comboPlaylist;
};