#include "pch.h"
#include <chrono>
#include <ctime>    
#include "date_time_event.h"
#include "pref_page_model.h"
#include "combo_helpers.h"
#include "service_manager.h"

//------------------------------------------------------------------------------
// DateTimeEvent
//------------------------------------------------------------------------------

DateTimeEvent::DateTimeEvent() :
	m_type(typeOnce), m_weekDays(0),
	m_date(boost::gregorian::day_clock::local_day()),
	m_wakeup(false), m_finalAction(finalActionRemove)
{

}

DateTimeEvent::DateTimeEvent(const DateTimeEvent& rhs) : Event(rhs),
	m_type(rhs.m_type), m_weekDays(rhs.m_weekDays), m_date(rhs.m_date),
	m_wakeup(rhs.m_wakeup), m_time(rhs.m_time), m_title(rhs.m_title),
	m_finalAction(rhs.m_finalAction)
{
}

std::wstring DateTimeEvent::GetName() const
{
	return L"date/time event";
}

std::wstring DateTimeEvent::GetDescription() const
{
	std::wstring result;

	std::wstring title = boost::trim_copy(m_title);
	std::wstring eventDescr;

	switch (m_type)
	{
	case typeDaily:
		eventDescr = GetDailyDescription();
		break;

	case typeWeekly:
		eventDescr = GetWeeklyDescription();
		break;

	case typeOnce:
		eventDescr = GetOnceDescription();
		break;
	}

	AddWakeUp(eventDescr);

	if (!title.empty())
		result += title + L" [" + eventDescr + L"]";
	else
		result += eventDescr;

	return result;
}

bool DateTimeEvent::ShowConfigDialog(CWindow parent, PrefPageModel* pPrefPageModel)
{
	DateTimeEventEditor dlg(this, pPrefPageModel);
	return dlg.DoModal(parent) == IDOK;
}

void DateTimeEvent::OnSignal()
{
	if (m_type != typeOnce)
		return;

	bool bupdate_preppage = false;
	if (ServiceManager::Instance().GetEventListWindow() &&
		::IsWindow(ServiceManager::Instance().GetEventListWindow()->m_hWnd)) {
		bupdate_preppage = true;
	}

	if (m_finalAction == finalActionRemove) {
		if (bupdate_preppage) {
			ServiceManager::Instance().GetEventListWindow()->DeleteEvent(this);
		}
		ServiceManager::Instance().GetModel().RemoveEvent(this);
	}
	else
	{
		if (bupdate_preppage) {
			ServiceManager::Instance().GetEventListWindow()->DisableEvent(this);
		}
		Enable(false);
		ServiceManager::Instance().GetModel().UpdateEvent(this);
	}
}

std::unique_ptr<Event> DateTimeEvent::Clone() const
{
    return std::unique_ptr<Event>(new DateTimeEvent(*this));
}

std::wstring DateTimeEvent::GetTitle() const
{
	return m_title;
}

void DateTimeEvent::SetTitle(const std::wstring& title)
{
	m_title = title;
}

DateTimeEvent::EType DateTimeEvent::GetType() const
{
	return m_type;
}

void DateTimeEvent::SetType(EType val)
{
	m_type = val;
}

boost::gregorian::date DateTimeEvent::GetDate() const
{
	return m_date;
}

void DateTimeEvent::SetDate(const boost::gregorian::date& val)
{
	m_date = val;
}

char DateTimeEvent::GetWeekDays() const
{
	return m_weekDays;
}

void DateTimeEvent::SetWeekDays(char val)
{
	m_weekDays = val;
}

boost::posix_time::time_duration DateTimeEvent::GetTime() const
{
	return m_time;
}

void DateTimeEvent::SetTime(const boost::posix_time::time_duration& val)
{
	m_time = val;
}

bool DateTimeEvent::GetWakeup() const
{
	return m_wakeup;
}

void DateTimeEvent::SetWakeup(bool val)
{
	m_wakeup = val;
}

std::unique_ptr<DateTimeEvent> DateTimeEvent::Duplicate(const std::wstring &newTitle) const
{
    std::unique_ptr<DateTimeEvent> result(new DateTimeEvent(*this));
	result->NewEventGUID();
    result->SetTitle(newTitle);
    return result;
}

std::wstring DateTimeEvent::GetDailyDescription() const
{
	return L"Daily at " + GetTimeDescription();
}

