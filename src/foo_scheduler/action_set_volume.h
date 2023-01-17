#pragma once

#include "resource.h"
#include "action.h"
#include "duration_units.h"
#include "popup_tooltip_message.h"
#include "timers_manager.h"

//------------------------------------------------------------------------------
// ActionSetVolume
//------------------------------------------------------------------------------

class ActionSetVolume : public IAction
{
public:
	class ExecSession : public IActionExecSession,
		public boost::enable_shared_from_this<ExecSession>
	{
	public:
		explicit ExecSession(const ActionSetVolume& action);
		~ExecSession();

		virtual void Init(IActionListExecSessionFuncs& alesFuncs);
		virtual void Run(const AsyncCall::CallbackPtr& completionCall);
		virtual const IAction* GetParentAction() const;
		virtual bool GetCurrentStateDescription(std::wstring& descr) const;

	private:
		void RunWithFade();
		void RunWithoutFade();
		void OnTimer();

	private:
		const ActionSetVolume& m_action;
		float m_powerStep;
		float m_power;
		int m_secondsLeft;
		TimersManager::TimerID m_timerID;
		AsyncCall::CallbackPtr m_completionCall;
        IActionListExecSessionFuncs* m_alesFuncs = nullptr;
	};

	static const int s_maxFadeDuration = 24 * 60 * 60 * 365;

	ActionSetVolume();

	float GetVolume() const;
	void SetVolume(float val);

	bool GetUseFade() const;
	void SetUseFade(bool val);

	int GetFadeDuration() const;
	void SetFadeDuration(int val);

	DurationUnits::Type GetFadeDurationUnits() const;
	void SetFadeDurationUnits(DurationUnits::Type val);

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
	float m_volume;
	bool m_useFade;
	int m_fadeDuration;
	DurationUnits::Type m_fadeDurationUnits;
};

//------------------------------------------------------------------------------
// ActionSetVolumeEditor
//------------------------------------------------------------------------------

class ActionSetVolumeEditor : public CDialogImpl<ActionSetVolumeEditor>, public fb2k::CDarkModeHooks
{
public:
	enum { IDD = IDD_ACTION_SET_VOLUME_CONFIG };

	explicit ActionSetVolumeEditor(ActionSetVolume& action);

private:
	ActionSetVolume& m_action;

private:
	BEGIN_MSG_MAP_EX(ActionDelayEditor)
		MSG_WM_INITDIALOG(OnInitDialog)

		COMMAND_ID_HANDLER_EX(IDC_BUTTON_PICK, OnPick)
		COMMAND_ID_HANDLER_EX(IDC_CHECK_USE_FADE, OnCheckUseFade)

		COMMAND_ID_HANDLER_EX(IDOK,     OnCloseCmd)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);
	void OnCloseCmd(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnPick(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnCheckUseFade(UINT uNotifyCode, int nID, CWindow wndCtl);

	void EnableFadeDurationControls(bool enable);
	void SetVolumeLabel();

private:
	CComboBox m_durationUnitsCombo;
	CButton m_checkUseFade;

	float m_volume;

	PopupTooltipMessage m_popupTooltipMsg;
};