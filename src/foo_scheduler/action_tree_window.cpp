#include "pch.h"
#include "action_tree_window.h"
#include "service_manager.h"
#include "action_list.h"
#include "pref_page_model.h"
#include "generate_duplicate_name.h"

void ActionTreeWindow::Init(HWND hwndParent, UINT ctrlID, PrefPageModel* pModel)
{
	m_hWnd = ::uGetDlgItem(hwndParent, ctrlID);
	m_pModel = pModel;

	std::vector<ActionList*> actionLists = m_pModel->GetActionLists();
	std::for_each(actionLists.begin(), actionLists.end(),
		boost::bind(&ActionTreeWindow::AddActionList, this, _1, TVI_LAST));

	m_pModel->ConnectActionListAddedSlot(boost::bind(&ActionTreeWindow::OnActionListAdded, this, _1));
	m_pModel->ConnectActionListRemovedSlot(boost::bind(&ActionTreeWindow::OnActionListRemoved, this, _1));
	m_pModel->ConnectActionListUpdatedSlot(boost::bind(&ActionTreeWindow::OnActionListUpdated, this, _1));

	m_pModel->ConnectActionAddedSlot(boost::bind(&ActionTreeWindow::OnActionAdded, this, _1, _2));
	m_pModel->ConnectActionUpdatedSlot(boost::bind(&ActionTreeWindow::OnActionUpdated, this, _1, _2));
	m_pModel->ConnectActionRemovedSlot(boost::bind(&ActionTreeWindow::OnActionRemoved, this, _1, _2));

	m_pModel->ConnectModelResetSlot(boost::bind(&ActionTreeWindow::OnModelReset, this));
}

void ActionTreeWindow::AddActionList(ActionList* pActionList, HTREEITEM hInsertAfter)
{

	CTreeItem ti = InsertItem(pActionList->GetDescription().c_str(), TVI_ROOT, hInsertAfter);
	ti.SetData(reinterpret_cast<DWORD_PTR>(new ItemData(itemTypeActionList, pActionList)));

	std::vector<IAction*> actions = pActionList->GetActions();
	std::for_each(actions.begin(), actions.end(),
		boost::bind(&ActionTreeWindow::AddAction, this, ti, _1));
}

void ActionTreeWindow::OnActionListAdded(ActionList* pActionList)
{
	AddActionList(pActionList, TVI_LAST);
}

LRESULT ActionTreeWindow::OnDeleteItem(LPNMHDR pnmh)
{
	LPNMTREEVIEW lpnmtv = reinterpret_cast<LPNMTREEVIEW>(pnmh);

	delete reinterpret_cast<ItemData*>(GetItemData(lpnmtv->itemOld.hItem));

	return 0;
}

void ActionTreeWindow::AddAction(CTreeItem tiActionList, IAction* pAction)
{
	CTreeItem tiNewAction = tiActionList.AddTail(pAction->GetDescription().c_str(), 0);
	tiNewAction.SetData(reinterpret_cast<DWORD_PTR>(new ItemData(itemTypeAction, pAction)));
}

void ActionTreeWindow::ShowItemContextMenu(CTreeItem ti, const CPoint& point)
{
	ItemData* pItemData = reinterpret_cast<ItemData*>(ti.GetData());
	CPoint    pnt = point;

	ClientToScreen(&pnt);

	if (pItemData->itemType == itemTypeActionList)
		ShowActionListContextMenu(ti, pnt);
	else if (pItemData->itemType == itemTypeAction)
		ShowActionContextMenu(ti, pnt);
}

void ActionTreeWindow::AppendActionListItems(CMenu& menuPopup)
{
	menuPopup.AppendMenu(MF_SEPARATOR);

	menuPopup.AppendMenu(MF_STRING | MF_BYCOMMAND,
		static_cast<UINT_PTR>(actionListMenuItemEdit), _T("Edit..."));

    menuPopup.AppendMenu(MF_STRING | MF_BYCOMMAND,
        static_cast<UINT_PTR>(actionListMenuItemDuplicate), _T("Duplicate"));

	menuPopup.AppendMenu(MF_STRING | MF_BYCOMMAND,
		static_cast<UINT_PTR>(actionListMenuItemRemove), _T("Remove"));
}

