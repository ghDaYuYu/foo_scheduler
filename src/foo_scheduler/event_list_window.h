#pragma once

#include "event.h"

#include <libPPUI/CListControlComplete.h>
#include <libPPUI/CListControl-Cells.h>

class PrefPageModel;

struct data_t {
	pfc::string8 eventdesc;
	pfc::string8 actionname;
	DWORD_PTR p;
	bool enabled;
};

class EventListWindow : public CListControlComplete,
	public boost::signals2::trackable
{
public:

	~EventListWindow();

	void Init(PrefPageModel* pModel);

	virtual groupID_t GetItemGroup(t_size p_item) const override {
		(void)p_item; return -1;
	}

	// list overrides

	t_size GetItemCount() const override {
		return m_vdata.size();
	}

	bool CanSelectItem(size_t row) const override {
		// can not select footer
		return row != footerRow();
	}

	size_t footerRow() const {
		return m_vdata.size();
	}
	void onFooterClicked() {
		//..
	}

	void OnSubItemClicked(t_size item, t_size subItem, CPoint pt) override;

	bool AllowScrollbar(bool vertical) const override {
		return true;
	}

	t_size InsertIndexFromPointEx(const CPoint& pt, bool& bInside) const override {
		// Drag&drop insertion point hook, for reordering only
		auto ret = __super::InsertIndexFromPointEx(pt, bInside);
		bInside = false; // never drop *into* an item, only between, as we only allow reorder
		if (ret > m_vdata.size()) ret = m_vdata.size(); // never allow drop beyond footer
		return ret;
	}

	void RequestReorder(size_t const* order, size_t count) override;

	void RemoveMask(pfc::bit_array const& mask);

	void RequestRemoveSelection() override {
		// Delete key etc
		RemoveMask(GetSelectionMask());
	}

	// default action

	void ExecuteDefaultAction(t_size index) override {
		// double click, enter key, etc
		if (index == footerRow()) onFooterClicked();
	}

	// set text

	bool GetSubItemText(t_size item, t_size subItem, pfc::string_base& out) const override {
		if (item == footerRow()) {
			if (subItem == 0) {
				out = "+ add new";
				return true;
			}
			return false;
		}
		auto& rec = m_vdata[item];
		switch (subItem) {
		case 0:
			// pass blank string or return false to create a checkbox only column
			// out = "check";
			return false;
		case 1:
			out = rec.eventdesc.c_str();
			return true;
		case 2:
			out = rec.actionname.c_str();
			return true;
		default:
			return false;
		}
	}

	size_t GetSubItemSpan(size_t row, size_t column) const override {
		if (row == footerRow() && column == 0) {
			return GetColumnCount();
		}
		return 1;
	}
	cellType_t GetCellType(size_t item, size_t subItem) const override {
		// cellType_t is a pointer to a cell class object supplying cell behavior specification & rendering methods
		// use PFC_SINGLETON to provide static instances of used cells
		if (item == footerRow()) {
			if (subItem == 0) {
				return &PFC_SINGLETON(CListCell_Button);
			}
			else {
				return nullptr;
			}
		}
		switch (subItem) {
		case 0:
			return &PFC_SINGLETON(CListCell_Checkbox);
		default:
			return &PFC_SINGLETON(CListCell_Text);
		}
	}
	bool GetCellTypeSupported() const override {
		return true;
	}
	bool GetCellCheckState(size_t item, size_t subItem) const override {
		if (subItem == 0) {
			auto& rec = m_vdata[item];
			return rec.enabled;
		}
		return false;
	}
	void SetCellCheckState(size_t item, size_t subItem, bool value) override;

	uint32_t QueryDragDropTypes() const override { return dragDrop_reorder; }

	// Inplace edit handlers
	// Overrides of CTableEditHelperV2 methods
	void TableEdit_SetField(t_size item, t_size subItem, const char* value) override;
	void TableEdit_GetField(t_size item, t_size subItem, pfc::string_base& out, t_size& lineCount) override;
	bool TableEdit_IsColumnEditable(t_size subItem) const override {
		return subItem == 0 || subItem == 1;
	}

private:
	void OnNewEventAdded(Event* pNewEvent);
	void OnEventUpdated(Event* pEvent);
	void OnEventRemoved(Event* pEvent);

	void OnModelReset();

	void InitColumns();

	// Returns index of the item that has been added.
	size_t AddNewEvent(Event* pNewEvent);
	void InsertEventAtPos(size_t pos, const Event* pEvent);

	size_t FindItemByEvent(const Event* pEvent);
	//void MoveEventItem(const Event* pEvent, bool up);
	void EditEvent(Event* pEvent);

	void ShowEventContextMenu(size_t item, const CPoint& point);

	void AppendActionListsItems(CMenu& menuPopup, bool replace, Event* pEvent);
	void AppendEventItems(CMenu& menuPopup, const Event* pEvent);

private:

	typedef CListControlComplete TParent;

	BEGIN_MSG_MAP(EventListWindow)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_CONTEXTMENU(OnContextMenu)
		CHAIN_MSG_MAP(TParent)
	END_MSG_MAP()

	int OnCreate(LPCREATESTRUCT lpCreateStruct) {
		InitColumns(); // set up header control with columns
		SetWindowText(L"Event list window"); // screen reader will see this
		return 0;
	}

	void OnContextMenu(CWindow wnd, CPoint point);
	void ItemAction(size_t item);

private:
	enum EPopupMenuItems
	{
		menuItemEdit = 1000,
        menuItemDuplicate,
		menuItemRemove,
		menuItemMoveUp,
		menuItemMoveDown
	};

	PrefPageModel* m_pModel;
	std::vector<data_t> m_vdata;
};