std::wstring DateTimeEvent::GetWeeklyDescription() const
{
	static std::wstring days[] = { L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat", L"Sun" };

	std::wstring strResult = L"Weekly on ";

	for (int i = 0; i < 7; ++i)
		if (m_weekDays & (0x01 << i))
			strResult += days[i] + L", ";

	boost::trim_right_if(strResult, boost::is_any_of(L", "));

	strResult += L" at " + GetTimeDescription();

	return strResult;
}

std::wstring DateTimeEvent::GetOnceDescription() const
{
	boost::wformat fmt(L"Once on %1% at %2%, %3%");

	fmt % GetDateDescription() % GetTimeDescription();

	switch (m_finalAction)
	{
	case finalActionRemove:
		fmt % L"remove after done";
		break;

	case finalActionDisable:
		fmt % L"disable after done";
		break;
	}

	return fmt.str();
}

std::wstring DateTimeEvent::GetTimeDescription() const
{
	SYSTEMTIME st;
	GetLocalTime(&st);

	st.wHour   = static_cast<WORD>(m_time.hours());
	st.wMinute = static_cast<WORD>(m_time.minutes());
	st.wSecond = static_cast<WORD>(m_time.seconds());

	WCHAR chBuf[64];
	GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, chBuf, 64);

	// Do not return std::wstring(chBuf), tchBuf is in stack, but the optimizer
	// will not pay attention to this moment
	std::wstring strResult(chBuf); 
	return strResult;
}

std::wstring DateTimeEvent::GetDateDescription() const
{
	SYSTEMTIME st;
	GetLocalTime(&st);

	st.wDay   = m_date.day();
	st.wMonth = m_date.month();
	st.wYear  = m_date.year();

	WCHAR chBuf[64];
	GetDateFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, chBuf, 64);

	// Do not return  std::wstring(chBuf), tchBuf is in stack, but the optimizer
	// will not pay attention to this moment
	std::wstring strResult(chBuf);
	return strResult;
}

void DateTimeEvent::AddWakeUp(std::wstring& strResult) const
{
	if (m_wakeup)
		strResult += L", wake up from hibernate/standby";
}

void DateTimeEvent::LoadFromS11nBlock(const EventS11nBlock& block)
{
	if (!block.dateTimeEvent.Exists())
		return;

	const DateTimeEventS11nBlock& b = block.dateTimeEvent.GetValue();

	if (b.title.Exists())
		m_title = pfc::stringcvt::string_wide_from_utf8(b.title.GetValue()).get_ptr();

	if (b.type.Exists())
		m_type = static_cast<EType>(b.type.GetValue());

	if (b.date.Exists())
	{
		std::wstring date = pfc::stringcvt::string_wide_from_utf8(b.date.GetValue()).get_ptr();
		std::wstringstream ssDate(date);
		ssDate >> m_date;
	}
	
	if (b.finalAction.Exists())
		m_finalAction = static_cast<EFinalAction>(b.finalAction.GetValue());

	b.weekDays.GetValueIfExists(m_weekDays);

	if (b.time.Exists())
	{
		std::wstring time = pfc::stringcvt::string_wide_from_utf8(b.time.GetValue()).get_ptr();
		std::wstringstream ssTime(time);
		ssTime >> m_time;
	}

	b.wakeup.GetValueIfExists(m_wakeup);
}

void DateTimeEvent::SaveToS11nBlock(EventS11nBlock& block) const
{
	DateTimeEventS11nBlock b;

	b.title.SetValue(pfc::stringcvt::string_utf8_from_wide(m_title.c_str()).toString());
	b.type.SetValue(m_type);

	if (m_type == typeOnce)
	{
		std::wstringstream ssDate;
		ssDate << m_date;
		b.date.SetValue(pfc::stringcvt::string_utf8_from_wide(ssDate.str().c_str()).toString());
		b.finalAction.SetValue(m_finalAction);
	}

	if (m_type == typeWeekly)
		b.weekDays.SetValue(m_weekDays);

	std::wstringstream ssTime;
	ssTime << m_time;
	b.time.SetValue(pfc::stringcvt::string_utf8_from_wide(ssTime.str().c_str()).toString());

	b.wakeup.SetValue(m_wakeup);

	block.dateTimeEvent.SetValue(b);
}

void DateTimeEvent::ApplyVisitor(IEventVisitor& visitor)
{
    visitor.Visit(*this);
}

GUID DateTimeEvent::GetPrototypeGUID() const
{
	// {c46b3826-0bbf-4598-9593-53190bf7078c} 
	static const GUID result =
	{ 0xc46b3826, 0x0bbf, 0x4598, { 0x95, 0x93, 0x53, 0x19, 0x0b, 0xf7, 0x07, 0x8c } };

	return result;
}

