#pragma once

#include "resource.h"
#include "action.h"

namespace ShutdownMethod
{
	enum Type
	{
		shutdown = 0,
		poweroff,
		suspend,
		hibernate
	};

	inline std::wstring Label(Type type)
	{
		switch (type)
		{
		case shutdown:
			return L"Shutdown";

		case poweroff:
			return L"Poweroff";

		case suspend:
			return L"Suspend";

		case hibernate:
			return L"Hibernate";
		}

		_ASSERTE(false);
		return std::wstring();
	}

} // namespace ShutdownType

//------------------------------------------------------------------------------
// ActionShutdown
//------------------------------------------------------------------------------

class ActionShutdown : public IAction
{
public:
	class ExecSession : public IActionExecSession
	{
	public:
		explicit ExecSession(const ActionShutdown& action);

		virtual void Init(IActionListExecSessionFuncs& alesFuncs);
		virtual void Run(const AsyncCall::CallbackPtr& completionCall);
		virtual const IAction* GetParentAction() const;
		virtual bool GetCurrentStateDescription(std::wstring& descr) const;

	private:
		const ActionShutdown& m_action;
	};

	ActionShutdown();

	ShutdownMethod::Type GetType() const;
	void SetType(ShutdownMethod::Type type);

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
	ShutdownMethod::Type m_shutdownType;
};

//------------------------------------------------------------------------------
// ActionShutdownEditor
//------------------------------------------------------------------------------

class ActionShutdownEditor : public CDialogImpl<ActionShutdownEditor>, public fb2k::CDarkModeHooks
{
public:
	enum { IDD = IDD_ACTION_SHUTDOWN_CONFIG };

	explicit ActionShutdownEditor(ActionShutdown& action);

private:
	ActionShutdown& m_action;

private:
	BEGIN_MSG_MAP_EX(ActionShutdownEditor)
		MSG_WM_INITDIALOG(OnInitDialog)

		COMMAND_ID_HANDLER_EX(IDOK,     OnCloseCmd)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);
	void OnCloseCmd(UINT uNotifyCode, int nID, CWindow wndCtl);

	CComboBox m_typeCombo;
};