void ActionTreeWindow::ShowActionListContextMenu(CTreeItem ti, const CPoint& pntScreen)
{
	ItemData* pItemData = reinterpret_cast<ItemData*>(ti.GetData());
	ActionList* pActionList = static_cast<ActionList*>(pItemData->pObj);

	std::vector<IAction*> actionPrototypes =
		ServiceManager::Instance().GetActionPrototypesManager().GetPrototypes();

	CMenu menuPopup;
	menuPopup.CreatePopupMenu();

	AppendActionsPopupMenu(menuPopup, actionPrototypes);
	AppendActionListItems(menuPopup);

	UINT uCmdID = menuPopup.TrackPopupMenu(
		TPM_LEFTALIGN | TPM_NONOTIFY | TPM_RETURNCMD,
		pntScreen.x, pntScreen.y, GetActiveWindow());

	switch (uCmdID)
	{
	case 0:
		return;

	case actionListMenuItemEdit:
		if (pActionList->ShowConfigDialog(*this, m_pModel))
			m_pModel->UpdateActionList(pActionList);
		break;

    case actionListMenuItemDuplicate:
        {
            const std::wstring newName = GenerateDuplicateName(
                pActionList->GetName(), m_pModel->GetActionLists(), [](ActionList *al) { return al->GetName(); });
            m_pModel->AddActionList(pActionList->Duplicate(newName));
        }
        break;

    case actionListMenuItemRemove:
		m_pModel->RemoveActionList(pActionList);
		break;

	default:
		{
			std::auto_ptr<IAction> pNewAction(actionPrototypes[uCmdID - 1]->Clone());

			if (pNewAction->HasConfigDialog() && !pNewAction->ShowConfigDialog(*this))
				return;

			m_pModel->AddActionToActionList(pActionList, pNewAction);
		}
		break;
	}
}

void ActionTreeWindow::ShowActionContextMenu(CTreeItem ti, const CPoint& pntScreen)
{
	ItemData* pItemData = reinterpret_cast<ItemData*>(ti.GetData());
	IAction* pAction = static_cast<IAction*>(pItemData->pObj);

	ActionList* pActionList = GetParentActionList(ti);

	CMenu menuPopup;
	menuPopup.CreatePopupMenu();

	menuPopup.AppendMenu(MF_STRING | MF_BYCOMMAND |
		(pAction->HasConfigDialog() ? 0 : MF_DISABLED | MF_GRAYED),
		static_cast<UINT_PTR>(actionMenuItemEdit), _T("Edit..."));
	
	menuPopup.AppendMenu(MF_STRING | MF_BYCOMMAND,
		static_cast<UINT_PTR>(actionMenuItemRemove), _T("Remove"));
	
	menuPopup.AppendMenu(MF_SEPARATOR);
	menuPopup.AppendMenu(MF_STRING | MF_BYCOMMAND |
		(pActionList->CanMoveActionUp(pAction) ? 0 : MF_DISABLED | MF_GRAYED),
		static_cast<UINT_PTR>(actionMenuItemMoveUp), _T("Move up"));
	
	menuPopup.AppendMenu(MF_STRING | MF_BYCOMMAND |
		(pActionList->CanMoveActionDown(pAction) ? 0 : MF_DISABLED | MF_GRAYED),
		static_cast<UINT_PTR>(actionMenuItemMoveDown), _T("Move down"));

	UINT uCmdID = menuPopup.TrackPopupMenu(
		TPM_LEFTALIGN | TPM_NONOTIFY | TPM_RETURNCMD,
		pntScreen.x, pntScreen.y, GetActiveWindow());

	switch (uCmdID)
	{
	case 0:
		return;

	case actionMenuItemEdit:
		if (pAction->ShowConfigDialog(*this))
			m_pModel->UpdateAction(pActionList, pAction);
		break;

	case actionMenuItemRemove:
		m_pModel->RemoveAction(pActionList, pAction);
		break;

	case actionMenuItemMoveUp:
		pActionList->MoveActionUp(pAction);
		SwapTreeItems(ti.GetPrevSibling(), ti);
		ti = ti.GetPrevSibling();
		ti.Select();

		// Separate call should be added but for now it's ok...
		m_pModel->UpdateActionList(pActionList);
		break;

	case actionMenuItemMoveDown:
		pActionList->MoveActionDown(pAction);
		SwapTreeItems(ti.GetNextSibling(), ti);
		ti = ti.GetNextSibling();
		ti.Select();

		// Separate call should be added but for now it's ok...
		m_pModel->UpdateActionList(pActionList);
		break;
	}
}

void ActionTreeWindow::OnActionListRemoved(ActionList* pActionList)
{
	CTreeItem ti = FindActionListItem(pActionList);

	if (ti.IsNull())
		return;

	ti.Delete();
}

CTreeItem ActionTreeWindow::FindActionListItem(ActionList* pActionList)
{
	CTreeItem ti = GetRootItem();

	while (!ti.IsNull())
	{
		ItemData* pItemData = reinterpret_cast<ItemData*>(ti.GetData());

		if (pActionList == static_cast<ActionList*>(pItemData->pObj))
			return ti;

		ti = ti.GetNextSibling();
	}

	return CTreeItem();
}

