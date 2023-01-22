#include "pch.h"
#include "resource.h"
#include "menu_item_event.h"
#include "service_manager.h"
#include "combo_helpers.h"

//------------------------------------------------------------------------------
// MenuItemEvent
//------------------------------------------------------------------------------

std::wstring MenuItemEvent::GetName() const
{
	return L"menu item event";
}

std::wstring MenuItemEvent::GetDescription() const
{
	std::wstring result = L"On menu item \"";
	result += m_menuItemName;
	result += L"\"";

	switch (m_finalAction)
	{
	case finalActionRemove:
		result += L" [remove event]";
		break;

	case finalActionDisable:
		result += L" [disable event]";
		break;

	case finalActionReenable:
		result += L" [re-enable event]";
		break;
	}

	return result;
}

MenuItemEvent::EFinalAction MenuItemEvent::GetFinalAction() const
{
	return m_finalAction;
}

void MenuItemEvent::SetFinalAction(EFinalAction finalAction)
{
	m_finalAction = finalAction;
}

std::wstring MenuItemEvent::GetMenuItemName() const
{
	return m_menuItemName;
}

void MenuItemEvent::SetMenuItemName(const std::wstring& menuItemName)
{
	m_menuItemName = menuItemName;
}

void MenuItemEvent::GenerateMenuItemGUID()
{
    UuidCreate(&m_menuItemGUID);
}

MenuItemEvent::MenuItemEvent() : m_finalAction(finalActionReenable)
{
    GenerateMenuItemGUID();
}

MenuItemEvent::MenuItemEvent(const MenuItemEvent& rhs) :
	Event(rhs),
	m_menuItemName(rhs.m_menuItemName),
	m_finalAction(rhs.m_finalAction),
	m_menuItemGUID(rhs.m_menuItemGUID)
{

}
bool MenuItemEvent::ShowConfigDialog(CWindow parent, PrefPageModel* pPrefPageModel)
{
	MenuItemEventEditor dlg(this, pPrefPageModel);
	return dlg.DoModal(parent) == IDOK;
}

GUID MenuItemEvent::GetGUID() const
{
	return m_menuItemGUID;
}

std::unique_ptr<MenuItemEvent> MenuItemEvent::Duplicate(const std::wstring &newMenuItemName) const
{
    std::unique_ptr<MenuItemEvent> result(new MenuItemEvent(*this));
	result->NewEventGUID(); //event instance
    result->SetMenuItemName(newMenuItemName);
    result->GenerateMenuItemGUID(); //ui
    return result;
}

void MenuItemEvent::OnSignal()
{
	switch (m_finalAction)
	{
	case finalActionRemove:
		ServiceManager::Instance().GetModel().RemoveEvent(this);
		return;

	case finalActionDisable:
		Enable(false);
		ServiceManager::Instance().GetModel().UpdateEvent(this);
		break;

	case finalActionReenable:
		break;
	}
}

std::unique_ptr<Event> MenuItemEvent::Clone() const
{
    return std::unique_ptr<Event>(new MenuItemEvent(*this));
}

void MenuItemEvent::LoadFromS11nBlock(const EventS11nBlock& block)
{
	if (!block.menuItemEvent.Exists())
		return;

	const MenuItemEventS11nBlock& b = block.menuItemEvent.GetValue();

	if (b.menuItemName.Exists())
		m_menuItemName = pfc::stringcvt::string_wide_from_utf8(b.menuItemName.GetValue()).get_ptr();

	if (b.finalAction.Exists())
		m_finalAction = static_cast<EFinalAction>(b.finalAction.GetValue());

	b.guid.GetValueIfExists(m_menuItemGUID);
}

void MenuItemEvent::SaveToS11nBlock(EventS11nBlock& block) const
{
	MenuItemEventS11nBlock b;

	b.menuItemName.SetValue(pfc::stringcvt::string_utf8_from_wide(m_menuItemName.c_str()).toString());
	b.finalAction.SetValue(m_finalAction);
	b.guid.SetValue(m_menuItemGUID);

	block.menuItemEvent.SetValue(b);
}

void MenuItemEvent::ApplyVisitor(IEventVisitor& visitor)
{
    visitor.Visit(*this);
}

GUID MenuItemEvent::GetPrototypeGUID() const
{
	// {b368eead-70bf-426b-b597-4d92c685dcb0} 
	static const GUID result = 
	{ 0xb368eead, 0x70bf, 0x426b, { 0xb5, 0x97, 0x4d, 0x92, 0xc6, 0x85, 0xdc, 0xb0 } };

	return result;
}

int MenuItemEvent::GetPriority() const
{
	return 20;
}

std::unique_ptr<Event> MenuItemEvent::CreateFromPrototype() const
{
    std::unique_ptr<MenuItemEvent> pClone(new MenuItemEvent(*this));
	pClone->NewEventGUID();
	// It's important to change uuid, otherwise there may be problems with keyboard shortcuts.
    pClone->GenerateMenuItemGUID();

	return pClone;
}

namespace
{
	const bool registered = ServiceManager::Instance().GetEventPrototypesManager().RegisterPrototype(
		new MenuItemEvent);
}

//------------------------------------------------------------------------------
// MenuItemEventEditor
//------------------------------------------------------------------------------

MenuItemEventEditor::MenuItemEventEditor(MenuItemEvent* pEvent, PrefPageModel* pPrefPageModel) :
	m_pEvent(pEvent), m_pPrefPageModel(pPrefPageModel)
{
}

BOOL MenuItemEventEditor::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	m_menuItemName = GetDlgItem(IDC_EDIT_MENU_ITEMS_NAME);
	m_finalAction = GetDlgItem(IDC_COMBO_FINAL_ACTION);

	m_menuItemName.SetWindowText(m_pEvent->GetMenuItemName().c_str());
	m_menuItemName.SetSel(0, -1);

	ComboHelpers::InitCombo(m_finalAction,
		boost::assign::list_of<std::pair<std::wstring, int> >
		(L"Re-enable event", MenuItemEvent::finalActionReenable)
		(L"Disable event", MenuItemEvent::finalActionDisable)
		(L"Remove event", MenuItemEvent::finalActionRemove),
		m_pEvent->GetFinalAction()
	);

	CenterWindow(GetParent());

	// dark mode
	AddDialog(m_hWnd);
	AddControls(m_hWnd);

	return FALSE;
}

void MenuItemEventEditor::OnClose(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (nID == IDOK)
	{
		CString text;
		m_menuItemName.GetWindowText(text);

		text.Trim();

		if (text.IsEmpty())
		{
			m_popupTooltipMsg.Show(L"Enter menu item's name.", m_menuItemName);
			return;
		}

		if (!CheckEventName(static_cast<LPCWSTR>(text)))
		{
			m_popupTooltipMsg.Show(L"Menu item with this name already exists.", m_menuItemName);
			return;
		}

		m_pEvent->SetMenuItemName(static_cast<LPCWSTR>(text));
		m_pEvent->SetFinalAction(ComboHelpers::GetSelectedItem<MenuItemEvent::EFinalAction>(m_finalAction));
	}

	EndDialog(nID);
}

bool MenuItemEventEditor::CheckEventName(const std::wstring& eventName) const
{
	std::vector<Event*> events = m_pPrefPageModel->GetEvents();

	for (std::size_t i = 0; i < events.size(); ++i)
	{
		if (MenuItemEvent* pEvent = dynamic_cast<MenuItemEvent*>(events[i]))
		{
			if ((pEvent->GetMenuItemName() == eventName) && (m_pEvent != pEvent))
				return false;
		}
	}

	return true;
}

