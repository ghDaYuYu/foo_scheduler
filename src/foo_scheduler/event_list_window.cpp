#include "pch.h"
#include "event_list_window.h"
#include "service_manager.h"
#include "action.h"
#include "action_list.h"
#include "pref_page_model.h"
#include "generate_duplicate_name.h"

namespace
{

class EventDuplicateVisitor : public IEventVisitor
{
public:
    explicit EventDuplicateVisitor(const std::vector<Event*>& events)
        : m_modelEvents(events)
    {
    }

    std::unique_ptr<Event> TakeEvent()
    {
        return std::move(m_event);
    }

private: // IEventVisitor
    void Visit(PlayerEvent& event) override
    {
        m_event = event.Clone();
    }

    void Visit(DateTimeEvent& event) override
    {
        const std::wstring newTitle = GenerateDuplicateName(event.GetTitle(), m_modelEvents, [] (Event* e) {
            if (const DateTimeEvent* dateTimeEvent = dynamic_cast<const DateTimeEvent*>(e))
                return dateTimeEvent->GetTitle();
            return std::wstring();
        });

        m_event = event.Duplicate(newTitle);
    }

    void Visit(MenuItemEvent& event) override
    {
        const std::wstring newMenuItemName =
            GenerateDuplicateName(event.GetMenuItemName(), m_modelEvents, [](Event *e) {
                if (const MenuItemEvent *menuItemEvent = dynamic_cast<const MenuItemEvent *>(e))
                    return menuItemEvent->GetMenuItemName();
                return std::wstring();
            });

        m_event = event.Duplicate(newMenuItemName);
    }

private:
    std::vector<Event*> m_modelEvents;
    std::unique_ptr<Event> m_event;
};

} // namespace

std::vector<data_t> m_vdata;

void EventListWindow::SaveModelColumns() {

	std::vector<int> cw;
	cw.push_back(static_cast<int>(GetColumnWidthF(1)));
	cw.push_back(static_cast<int>(GetColumnWidthF(2)));

	ServiceManager::Instance().GetModel().SetEventsWindowColumnsWidths(cw);
}

EventListWindow::~EventListWindow()
{
	for (auto& w : m_vdata) {
		w.p = NULL;
	}
	m_vdata.clear();
}

void EventListWindow::Init(PrefPageModel* pModel)
{
	m_pModel = pModel;

	std::vector<Event*> events = m_pModel->GetEvents();
	std::for_each(events.begin(), events.end(), boost::bind(&EventListWindow::AddNewEvent, this, _1));

	m_pModel->ConnectEventAddedSlot(boost::bind(&EventListWindow::OnNewEventAdded, this, _1));
	m_pModel->ConnectEventUpdatedSlot(boost::bind(&EventListWindow::OnEventUpdated, this, _1));
	m_pModel->ConnectEventRemovedSlot(boost::bind(&EventListWindow::OnEventRemoved, this, _1));

	m_pModel->ConnectModelResetSlot(boost::bind(&EventListWindow::OnModelReset, this));
}

void EventListWindow::OnNewEventAdded(Event* pNewEvent)
{
	SelectSingle(AddNewEvent(pNewEvent));
}

size_t EventListWindow::AddNewEvent(Event* pNewEvent)
{
	size_t pos = GetItemCount();
	InsertEventAtPos(pos, pNewEvent);
	return pos;
}

void build_row_data(data_t& out, size_t pos, const Event* pEvent, ActionList* pActionList) {

	std::wstring wstr = pEvent->GetDescription().c_str();
	char buffer[255];
	pfc::stringcvt::convert_wide_to_utf8(buffer, 255, wstr.c_str(), wstr.size());

	out.actionname = "";
	out.eventdesc = buffer;

	if (pActionList != 0) {
		wstr = pActionList->GetName().c_str();
		pfc::stringcvt::convert_wide_to_utf8(buffer, 255, wstr.c_str(), wstr.size());
	}
	else {
		buffer[0] = '\0';
	}
	out.actionname = buffer;
	out.enabled = pEvent->IsEnabled();
	out.p = reinterpret_cast<DWORD_PTR>(pEvent);
}

void EventListWindow::OnEventUpdated(Event* pEvent)
{
	size_t item = FindItemByEventID(pEvent);
	_ASSERTE(item != -1);

	auto & d = m_vdata.at(item);
	ActionList* pActionList = m_pModel->GetActionListByGUID(pEvent->GetActionListGUID());
	build_row_data(d, item, pEvent, pActionList);

	UpdateItemsAll();
}

