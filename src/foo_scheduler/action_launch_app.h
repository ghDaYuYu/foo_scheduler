#pragma once

#include "resource.h"
#include "action.h"

//------------------------------------------------------------------------------
// ActionLaunchApp
//------------------------------------------------------------------------------

class ActionLaunchApp : public IAction
{
public:
	class ExecSession : public IActionExecSession
	{
	public:
		explicit ExecSession(const ActionLaunchApp& action);

		virtual void Init(IActionListExecSessionFuncs& alesFuncs);
		virtual void Run(const AsyncCall::CallbackPtr& completionCall);
		virtual const IAction* GetParentAction() const;
		virtual bool GetCurrentStateDescription(std::wstring& descr) const;

	private:
		const ActionLaunchApp& m_action;
	};

	ActionLaunchApp();

	std::wstring GetCommandLine() const;
	void SetCommandLine(const std::wstring& val);

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
	std::wstring m_commandLine;
};

class ActionLaunchAppEditor : public CDialogImpl<ActionLaunchAppEditor>
{
public:
	enum { IDD = IDD_ACTION_LAUNCH_APP_CONFIG };

	explicit ActionLaunchAppEditor(ActionLaunchApp& action);

private:
	BEGIN_MSG_MAP_EX(ActionLaunchAppEditor)
		MSG_WM_INITDIALOG(OnInitDialog)

		COMMAND_ID_HANDLER_EX(IDC_BTN_OPEN_FILE, OnOpenFile)

		COMMAND_ID_HANDLER_EX(IDOK,     OnCloseCmd)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);
	void OnCloseCmd(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnOpenFile(UINT uNotifyCode, int nID, CWindow wndCtl);

private:
	ActionLaunchApp& m_action;
	fb2k::CDarkModeHooks m_dark;
};