void ActionTreeWindow::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	UINT nHitFlags;
	CTreeItem tiHit = HitTest(point, &nHitFlags);

	SetMsgHandled(false);

	if (tiHit.IsNull() || !(nHitFlags & TVHT_ONITEMLABEL))
		return;

	ItemData* pItemData = reinterpret_cast<ItemData*>(tiHit.GetData());

	if (pItemData->itemType != itemTypeAction)
		return;

	SetFocus();
	tiHit.Select();

	IAction* pAction = static_cast<IAction*>(pItemData->pObj);
	if (pAction->HasConfigDialog() && pAction->ShowConfigDialog(*this))
		m_pModel->UpdateAction(GetParentActionList(tiHit), pAction);

	SetMsgHandled(true);
}

void ActionTreeWindow::OnActionListUpdated(ActionList* pActionList)
{
	CTreeItem ti = FindActionListItem(pActionList);
	_ASSERTE(!ti.IsNull());

	ti.SetText(pActionList->GetDescription().c_str());
}

void ActionTreeWindow::AppendActionsPopupMenu(CMenu& menuPopup,
	const std::vector<IAction*>& actionPrototypes)
{
	CMenu menuActions;
	menuActions.CreatePopupMenu();

	for (std::size_t i = 0; i < actionPrototypes.size(); ++i)
	{
		menuActions.AppendMenu(MF_STRING | MF_BYCOMMAND, static_cast<UINT_PTR>(i + 1),
			actionPrototypes[i]->GetName().c_str());
	}

	menuPopup.AppendMenu(MF_POPUP, menuActions.Detach(), _T("Add action"));
}

void ActionTreeWindow::SwapTreeItems(CTreeItem ti1, CTreeItem ti2)
{
	CString str1, str2;

	ti1.GetText(str1);
	ti2.GetText(str2);

	ti1.SetText(str2);
	ti2.SetText(str1);

	DWORD_PTR pData1 = ti1.GetData();
	DWORD_PTR pData2 = ti2.GetData();

	ti1.SetData(pData2);
	ti2.SetData(pData1);
}

void ActionTreeWindow::OnActionAdded(ActionList* pActionList, IAction* pAction)
{
	CTreeItem ti = FindActionListItem(pActionList);
	_ASSERTE(!ti.IsNull());

	AddAction(ti, pAction);

	ti.Expand();
}

void ActionTreeWindow::OnActionUpdated(ActionList* pActionList, IAction* pAction)
{
	CTreeItem ti = FindActionItem(pActionList, pAction);
	_ASSERTE(!ti.IsNull());

	ti.SetText(pAction->GetDescription().c_str());
}

ActionList* ActionTreeWindow::GetParentActionList(const CTreeItem& tiAction)
{
	CTreeItem tiParent = tiAction.GetParent();
	_ASSERTE(!tiParent.IsNull());

	ItemData* pItemData = reinterpret_cast<ItemData*>(tiParent.GetData());
	return static_cast<ActionList*>(pItemData->pObj);
}

CTreeItem ActionTreeWindow::FindActionItem(ActionList* pActionList, IAction* pAction)
{
	CTreeItem ti = FindActionListItem(pActionList);
	_ASSERTE(!ti.IsNull());

	ti = ti.GetChild();
	_ASSERTE(!ti.IsNull());

	while (!ti.IsNull())
	{
		ItemData* pItemData = reinterpret_cast<ItemData*>(ti.GetData());

		if (pAction == static_cast<IAction*>(pItemData->pObj))
			return ti;

		ti = ti.GetNextSibling();
	}

	return CTreeItem();
}

void ActionTreeWindow::OnActionRemoved(ActionList* pActionList, IAction* pAction)
{
	CTreeItem ti = FindActionItem(pActionList, pAction);
	_ASSERTE(!ti.IsNull());

	ti.Delete();
}

void ActionTreeWindow::OnModelReset()
{
	DeleteAllItems();
}

void ActionTreeWindow::OnRButtonDown(UINT nFlags, CPoint point)
{
	UINT nHitFlags;
	CTreeItem tiHit = HitTest(point, &nHitFlags);

	if (nHitFlags & (TVHT_ONITEMSTATEICON | TVHT_ONITEMBUTTON | TVHT_ONITEMINDENT))
		return;

	SetFocus();

	if (tiHit.IsNull() || !(nHitFlags & TVHT_ONITEMLABEL))
		return;

	tiHit.Select();
	ShowItemContextMenu(tiHit, point);
}

void ActionTreeWindow::OnContextMenu(CWindow wnd, CPoint point)
{
	CTreeItem ti;

	if (point.x < 0 && point.y < 0)
	{
		ti = GetSelectedItem();

		if (ti.IsNull())
			return;

		CRect itemRect;
		GetItemRect(ti, itemRect, TRUE);

		point.x = itemRect.left;
		point.y = itemRect.bottom;

		ShowItemContextMenu(ti, point);
	}
	else
	{
		ScreenToClient(&point);
		OnRButtonDown(0, point);
	}
}