int DateTimeEvent::GetPriority() const
{
	return 0;
}

DateTimeEvent::EFinalAction DateTimeEvent::GetFinalAction() const
{
	return m_finalAction;
}

void DateTimeEvent::SetFinalAction(EFinalAction val)
{
	m_finalAction = val;
}

std::unique_ptr<Event> DateTimeEvent::CreateFromPrototype() const
{
	std::unique_ptr<Event>pClone(new DateTimeEvent(*this));
	pClone->NewEventGUID();
	return pClone;
}

namespace
{
	const bool registered = ServiceManager::Instance().GetEventPrototypesManager().RegisterPrototype(
		new DateTimeEvent);
}

//------------------------------------------------------------------------------
// DateTimeEventEditor
//------------------------------------------------------------------------------

DateTimeEventEditor::DateTimeEventEditor(DateTimeEvent* pEvent, PrefPageModel* pPrefPageModel) :
	m_pEvent(pEvent), m_pPrefPageModel(pPrefPageModel)
{

}

BOOL DateTimeEventEditor::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	m_date = GetDlgItem(IDC_DATE_PICKER);
	m_time = GetDlgItem(IDC_TIME_PICKER);

	m_dark.AddDialogWithControls(m_hWnd);

	SetDlgItemText(IDC_EDIT_TITLE, m_pEvent->GetTitle().c_str());

	m_typeCombo = GetDlgItem(IDC_COMBO_DAY);
	m_finalActionCombo = GetDlgItem(IDC_COMBO_FINAL_ACTION);

	ComboHelpers::InitCombo(m_typeCombo,
		boost::assign::list_of<std::pair<std::wstring, int> >
		(L"once", DateTimeEvent::typeOnce)
		(L"daily", DateTimeEvent::typeDaily)
		(L"weekly", DateTimeEvent::typeWeekly),
		m_pEvent->GetType()
	);

	ComboHelpers::InitCombo(m_finalActionCombo,
		boost::assign::list_of<std::pair<std::wstring, int> >
		(L"Remove after done", DateTimeEvent::finalActionRemove)
		(L"Disable after done", DateTimeEvent::finalActionDisable),
		m_pEvent->GetType() == DateTimeEvent::typeOnce ?
			m_pEvent->GetFinalAction() :
			DateTimeEvent::finalActionRemove
	);

	switch (m_pEvent->GetType())
	{
	case DateTimeEvent::typeOnce:
		InitDate();
		break;

	case DateTimeEvent::typeWeekly:
		InitWeekDays();
		break;
	}

	InitTime();
	CheckDlgButton(IDC_CHECK_WAKEUP, m_pEvent->GetWakeup());

	CreateWeekDaysControl();

	UpdateDayControls();

	CenterWindow(GetParent());

	return TRUE;
}

void DateTimeEventEditor::InitWeekDays()
{
	for (int day = 0; day < 7; ++day)
		m_weekDays.SetCheckState(day, m_pEvent->GetWeekDays() & (0x01 << day));
}

void DateTimeEventEditor::InitDate()
{
	SYSTEMTIME st;
	GetLocalTime(&st);

	boost::gregorian::date d = m_pEvent->GetDate();

	st.wDay   = d.day();
	st.wMonth = d.month();
	st.wYear  = d.year();

	m_date.SetSystemTime(GDT_VALID, &st);
}

void DateTimeEventEditor::InitTime()
{
	SYSTEMTIME st;
	GetLocalTime(&st);

	boost::posix_time::time_duration t = m_pEvent->GetTime();

	st.wHour   = static_cast<WORD>(t.hours());
	st.wMinute = static_cast<WORD>(t.minutes());
	st.wSecond = static_cast<WORD>(t.seconds());

	m_time.SetSystemTime(GDT_VALID, &st);
}

char DateTimeEventEditor::GetWeekDays() const
{
	char days = 0;

	for (int d = 0; d < 7; ++d)
		days |= (m_weekDays.GetCheckState(d) << d);

	return days;
}

boost::gregorian::date DateTimeEventEditor::GetDate() const
{
	SYSTEMTIME st;
	m_date.GetSystemTime(&st);

	return boost::gregorian::date(st.wYear, st.wMonth, st.wDay);
}

