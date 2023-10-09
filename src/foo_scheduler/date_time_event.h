#pragma once

#include "resource.h"
#include "event.h"
#include "popup_tooltip_message.h"
#include "date_time_event_s11n_block.h"

class DateTimeEvent : public Event
{
public:
	DateTimeEvent();

	enum EType
	{
		typeOnce, typeDaily, typeWeekly
	};

	enum EFinalAction
	{
		finalActionRemove,
		finalActionDisable
	};

	enum EDay
	{
		dayMon = 0x01, dayTue = 0x02, dayWed = 0x04, dayThu = 0x08,
		dayFri = 0x10, daySat = 0x20, daySun = 0x40
	};

	std::wstring GetTitle() const;
	void SetTitle(const std::wstring& title);

	EType GetType() const;
	void SetType(EType val);

	boost::gregorian::date GetDate() const;
	void SetDate(const boost::gregorian::date& val);

	EFinalAction GetFinalAction() const;
	void SetFinalAction(EFinalAction val);

	char GetWeekDays() const;
	void SetWeekDays(char val);

	boost::posix_time::time_duration GetTime() const;
	void SetTime(const boost::posix_time::time_duration& val);

	bool GetWakeup() const;
	void SetWakeup(bool val);

	std::unique_ptr<DateTimeEvent> Duplicate(const std::wstring &newTitle) const;

public: // Event
	GUID GetPrototypeGUID() const override;
	int GetPriority() const override;
	std::wstring GetName() const override;
	std::wstring GetDescription() const override;
	bool ShowConfigDialog(CWindow parent, class PrefPageModel* pPrefPageModel) override;
	void OnSignal() override;
	std::unique_ptr<Event> Clone() const override;
	std::unique_ptr<Event> CreateFromPrototype() const override;
	void LoadFromS11nBlock(const EventS11nBlock& block) override;
	void SaveToS11nBlock(EventS11nBlock& block) const override;
	void ApplyVisitor(IEventVisitor& visitor) override;

private:
	DateTimeEvent(const DateTimeEvent& rhs);

	std::wstring GetTimeDescription() const;
	std::wstring GetDateDescription() const;

	std::wstring GetDailyDescription() const;
	std::wstring GetWeeklyDescription() const;
	std::wstring GetOnceDescription() const;
	void AddWakeUp(std::wstring& strResult) const;

private:
	std::wstring m_title;
	EType m_type;
	boost::gregorian::date m_date;
	EFinalAction m_finalAction;
	char m_weekDays;
	boost::posix_time::time_duration m_time;
	bool m_wakeup;
};

class DateTimeEventEditor : public CDialogImpl<DateTimeEventEditor>
{
public:
	enum { IDD = IDD_DATETIME_EVENT_CONFIG };

	enum {
		ID_TP_POPUP_N1M = 1,
		ID_TP_POPUP_N2M,
		ID_TP_POPUP_N3M,
		ID_TP_POPUP_N15M,
		ID_TP_POPUP_N30M,
		ID_TP_POPUP_N45M,
		ID_TP_POPUP_N60M,
		ID_TP_POPUP_RSECS,
	};

	DateTimeEventEditor(DateTimeEvent* pEvent, PrefPageModel* pPrefPageModel);

private:
	BEGIN_MSG_MAP(DateTimeEventEditor)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_HANDLER_EX(IDC_COMBO_DAY, CBN_SELCHANGE, OnDayTypeSelChange)
		MSG_WM_NOTIFY(OnNotify)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
		COMMAND_ID_HANDLER_EX(IDOK, OnClose)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnClose)
	END_MSG_MAP()

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);
	void OnClose(UINT uNotifyCode, int nID, CWindow wndCtl);

	void OnDayTypeSelChange(UINT uNotifyCode, int nID, CWindow wndCtl);
	LRESULT OnNotify(UINT /*ctrl*/, LPNMHDR /*lParam*/);
	LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	bool context_menu_show(HWND wnd, LPARAM lParamPos);
	bool context_menu_switch(HWND wnd, POINT point, int cmd);

private:
	void InitTime();
	void InitDate();
	void InitWeekDays();

	void CreateWeekDaysControl();
	void UpdateDayControls();

	char GetWeekDays() const;
	boost::gregorian::date GetDate() const;
	boost::posix_time::time_duration GetTime() const;

	bool OnceDateTimeEventExpired() const;

private:
	DateTimeEvent* m_pEvent;
	PrefPageModel* m_pPrefPageModel;

	CComboBox m_typeCombo;
	CComboBox m_finalActionCombo;
	CCheckListViewCtrl m_weekDays;
	CDateTimePickerCtrl m_date;
	CDateTimePickerCtrl m_time;
	mutable PopupTooltipMessage m_popupTooltipMsg;
	fb2k::CDarkModeHooks m_dark;
};