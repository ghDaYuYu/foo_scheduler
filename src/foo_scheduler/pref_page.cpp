#include "pch.h"
#include "version.h"
#include "pref_page.h"
#include "service_manager.h"

PreferencesPage::PreferencesPage(preferences_page_callback::ptr callback) :
	m_callback(callback), m_changed(false), m_pModel(nullptr)
{
	m_pModel.reset(new PrefPageModel(ServiceManager::Instance().GetModel().GetState()));

	m_pModel->ConnectModelChangedSlot(boost::bind(&PreferencesPage::OnChanged, this));
	m_pModel->ConnectModelResetSlot(boost::bind(&PreferencesPage::OnChanged, this));
}

PreferencesPage::~PreferencesPage() {
	ServiceManager::Instance().SetEventListWindow(nullptr);

	m_eventList.Detach();
	m_actionTree.Detach();
	DeleteObject(m_font);

	m_staticStatusDateTimeEventsHeader.Detach();
	m_staticActiveSessionsHeader.Detach();
	m_staticStatusHeader.Detach();
}

t_uint32 PreferencesPage::get_state()
{
	t_uint32 state = preferences_state::resettable;
	
	if (m_changed)
		state |= preferences_state::changed;

	return state | preferences_state::dark_mode_supported;
}

void PreferencesPage::apply()
{
	// Our dialog content has not changed but the flags have - our currently shown
	// values now match the settings so the apply button can be disabled.
	ServiceManager::Instance().GetModel().SetState(m_pModel->GetState());

	m_changed = false;
	m_callback->on_state_changed();
}

void PreferencesPage::reset()
{
	m_pModel->Reset();
}

BOOL PreferencesPage::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{

	DlgResize_Init(false, true);

	HWND wndStaticHeader = uGetDlgItem(IDC_EVENTS_LIST_HEADER);
	m_staticStatusDateTimeEventsHeader.SubclassWindow(wndStaticHeader);
	m_staticStatusDateTimeEventsHeader.PaintGradientHeader();
	wndStaticHeader = uGetDlgItem(IDC_STATIC_ACTION_LIST_HEADER);
	m_staticActiveSessionsHeader.SubclassWindow(wndStaticHeader);
	m_staticActiveSessionsHeader.PaintGradientHeader();
	wndStaticHeader = uGetDlgItem(IDC_STATIC_STATUS_HEADER);
	m_staticStatusHeader.SubclassWindow(wndStaticHeader);
	m_staticStatusHeader.PaintGradientHeader();

	m_eventList.CreateInDialog(*this, IDC_EVENT_LIST);
	m_eventList.Init(m_pModel.get());
	ServiceManager::Instance().SetEventListWindow(&m_eventList);

	m_actionTree.Init(m_hWnd, IDC_ACTION_LIST_TREE, m_pModel.get());

	//dark mode
	m_dark.AddDialog(m_hWnd);
	m_dark.AddControls(m_hWnd);

	CheckDlgButton(IDC_ENABLED_CHECK, m_pModel->IsSchedulerEnabled());

	return TRUE;
}

void PreferencesPage::SetThemeFont() {
	LOGFONTW lf;
	CWindowDC dc(core_api::get_main_window());
	CTheme wtheme;
	HTHEME theme = wtheme.OpenThemeData(core_api::get_main_window(), L"TEXTSTYLE");
	GetThemeFont(theme, dc, TEXT_BODYTEXT, 0, TMT_FONT, &lf);
	m_font = CreateFontIndirectW(&lf);
	SetFont(m_font, true);

	for (HWND walk = ::GetWindow(m_hWnd, GW_CHILD); walk != NULL; ) {
		HWND next = ::GetWindow(walk, GW_HWNDNEXT);
		if (::IsWindow(next)) {
			::SendMessage(next, WM_SETFONT, (WPARAM)m_font, MAKELPARAM(1/*bRedraw*/, 0));
		}
		walk = next;
	}
}

void PreferencesPage::OnBtnAddEvent(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	CMenu menuPopup;
	menuPopup.CreatePopupMenu();

	std::vector<Event*> eventPrototypes = ServiceManager::Instance().GetEventPrototypesManager().GetPrototypes();

	for (std::size_t i = 0; i < eventPrototypes.size(); ++i)
	{
		std::wstring item = L"Add " + eventPrototypes[i]->GetName() + L"...";
		menuPopup.AppendMenu(MF_STRING | MF_BYCOMMAND, static_cast<UINT_PTR>(i + 1), item.c_str());
	}

	CRect rcBtnAddEvent;
	GetDlgItem(IDC_BTN_ADD_EVENT).GetWindowRect(rcBtnAddEvent);

	UINT uCmdID = menuPopup.TrackPopupMenu(
		TPM_LEFTALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_RIGHTALIGN,
		rcBtnAddEvent.BottomRight().x - 1, rcBtnAddEvent.BottomRight().y - 1,
		*this);

	if (uCmdID == 0)
		return;

	auto pNewEvent(eventPrototypes[uCmdID - 1]->CreateFromPrototype());

	if (!pNewEvent->ShowConfigDialog(*this, m_pModel.get()))
		return;

	m_pModel->AddEvent(std::move(pNewEvent));
}

void PreferencesPage::OnBtnAddActionList(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	std::unique_ptr<ActionList> pActionList(new ActionList);

	if (!pActionList->ShowConfigDialog(*this, m_pModel.get()))
		return;

	m_pModel->AddActionList(std::move(pActionList));
}

void PreferencesPage::OnChanged()
{
	CheckDlgButton(IDC_ENABLED_CHECK, m_pModel->IsSchedulerEnabled());

	m_changed = true;

	// Tell the host that our state has changed to enable/disable the apply button appropriately.
	m_callback->on_state_changed();
}

void PreferencesPage::OnBtnShowStatusWindow(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
{
	ServiceManager::Instance().GetRootController().ShowStatusWindow();
}

void PreferencesPage::OnEnableScheduler(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	m_pModel->SetSchedulerEnabled(IsDlgButtonChecked(IDC_ENABLED_CHECK) == 1);
}

//------------------------------------------------------------------------------
// PreferencesPageImpl
//------------------------------------------------------------------------------

// {E6CC8BDA-590C-472A-8375-8C8F1A5CDF74} mod guid
const GUID PreferencesPageImpl::m_guid =
	{ 0xe6cc8bda, 0x590c, 0x472a, { 0x83, 0x75, 0x8c, 0x8f, 0x1a, 0x5c, 0xdf, 0x74 } };

const char* PreferencesPageImpl::get_name()
{
	return PLUGIN_NAME;
}

GUID PreferencesPageImpl::get_guid()
{
	return m_guid;
}

GUID PreferencesPageImpl::get_parent_guid()
{
	return preferences_page::guid_root;
}