boost::posix_time::time_duration DateTimeEventEditor::GetTime() const
{
	SYSTEMTIME st;
	m_time.GetSystemTime(&st);

	return boost::posix_time::time_duration(st.wHour, st.wMinute, st.wSecond);
}

void DateTimeEventEditor::OnClose(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (nID == IDOK)
	{
		DateTimeEvent::EType type = ComboHelpers::GetSelectedItem<DateTimeEvent::EType>(m_typeCombo);

		if (type == DateTimeEvent::typeOnce && OnceDateTimeEventExpired())
			return;
		else if (type == DateTimeEvent::typeWeekly && GetWeekDays() == 0)
		{
			m_popupTooltipMsg.Show(L"Specify at least one day.", m_weekDays);
			return;
		}

		CString s;
		GetDlgItemText(IDC_EDIT_TITLE, s);

		m_pEvent->SetTitle(s.GetString());
		m_pEvent->SetWakeup(IsDlgButtonChecked(IDC_CHECK_WAKEUP) == TRUE);
		m_pEvent->SetTime(GetTime());
		m_pEvent->SetType(type);

		switch (type)
		{
		case DateTimeEvent::typeOnce:
			m_pEvent->SetDate(GetDate());
			m_pEvent->SetFinalAction(
				ComboHelpers::GetSelectedItem<DateTimeEvent::EFinalAction>(m_finalActionCombo));
			break;

		case DateTimeEvent::typeWeekly:
			m_pEvent->SetWeekDays(GetWeekDays());
			break;
		}
	}

	EndDialog(nID);
}

void DateTimeEventEditor::UpdateDayControls()
{
	DateTimeEvent::EType currentType = ComboHelpers::GetSelectedItem<DateTimeEvent::EType>(m_typeCombo);

	m_date.ShowWindow(currentType == DateTimeEvent::typeOnce);
	m_weekDays.ShowWindow(currentType == DateTimeEvent::typeWeekly);
	m_finalActionCombo.ShowWindow(currentType == DateTimeEvent::typeOnce);
}

void DateTimeEventEditor::OnDayTypeSelChange(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	m_popupTooltipMsg.CleanUp();
	UpdateDayControls();
}

LRESULT DateTimeEventEditor::OnNotify(UINT ctrl, LPNMHDR lpNmhdr) {

	if (!m_dark.IsDark() || lpNmhdr->idFrom != IDC_DATE_PICKER) {
		return FALSE;
	}

	if (lpNmhdr->code == (WPARAM)DTN_DROPDOWN) {
		auto hwndMothCal = DateTime_GetMonthCal(m_date);
		SetWindowTheme(hwndMothCal, L"", L"");
		//todo: save/reset current field
	}
	else if (lpNmhdr->code == (WPARAM)DTN_CLOSEUP) {
		//todo: restore current field
	}
	return FALSE;
}

LRESULT DateTimeEventEditor::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	context_menu_show((HWND)wParam, lParam);
	return FALSE;
}

bool DateTimeEventEditor::context_menu_show(HWND wnd, LPARAM lParamPos)
{
	SYSTEMTIME st;
	boost::posix_time::time_duration t = m_pEvent->GetTime();
	st.wHour = static_cast<WORD>(t.hours());
	st.wMinute = static_cast<WORD>(t.minutes());
	st.wSecond = static_cast<WORD>(t.seconds());

	const bool isTimePicker = uGetDlgItem(IDC_TIME_PICKER) == wnd;
	bool bvk_apps = lParamPos == -1;

	POINT point;

	if (bvk_apps) {
		CRect rect;
		CWindow(wnd).GetWindowRect(&rect);
		point = rect.CenterPoint();
	}
	else {
		point.x = GET_X_LPARAM(lParamPos);
		point.y = GET_Y_LPARAM(lParamPos);
	}

	try {

		HMENU menu = CreatePopupMenu();

		uAppendMenu(menu, MF_STRING, ID_TP_POPUP_N1M, "Now +1'");
		uAppendMenu(menu, MF_STRING, ID_TP_POPUP_N2M, "Now +2'");
		uAppendMenu(menu, MF_STRING, ID_TP_POPUP_N3M, "Now +3'");
		uAppendMenu(menu, MF_SEPARATOR, 0, "");
		uAppendMenu(menu, MF_STRING, ID_TP_POPUP_N15M, "Now +15'");
		uAppendMenu(menu, MF_STRING, ID_TP_POPUP_N30M, "Now +30'");
		uAppendMenu(menu, MF_STRING, ID_TP_POPUP_N45M, "Now +45'");
		uAppendMenu(menu, MF_STRING, ID_TP_POPUP_N60M, "Now +60'");
		uAppendMenu(menu, MF_SEPARATOR, 0, "");
		uAppendMenu(menu, MF_STRING, ID_TP_POPUP_RSECS, "&Round seconds");

		int cmd = TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, point.x, point.y, 0, wnd, 0);
		DestroyMenu(menu);

		context_menu_switch(wnd, point, cmd);
	}
	catch (...) {}
	return false;
}