void EventListWindow::ItemAction(size_t item)
{
	EditEvent(reinterpret_cast<Event*>(m_vdata.at(item).p));
}

size_t EventListWindow::FindItemByEventID(const Event* pEvent)
{

	auto fit = std::find_if(m_vdata.begin(), m_vdata.end(), [pEvent](const data_t item) {
		return reinterpret_cast<const Event*>(item.p)->GetEventGUID() == pEvent->GetEventGUID();
		});

	if (fit != m_vdata.end()) return std::distance(m_vdata.begin(), fit);

	return -1;
}

size_t EventListWindow::FindItemByEvent(const Event* pEvent)
{

	auto fit = std::find_if(m_vdata.begin(), m_vdata.end(), [pEvent](const data_t item) {
		return reinterpret_cast<const Event*>(item.p) == pEvent;
		});

	if (fit != m_vdata.end()) return std::distance(m_vdata.begin(), fit);

	return -1;
}

void EventListWindow::ShowEventContextMenu(pfc::bit_array_bittable selmask, const CPoint& point)
{
	auto first_sel = selmask.find(true, 0, GetItemCount());
	Event* pEvent = first_sel < GetItemCount() ? reinterpret_cast<Event*>(m_vdata.at(first_sel).p) : nullptr;

	CPoint pnt = point;
	ClientToScreen(&pnt);

	CMenu menuPopup;
	menuPopup.CreatePopupMenu();
	if (pEvent) {
		AppendActionListsItems(menuPopup, false, pEvent);
	}
	AppendEventItems(menuPopup, GetSelectedCount() == 1);

	UINT uCmdID = menuPopup.TrackPopupMenu(TPM_LEFTALIGN | TPM_NONOTIFY | TPM_RETURNCMD, pnt.x, pnt.y, GetActiveWindow());

	switch (uCmdID)
	{
	case 0:
		return;

	case menuItemEdit:
		EditEvent(pEvent);
		break;

    case menuItemDuplicate:
		{
			EventDuplicateVisitor visitor(m_pModel->GetEvents());
			pEvent->ApplyVisitor(visitor);
			pEvent->NewEventGUID();
			m_pModel->AddEvent(visitor.TakeEvent());
		}
		break;

	case menuItemRemove:
		RemoveMask(GetSelectionMask());
		break;

	default: // Action list selected.
		{
			std::size_t actionListIndex = uCmdID - 1;
			std::vector<ActionList*> actionLists = m_pModel->GetActionLists();

			size_t w = selmask.find_first(true, 0, GetItemCount());
			while (w < GetItemCount()) {
				Event* pEvent = first_sel < GetItemCount() ? reinterpret_cast<Event*>(m_vdata.at(w).p) : nullptr;
				pEvent->SetActionListGUID(actionListIndex != actionLists.size() ?
					actionLists[actionListIndex]->GetGUID() : pfc::guid_null);
				m_pModel->UpdateEvent(pEvent);
				w = selmask.find_next(true, w, GetItemCount());
			};
		}
		break;
	}
}

void EventListWindow::AppendActionListsItems(CMenu& menuPopup, bool replace, Event* pEvent)
{
	CMenu * menuActionLists;
	if (replace) {
		menuActionLists = &menuPopup;
	}
	else {
		menuActionLists = new CMenu();
		menuActionLists->CreatePopupMenu();
	}

	std::vector<ActionList*> actionLists = m_pModel->GetActionLists();

	for (std::size_t i = 0; i < actionLists.size(); ++i)
	{
		menuActionLists->AppendMenu(MF_STRING | MF_BYCOMMAND, static_cast<UINT_PTR>(i + 1),
			actionLists[i]->GetName().c_str());
	}

	// Create "None" item.
	if (!actionLists.empty())
		menuActionLists->AppendMenu(MF_SEPARATOR);

	menuActionLists->AppendMenu(MF_STRING | MF_BYCOMMAND,
		static_cast<UINT_PTR>(actionLists.size() + 1), L"None");

	// Check item.
	if (GetSelectedCount() == 1) {
		if (pEvent->GetActionListGUID() == pfc::guid_null)
		{
			menuActionLists->CheckMenuRadioItem((UINT)1, (UINT)actionLists.size() + 1,
				(UINT)actionLists.size() + 1, MF_BYCOMMAND);
		}
		else
		{
			for (std::size_t i = 0; i < actionLists.size(); ++i)
				if (pEvent->GetActionListGUID() == actionLists[i]->GetGUID())
				{
					menuActionLists->CheckMenuRadioItem((UINT)1, (UINT)actionLists.size(),
						(UINT)i + 1, MF_BYCOMMAND);
					break;
				}
		}
	}
	if (!replace) {
		menuPopup.AppendMenu(MF_POPUP, menuActionLists->Detach(), L"Assign action list");
		delete menuActionLists;
	}
}

