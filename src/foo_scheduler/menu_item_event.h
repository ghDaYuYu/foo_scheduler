#pragma once

#include "resource.h"
#include "event.h"
#include "popup_tooltip_message.h"
#include "pref_page_model.h"
#include "menu_item_event_s11n_block.h"

class MenuItemEvent : public Event
{
public: // Event
    GUID GetPrototypeGUID() const override;
    int GetPriority() const override;
    std::wstring GetName() const override;
    std::wstring GetDescription() const override;
    bool ShowConfigDialog(CWindow parent, PrefPageModel* pPrefPageModel) override;
    void OnSignal() override;
    std::unique_ptr<Event> Clone() const override;
    std::unique_ptr<Event> CreateFromPrototype() const override;
    void LoadFromS11nBlock(const EventS11nBlock& block) override;
    void SaveToS11nBlock(EventS11nBlock& block) const override;
    void ApplyVisitor(IEventVisitor& visitor) override;

public:
	MenuItemEvent();

	enum EFinalAction
	{
		finalActionReenable,
		finalActionDisable,
		finalActionRemove
	};

	EFinalAction GetFinalAction() const;
	std::wstring GetMenuItemName() const;
	GUID GetGUID() const;
    std::unique_ptr<MenuItemEvent> Duplicate(const std::wstring &newMenuItemName) const;
	
private:
	friend class MenuItemEventEditor;
	void SetFinalAction(EFinalAction finalAction);
	void SetMenuItemName(const std::wstring& menuItemName);
  void GenerateMenuItemGUID();

private:
	MenuItemEvent(const MenuItemEvent& rhs);

	std::wstring m_menuItemName;
	EFinalAction m_finalAction;
	GUID m_menuItemGUID;
};

class MenuItemEventEditor : public CDialogImpl<MenuItemEventEditor>
{
public:
	enum { IDD = IDD_MENU_ITEM_EVENT_CONFIG };

	MenuItemEventEditor(MenuItemEvent* pEvent, PrefPageModel* pPrefPageModel);

private:
	BEGIN_MSG_MAP(MenuItemEventEditor)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDOK, OnClose)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnClose)
	END_MSG_MAP()

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);
	void OnClose(UINT uNotifyCode, int nID, CWindow wndCtl);

private:
	bool CheckEventName(const std::wstring& eventName) const;

private:
	MenuItemEvent* m_pEvent;
	PrefPageModel* m_pPrefPageModel;

	CEdit m_menuItemName;
	CComboBox m_finalAction;

	PopupTooltipMessage m_popupTooltipMsg;
	fb2k::CDarkModeHooks m_dark;
};