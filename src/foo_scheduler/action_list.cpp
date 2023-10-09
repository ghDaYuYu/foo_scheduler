#include "pch.h"
#include "action_list.h"
#include "service_manager.h"
#include "pref_page_model.h"

//------------------------------------------------------------------------------
// ActionList
//------------------------------------------------------------------------------

ActionList::ActionList()
{
	CreateGUID();
}

ActionList::ActionList(const ActionList& rhs) :
	m_actionListGUID(rhs.m_actionListGUID),
	m_name(rhs.m_name),
	m_actions(rhs.m_actions),
	m_restartAfterCompletion(rhs.m_restartAfterCompletion)
{
}

void ActionList::CreateGUID()
{
	::UuidCreate(&m_actionListGUID);
}

bool ActionList::ShowConfigDialog(CWindow parent, PrefPageModel* pPrefPageModel)
{
	ActionListEditor dlg(this, pPrefPageModel);
	return dlg.DoModal(parent) == IDOK;
}

std::vector<IAction*> ActionList::GetActions()
{
	std::vector<IAction*> result(m_actions.size());
	std::transform(m_actions.begin(), m_actions.end(), result.begin(), &boost::lambda::_1);
	return result;
}

std::wstring ActionList::GetName() const
{
	return m_name;
}

std::wstring ActionList::GetDescription() const
{
	auto result = GetName();
	if (m_restartAfterCompletion)
		result += L" [restart after completion]";
	return result;
}

bool ActionList::GetRestartAfterCompletion() const
{
	return m_restartAfterCompletion;
}

void ActionList::SetName(const std::wstring& name)
{
	m_name = name;
}

void ActionList::SetRestartAfterCompletion(bool restart)
{
	m_restartAfterCompletion = restart;
}

void ActionList::AddAction(std::unique_ptr<IAction> pAction)
{
	m_actions.push_back(std::move(pAction));
}

ActionList* ActionList::Clone() const
{
	return new ActionList(*this);
}

std::unique_ptr<ActionList> ActionList::Duplicate(const std::wstring &newName) const
{
	std::unique_ptr<ActionList> result(new ActionList(*this));
	result->SetName(newName);
	result->CreateGUID();
	return result;
} 

ActionList::ActionsContainer::auto_type ActionList::RemoveAction(IAction* pAction)
{
	auto it = std::find_if(m_actions.begin(), m_actions.end(), &boost::lambda::_1 == pAction);
	_ASSERTE(it != m_actions.end());

	return m_actions.release(it);
}

bool ActionList::CanMoveActionUp(const IAction* pAction) const
{
	_ASSERTE(!m_actions.empty());
	return pAction != &m_actions.front();
}

bool ActionList::CanMoveActionDown(const IAction* pAction) const
{
	_ASSERTE(!m_actions.empty());
	return pAction != &m_actions.back();
}

void ActionList::MoveActionUp(const IAction* pAction)
{
	_ASSERTE(CanMoveActionUp(pAction));
	MoveAction(pAction, true);
}

void ActionList::MoveActionDown(const IAction* pAction)
{
	_ASSERTE(CanMoveActionDown(pAction));
	MoveAction(pAction, false);
}

void ActionList::MoveAction(const IAction* pAction, bool up)
{
	auto it = std::find_if(m_actions.begin(), m_actions.end(), &boost::lambda::_1 == pAction);
	_ASSERTE(it != m_actions.end());

	std::size_t pos = std::distance(m_actions.begin(), it) + (up ? -1 : 1);
	ActionsContainer::auto_type pA = m_actions.release(it);
	m_actions.insert(m_actions.begin() + pos, pA.release());
}

const GUID& ActionList::GetGUID() const
{
	return m_actionListGUID;
}

void ActionList::LoadFromS11nBlock(const ActionListS11nBlock& block)
{
	block.guid.GetValueIfExists(m_actionListGUID);

	if (block.name.Exists())
		m_name = pfc::stringcvt::string_wide_from_utf8(block.name.GetValue()).get_ptr();

	if (block.actions.Exists())
	{
		for (int i = 0; i < block.actions.GetSize(); ++i)
		{
			const ActionS11nBlock& actionBlock = block.actions.GetAt(i);

			// actionGUID is a required field, unnecessary to check if it exists.
			IAction* pPrototype = ServiceManager::Instance().GetActionPrototypesManager().
				GetPrototypeByGUID(actionBlock.actionGUID.GetValue());

			if (!pPrototype)
				continue;

			std::unique_ptr<IAction> pAction(pPrototype->Clone());
			pAction->LoadFromS11nBlock(actionBlock);

			m_actions.push_back(std::move(pAction));
		}
	}

	if (block.restartAfterCompletion.Exists())
		m_restartAfterCompletion = block.restartAfterCompletion.GetValue();
}

void ActionList::SaveToS11nBlock(ActionListS11nBlock& block) const
{
	block.guid.SetValue(m_actionListGUID);
	block.name.SetValue(pfc::stringcvt::string_utf8_from_wide(m_name.c_str()).toString());

	for (std::size_t i = 0; i < m_actions.size(); ++i)
	{
		ActionS11nBlock actionBlock;
		actionBlock.actionGUID.SetValue(m_actions[i].GetPrototypeGUID());

		m_actions[i].SaveToS11nBlock(actionBlock);
		block.actions.Add(actionBlock);
	}

	block.restartAfterCompletion.SetValue(m_restartAfterCompletion);
}

//------------------------------------------------------------------------------
// ActionListEditor
//------------------------------------------------------------------------------

ActionListEditor::ActionListEditor(ActionList* pActionList, PrefPageModel* pPrefPageModel) :
	m_pActionList(pActionList), m_pPrefPageModel(pPrefPageModel)
{

}

BOOL ActionListEditor::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	m_actionListName = GetDlgItem(IDC_ACTION_LIST_NAME);

	CheckDlgButton(IDC_CHECK_RESTART_AFTER_COMPLETION, m_pActionList->GetRestartAfterCompletion());

	m_actionListName.SetWindowText(m_pActionList->GetName().c_str());
	m_actionListName.SetSel(0, -1);

	m_actionListName.SetFocus();

	CenterWindow(GetParent());

	// dark mode
	m_dark.AddDialogWithControls(m_hWnd);

	return FALSE;
}

void ActionListEditor::OnClose(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (nID == IDOK)
	{
		CString text;
		m_actionListName.GetWindowText(text);

		text.Trim();

		if (text.IsEmpty())
		{
			m_popupTooltipMsg.Show(L"Enter action list's name.", m_actionListName);
			return;
		}

		if (!CheckActionListName(static_cast<LPCWSTR>(text)))
		{
			m_popupTooltipMsg.Show(L"Action list with this name already exists.", m_actionListName);
			return;
		}

		m_pActionList->SetName(static_cast<LPCWSTR>(text));
		m_pActionList->SetRestartAfterCompletion(IsDlgButtonChecked(IDC_CHECK_RESTART_AFTER_COMPLETION) == TRUE);
	}

	EndDialog(nID);
}

bool ActionListEditor::CheckActionListName(const std::wstring& actionListName) const
{
	std::vector<ActionList*> alists = m_pPrefPageModel->GetActionLists();

	for (std::size_t i = 0; i < alists.size(); ++i)
	{
		if ((alists[i]->GetName() == actionListName) && (m_pActionList != alists[i]))
			return false;
	}

	return true;
}