void EventListWindow::AppendEventItems(CMenu& menuPopup, const bool single_sel)
{
	menuPopup.AppendMenu(MF_SEPARATOR);

	menuPopup.AppendMenu(MF_STRING | !single_sel ? MF_DISABLED | MF_GRAYED : 0 | MF_BYCOMMAND,
		static_cast<UINT_PTR>(menuItemEdit), L"Edit...");

    menuPopup.AppendMenu(MF_STRING | !single_sel ? MF_DISABLED | MF_GRAYED : 0 | MF_BYCOMMAND,
        static_cast<UINT_PTR>(menuItemDuplicate), L"Duplicate");

	menuPopup.AppendMenu(MF_STRING | MF_BYCOMMAND,
		static_cast<UINT_PTR>(menuItemRemove), L"Remove");

}

void EventListWindow::EditEvent(Event* pEvent)
{
	if (!pEvent->ShowConfigDialog(*this, m_pModel))
		return;
	
	size_t pos = FindItemByEvent(pEvent);
	ActionList* pActionList = m_pModel->GetActionListByGUID(pEvent->GetActionListGUID());
	data_t& adata = m_vdata.at(pos);
	build_row_data(adata, pos, pEvent, pActionList);
	UpdateItem(pos);

	m_pModel->UpdateEvent(pEvent);
}

void EventListWindow::OnEventRemoved(Event* pEvent)
{
	size_t pos = FindItemByEvent(pEvent);
	_ASSERTE(pos != -1);
	m_vdata.erase(m_vdata.begin() + pos);
	UpdateItemsAll();
}

void EventListWindow::OnModelReset()
{
	m_vdata.clear();
	UpdateItemsAll();
}

void EventListWindow::OnContextMenu(CWindow wnd, CPoint point)
{
	if (GetSelectedCount() == 0)
		return;

	pfc::bit_array_bittable selmask = GetSelectionMask();

	if (point.x < 0 && point.y < 0)
	{
		CRect itemRect = GetItemRect(GetFirstSelected());
		point.x = itemRect.left;
		point.y = itemRect.bottom;
	}
	else
		ScreenToClient(&point);

	ShowEventContextMenu(selmask, point);
}

void EventListWindow::InitColumns()
{
	InitializeHeaderCtrl(HDS_DRAGDROP);

	CRect rcList;
	GetClientRect(rcList);

	auto scw = GetSystemMetrics(SM_CXVSCROLL);

	// Using global model to request columns' widths.
	std::vector<int> cw = ServiceManager::Instance().GetModel().GetEventsWindowColumnsWidths();

	if (cw.empty())
	{
		AddColumnEx("", scw);
		AddColumnEx("Event", (rcList.Width() * 2) / 3);
		AddColumn("Action list", UINT32_MAX);
	}
	else
	{
		_ASSERTE(cw.size() == 2);
		AddColumnEx("", scw);
		AddColumnEx("Event", cw[0]);
		AddColumn("Action list", cw[1]);
	}
}

void EventListWindow::InsertEventAtPos(size_t pos, const Event* pEvent)
{
	ActionList* pActionList = m_pModel->GetActionListByGUID(pEvent->GetActionListGUID());

	data_t item;
	build_row_data(item, pos, pEvent, pActionList);
	m_vdata.emplace_back(item);
}

void EventListWindow::SetCellCheckState(size_t item, size_t subItem, bool value) {
	if (subItem == 0) {
		auto& rec = m_vdata[item];
		if (rec.enabled != value) {
			rec.enabled = value;
			Event* pEvent = reinterpret_cast<Event*>(m_vdata.at(item).p);
			pEvent->Enable(rec.enabled);
			m_pModel->UpdateEvent(pEvent);

			__super::SetCellCheckState(item, subItem, value);
		}
	}
}