bool DateTimeEventEditor::context_menu_switch(HWND wnd, POINT point, int cmd) {

	SYSTEMTIME st;
	
	size_t minute_offset = 0;
	bool reset_seconds = false;

	switch (cmd)
	{
	case ID_TP_POPUP_N1M: {
		minute_offset += 1;
		break;
	}
	case ID_TP_POPUP_N2M: {
		minute_offset += 2;
		break;
	}
	case ID_TP_POPUP_N3M: {
		minute_offset += 3;
		break;
	}
	case ID_TP_POPUP_N15M: {
		minute_offset += 15;
		break;
	}
	case ID_TP_POPUP_N30M: {
		minute_offset += 30;
		break;
	}
	case ID_TP_POPUP_N45M: {
		minute_offset += 45;
		break;
	}
	case ID_TP_POPUP_N60M: {
		minute_offset += 60;
		break;
	}
	case ID_TP_POPUP_RSECS: {
		reset_seconds = true;
		break;
	}
	default: {
			return false;
	}
	} //end switch

	std::chrono::system_clock::time_point start;

	if (reset_seconds) {

		m_time.GetSystemTime(&st);

		if (st.wSecond >= 30) {
			minute_offset = 1;
		}

		struct tm tm = {0};
		tm.tm_year = st.wYear - 1900;
		tm.tm_mon = st.wMonth;
		tm.tm_mday = st.wDay;
		tm.tm_hour = st.wHour;
		tm.tm_min = st.wMinute;
		tm.tm_sec = 0;
		time_t tt = mktime(&tm);

		start = std::chrono::system_clock::from_time_t(tt);
	}
	else {
		GetLocalTime(&st);
		start = std::chrono::system_clock::now();	
	}

	const std::chrono::seconds sec{ minute_offset * 60 };
	time_t mynow = std::chrono::system_clock::to_time_t(start + sec);

	std::tm bt{};
	localtime_s(&bt, &mynow);

	st.wHour = bt.tm_hour;
	st.wMinute = bt.tm_min;
	st.wSecond = bt.tm_sec;
	st.wMilliseconds = reset_seconds ? 0 : st.wMilliseconds;

	m_time.SetSystemTime(GDT_VALID, &st);

	return false;
}

void DateTimeEventEditor::CreateWeekDaysControl()
{
	m_weekDays.SubclassWindow(GetDlgItem(IDC_LIST_WEEK_DAYS));

	DWORD dwStyle = LVS_EX_LABELTIP | LVS_EX_DOUBLEBUFFER;
	m_weekDays.SetExtendedListViewStyle(dwStyle, dwStyle);

	CRect rect;
	m_weekDays.GetClientRect(rect);

	m_weekDays.AddColumn(L"", 0);
	m_weekDays.SetColumnWidth(0, rect.Width() - ::GetSystemMetrics(SM_CXVSCROLL));
	m_weekDays.AddItem(0, 0, _T("Mon"));
	m_weekDays.AddItem(1, 0, _T("Tue"));
	m_weekDays.AddItem(2, 0, _T("Wed"));
	m_weekDays.AddItem(3, 0, _T("Thu"));
	m_weekDays.AddItem(4, 0, _T("Fri"));
	m_weekDays.AddItem(5, 0, _T("Sat"));
	m_weekDays.AddItem(6, 0, _T("Sun"));
}

bool DateTimeEventEditor::OnceDateTimeEventExpired() const
{
	boost::gregorian::date curDate = boost::gregorian::day_clock::local_day();
	boost::gregorian::date newDate = GetDate();
	
	if (newDate < curDate)
	{
		m_popupTooltipMsg.Show(L"Date expired.", m_date);
		return true;
	}

	boost::posix_time::ptime newTime(newDate, GetTime());

	if (newTime <= boost::posix_time::ptime(boost::posix_time::second_clock::local_time()))
	{
		m_popupTooltipMsg.Show(L"Time expired.", m_time);
		return true;
	}

	return false;
}
