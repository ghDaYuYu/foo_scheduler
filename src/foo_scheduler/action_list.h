#pragma once

#include "resource.h"
#include "action.h"
#include "popup_tooltip_message.h"
#include "action_list_s11n_block.h"

class PrefPageModel;

//------------------------------------------------------------------------------
// ActionList
//------------------------------------------------------------------------------

class ActionList : boost::noncopyable
{
public:
	typedef boost::ptr_vector<IAction> ActionsContainer;

	ActionList();

	ActionList* Clone() const;

	// Duplicate generates new guid,
	// Clone is just a copy used for boost::ptr_vector
	std::unique_ptr<ActionList> Duplicate(const std::wstring &newName) const;

	const GUID& GetGUID() const;
	std::wstring GetName() const;
	std::wstring GetDescription() const;
	bool GetRestartAfterCompletion() const;

	bool ShowConfigDialog(CWindow parent, PrefPageModel* pPrefPageModel);

	std::vector<IAction*> GetActions();

	void AddAction(std::unique_ptr<IAction> pAction);
	ActionsContainer::auto_type RemoveAction(IAction* pAction);

	bool CanMoveActionUp(const IAction* pAction) const;
	bool CanMoveActionDown(const IAction* pAction) const;
	void MoveActionUp(const IAction* pAction);
	void MoveActionDown(const IAction* pAction);

	void LoadFromS11nBlock(const ActionListS11nBlock& block);
	void SaveToS11nBlock(ActionListS11nBlock& block) const;

private:
	ActionList(const ActionList& rhs);
	void CreateGUID();
	friend class ActionListEditor;
	void SetName(const std::wstring& name);
	void SetRestartAfterCompletion(bool restart);

	void MoveAction(const IAction* pAction, bool up);

private:
	GUID m_actionListGUID;
	std::wstring m_name;
	bool m_restartAfterCompletion = false;

	ActionsContainer m_actions;
};

// For boost::ptr_vector.
inline ActionList* new_clone(const ActionList& a)
{
	return a.Clone();
}

//------------------------------------------------------------------------------
// ActionListEditor
//------------------------------------------------------------------------------

class ActionListEditor : public CDialogImpl<ActionListEditor>
{
public:
	enum { IDD = IDD_ACTION_LIST_CONFIG };

	ActionListEditor(ActionList* pActionList, PrefPageModel* pPrefPageModel);

private:
	BEGIN_MSG_MAP(ActionListEditor)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDOK, OnClose)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnClose)
	END_MSG_MAP()

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);
	void OnClose(UINT uNotifyCode, int nID, CWindow wndCtl);

private:
	bool CheckActionListName(const std::wstring& actionListName) const;

private:
	ActionList* m_pActionList;
	PrefPageModel* m_pPrefPageModel;

	CEdit m_actionListName;

	PopupTooltipMessage m_popupTooltipMsg;
	fb2k::CDarkModeHooks m_dark;
};