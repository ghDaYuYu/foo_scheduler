#include "pch.h"
#include "action_set_playback_order.h"
#include "service_manager.h"
#include "combo_helpers.h"

namespace
{
	std::wstring GetPlaybackOrderName(const GUID& orderGUID)
	{
		static_api_ptr_t<playlist_manager> pm;

		for (t_size i = 0; i < pm->playback_order_get_count(); ++i)
			if (pm->playback_order_get_guid(i) == orderGUID)
				return std::wstring(pfc::stringcvt::string_wide_from_utf8(pm->playback_order_get_name(i)).get_ptr());

		return std::wstring();
	}
}

//------------------------------------------------------------------------------
// ActionSetPlaybackOrder
//------------------------------------------------------------------------------

ActionSetPlaybackOrder::ActionSetPlaybackOrder() : m_orderGUID(pfc::guid_null)
{

}

GUID ActionSetPlaybackOrder::GetOrderGUID() const
{
	return m_orderGUID;
}

void ActionSetPlaybackOrder::SetOrderGUID(const GUID& guid)
{
	m_orderGUID = guid;
}

GUID ActionSetPlaybackOrder::GetPrototypeGUID() const
{
	// {e5c95938-dccd-4c83-bdef-dac4103e45c4} 
	static const GUID result = 
	{ 0xe5c95938, 0xdccd, 0x4c83, { 0xbd, 0xef, 0xda, 0xc4, 0x10, 0x3e, 0x45, 0xc4 } };

	return result;
}

int ActionSetPlaybackOrder::GetPriority() const
{
	return 40;
}

std::wstring ActionSetPlaybackOrder::GetName() const
{
	return L"Set playback order";
}

IAction* ActionSetPlaybackOrder::Clone() const
{
	return new ActionSetPlaybackOrder(*this);
}

std::wstring ActionSetPlaybackOrder::GetDescription() const
{
	return boost::str(boost::wformat(L"Set playback order to \"%1%\"") % GetPlaybackOrderName(m_orderGUID));
}

bool ActionSetPlaybackOrder::HasConfigDialog() const
{
	return true;
}

bool ActionSetPlaybackOrder::ShowConfigDialog(CWindow parent)
{
	ActionSetPlaybackOrderEditor dlg(*this);
	return dlg.DoModal(parent) == IDOK;
}

ActionExecSessionPtr ActionSetPlaybackOrder::CreateExecSession() const
{
	return ActionExecSessionPtr(new ExecSession(*this));
}

void ActionSetPlaybackOrder::LoadFromS11nBlock(const ActionS11nBlock& block)
{
	if (!block.setPlaybackOrder.Exists())
		return;

	const ActionSetPlaybackOrderS11nBlock& b = block.setPlaybackOrder.GetValue();

	b.orderGUID.GetValueIfExists(m_orderGUID);
}

void ActionSetPlaybackOrder::SaveToS11nBlock(ActionS11nBlock& block) const
{
	ActionSetPlaybackOrderS11nBlock b;
	b.orderGUID.SetValue(m_orderGUID);

	block.setPlaybackOrder.SetValue(b);
}

namespace
{
	const bool registered = ServiceManager::Instance().GetActionPrototypesManager().RegisterPrototype(
		new ActionSetPlaybackOrder);
}

//------------------------------------------------------------------------------
// ActionSetPlaybackOrder::ExecSession
//------------------------------------------------------------------------------

ActionSetPlaybackOrder::ExecSession::ExecSession(const ActionSetPlaybackOrder& action) : m_action(action)
{
}

void ActionSetPlaybackOrder::ExecSession::Run(const AsyncCall::CallbackPtr& completionCall)
{
	static_api_ptr_t<playlist_manager> pm;

	for (t_size i = 0; i < pm->playback_order_get_count(); ++i)
		if (pm->playback_order_get_guid(i) == m_action.GetOrderGUID())
		{
			pm->playback_order_set_active(i);
			break;
		}

	AsyncCall::AsyncRunInMainThread(completionCall);
}

const IAction* ActionSetPlaybackOrder::ExecSession::GetParentAction() const
{
	return &m_action;
}

void ActionSetPlaybackOrder::ExecSession::Init(IActionListExecSessionFuncs&)
{
}

bool ActionSetPlaybackOrder::ExecSession::GetCurrentStateDescription(std::wstring& /*descr*/) const
{
	return false;
}

//------------------------------------------------------------------------------
// ActionSetPlaybackOrderEditor
//------------------------------------------------------------------------------

ActionSetPlaybackOrderEditor::ActionSetPlaybackOrderEditor(ActionSetPlaybackOrder& action) : m_action(action)
{

}

BOOL ActionSetPlaybackOrderEditor::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	m_orderCombo = GetDlgItem(IDC_COMBO_ORDER);

	static_api_ptr_t<playlist_manager> pm;

	std::vector<std::pair<std::wstring, int>> comboItems;
	int selectedItem = 0;

	for (t_size i = 0; i < pm->playback_order_get_count(); ++i)
	{
		GUID orderGUID = pm->playback_order_get_guid(i);

		m_orderGUIDs.push_back(orderGUID);

		if (orderGUID == m_action.GetOrderGUID())
			selectedItem = static_cast<int>(i);

		std::wstring orderName(pfc::stringcvt::string_wide_from_utf8(pm->playback_order_get_name(i)).get_ptr());

		comboItems.push_back(std::make_pair(orderName, static_cast<int>(i)));
	}

	ComboHelpers::InitCombo(m_orderCombo, comboItems, selectedItem);

	CenterWindow(GetParent());

	// dark mode
	AddDialog(m_hWnd);
	AddControls(m_hWnd);

	return TRUE;
}

void ActionSetPlaybackOrderEditor::OnCloseCmd(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (nID == IDOK)
	{
		m_action.SetOrderGUID(m_orderGUIDs[ComboHelpers::GetSelectedItem<int>(m_orderCombo)]);
	}

	EndDialog(nID);
}