void EventListWindow::OnSubItemClicked(t_size item, t_size subItem, CPoint pt) {

	if (item == footerRow()) {
		onFooterClicked(); return;
	}
	else if (subItem == 2) {
		CRect rcButton = GetSubItemRectAbs(item, subItem);
		rcButton.MoveToY(rcButton.top + (int)(1.5 * rcButton.Height()));

		POINT pt;
		pt.x = rcButton.left;
		pt.y = rcButton.bottom;
		::ClientToScreen(m_hWnd, &pt);

		enum { MENU_DEFAULT = 1, MENU_EXPORT, MENU_IMPORT };

		Event* pEvent = reinterpret_cast<Event*>(m_vdata.at(item).p/*GetItemData(item)*/);

		CMenu menuPopup;
		menuPopup.CreatePopupMenu();
		AppendActionListsItems(menuPopup, true, pEvent);

		int uCmdID = TrackPopupMenu(menuPopup, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, pt.x, pt.y, 0, m_hWnd, NULL);
		DestroyMenu(menuPopup);

		if (uCmdID) {
			std::size_t actionListIndex = uCmdID - 1;
			std::vector<ActionList*> actionLists = m_pModel->GetActionLists();

			pEvent->SetActionListGUID(actionListIndex != actionLists.size() ?
				actionLists[actionListIndex]->GetGUID() : pfc::guid_null);
			m_pModel->UpdateEvent(pEvent);

			UpdateItem(item);
		}
	}	else if (TableEdit_IsColumnEditable(subItem)) {
		Event* pEvent = reinterpret_cast<Event*>(m_vdata.at(item).p);
		bool btime_event = pEvent->GetName()._Equal(L"date/time event");

			if (btime_event && subItem == 1) {
				TableEdit_Start(item, subItem); return;
		}
	}
	__super::OnSubItemClicked(item, subItem, pt);
}

// Inplace edit handlers
// Overrides of CTableEditHelperV2 methods
void EventListWindow::TableEdit_SetField(t_size item, t_size subItem, const char* value) {
	if (subItem == 0) {
		m_vdata[item].enabled = value;
		ReloadItem(item);
	}
	else if (subItem == 1) {
		m_vdata[item].eventdesc = value;
		ReloadItem(item);
		Event* pEvent = reinterpret_cast<Event*>(m_vdata.at(item).p);
		ActionList* pActionList = m_pModel->GetActionListByGUID(pEvent->GetActionListGUID());

		if (pEvent->GetName()._Equal(L"date/time event")) {
			TCHAR w[255];
			pfc::string8 str = pfc::string8(value);
			pfc::stringcvt::convert_utf8_to_wide(w, 255, value, str.get_length());
			((DateTimeEvent*)pEvent)->SetTitle(w);
			m_pModel->UpdateEvent(pEvent);
		}

		data_t dt;
		build_row_data(dt, item, pEvent, pActionList);
		m_vdata.at(item) = dt;
		UpdateItem(item);
	}
}

void EventListWindow::TableEdit_GetField(t_size item, t_size subItem, pfc::string_base& out, t_size& lineCount) {
	if (subItem == 1) {
		Event* pEvent = reinterpret_cast<Event*>(m_vdata.at(item).p);	
		std::wstring wstr = ((DateTimeEvent*)pEvent)->GetTitle().c_str();
		char buffer[255];
		pfc::stringcvt::convert_wide_to_utf8(buffer, 255, wstr.c_str(), wstr.size());
		out = buffer;
	}
}

// remove mask

void EventListWindow::RemoveMask(pfc::bit_array const& mask) {
	
	if (mask.get(footerRow())) return;

	auto oldCount = GetItemCount();
	auto w = mask.find_first(true, 0, GetItemCount());
	
	while (w < oldCount) {
		Event* pEvent = reinterpret_cast<Event*>(m_vdata.at(t - (oldCount - m_vdata.size())).p);
		m_pModel->RemoveEvent(pEvent);
		t = mask.find_next(true, w, oldCount);
	}
	this->OnItemsRemoved(mask, oldCount);
}

// request reorder

void EventListWindow::RequestReorder(size_t const* order, size_t count) {
	// we've been asked to reorder the items, by either drag&drop or cursors+modifiers
	// we can either reorder as requested, reorder partially if some of the items aren't moveable, or reject the request

	PFC_ASSERT(count == GetItemCount());

	// Footer row cannot be moved
	if (GetItemCount() > m_vdata.size() && order[footerRow()] != footerRow()) return;

	for (size_t w = 0; w < m_vdata.size(); w++) {
		if (w != order[w]) {
			bool done = false;
			for (size_t wprev = 0; wprev <= w && wprev < m_vdata.size(); wprev++) {
				if (order[wprev] == w) {
					done = true;
					break;
				}
			}
			if (!done) {
				Event* pEvent = reinterpret_cast<Event*>(m_vdata.at(w).p);
				m_pModel->SwapEvent(pEvent, w, order[0]);
			}
		}
	}

	pfc::reorder_t(m_vdata, order, count);

	this->OnItemsReordered(order, count);
}