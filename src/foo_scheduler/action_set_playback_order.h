#pragma once

#include "resource.h"
#include "action.h"

//------------------------------------------------------------------------------
// ActionSetPlaybackOrder
//------------------------------------------------------------------------------

class ActionSetPlaybackOrder : public IAction
{
public:
	class ExecSession : public IActionExecSession
	{
	public:
		explicit ExecSession(const ActionSetPlaybackOrder& action);

		virtual void Init(IActionListExecSessionFuncs& alesFuncs);
		virtual void Run(const AsyncCall::CallbackPtr& completionCall);
		virtual const IAction* GetParentAction() const;
		virtual bool GetCurrentStateDescription(std::wstring& descr) const;

	private:
		const ActionSetPlaybackOrder& m_action;
	};

	ActionSetPlaybackOrder();

	GUID GetOrderGUID() const;
	void SetOrderGUID(const GUID& guid);

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
	GUID m_orderGUID;
};

//------------------------------------------------------------------------------
// ActionSetPlaybackOrderEditor
//------------------------------------------------------------------------------

class ActionSetPlaybackOrderEditor : public CDialogImpl<ActionSetPlaybackOrderEditor>, public fb2k::CDarkModeHooks
{
public:
	enum { IDD = IDD_ACTION_SET_PLAY_ORDER_CONFIG };

	explicit ActionSetPlaybackOrderEditor(ActionSetPlaybackOrder& action);

private:
	ActionSetPlaybackOrder& m_action;

private:
	BEGIN_MSG_MAP_EX(ActionSetPlaybackOrderEditor)
		MSG_WM_INITDIALOG(OnInitDialog)

		COMMAND_ID_HANDLER_EX(IDOK,     OnCloseCmd)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);
	void OnCloseCmd(UINT uNotifyCode, int nID, CWindow wndCtl);

	CComboBox m_orderCombo;

	std::vector<GUID> m_orderGUIDs;
};