#pragma once

#include "resource.h"
#include "action.h"
#include "popup_tooltip_message.h"
#include "duration_units.h"
#include "timers_manager.h"

//------------------------------------------------------------------------------
// ActionDelay
//------------------------------------------------------------------------------

class ActionDelay : public IAction
{
public:
	class ExecSession : public IActionExecSession,
		public boost::enable_shared_from_this<ExecSession>
	{
	public:
		explicit ExecSession(const ActionDelay& action);
		~ExecSession();

		virtual void Init(IActionListExecSessionFuncs& alesFuncs);
		virtual void Run(const AsyncCall::CallbackPtr& completionCall);
		virtual const IAction* GetParentAction() const;
		virtual bool GetCurrentStateDescription(std::wstring& descr) const;

	private:
		void OnTimer();

	private:
		const ActionDelay& m_action;
		TimersManager::TimerID m_timerID;
		AsyncCall::CallbackPtr m_completionCall;
		int m_secondsLeft;
        IActionListExecSessionFuncs *m_alesFuncs = nullptr;
	};

	ActionDelay();

	int GetDelay() const;
	void SetDelay(int delay);

	DurationUnits::Type GetDelayUnits() const;
	void SetDelayUnits(DurationUnits::Type delayUnits);

	static const int s_maxDelay = 24 * 60 * 60 * 365;

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
	int m_delay;
	DurationUnits::Type m_delayUnits;
};

//------------------------------------------------------------------------------
// ActionDelayEditor
//------------------------------------------------------------------------------

class ActionDelayEditor : public CDialogImpl<ActionDelayEditor>, public fb2k::CDarkModeHooks
{
public:
	enum { IDD = IDD_ACTION_DELAY_CONFIG };

	explicit ActionDelayEditor(ActionDelay& action);

private:
	ActionDelay& m_action;

private:
	BEGIN_MSG_MAP_EX(ActionDelayEditor)
		MSG_WM_INITDIALOG(OnInitDialog)

		COMMAND_ID_HANDLER_EX(IDOK,     OnCloseCmd)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);
	void OnCloseCmd(UINT uNotifyCode, int nID, CWindow wndCtl);

	CComboBox m_delayUnitsCombo;

	PopupTooltipMessage m_popupTooltipMsg;
};