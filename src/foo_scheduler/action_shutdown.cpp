#include "pch.h"
#include "action_shutdown.h"
#include "combo_helpers.h"
#include "service_manager.h"
#include "scope_exit_function.h"

//------------------------------------------------------------------------------
// ActionShutdown
//------------------------------------------------------------------------------

ActionShutdown::ActionShutdown() : m_shutdownType(ShutdownMethod::shutdown)
{

}

ShutdownMethod::Type ActionShutdown::GetType() const
{
	return m_shutdownType;
}

void ActionShutdown::SetType(ShutdownMethod::Type type)
{
	m_shutdownType = type;
}

GUID ActionShutdown::GetPrototypeGUID() const
{
	// {89d52623-bc6d-4848-934e-1fa10b88f3b3} 
	static const GUID result = 
	{ 0x89d52623, 0xbc6d, 0x4848, { 0x93, 0x4e, 0x1f, 0xa1, 0x0b, 0x88, 0xf3, 0xb3 } };

	return result;
}

int ActionShutdown::GetPriority() const
{
	return 110;
}

std::wstring ActionShutdown::GetName() const
{
	return L"Shutdown";
}

IAction* ActionShutdown::Clone() const
{
	return new ActionShutdown(*this);
}

std::wstring ActionShutdown::GetDescription() const
{
	return ShutdownMethod::Label(m_shutdownType);
}

bool ActionShutdown::HasConfigDialog() const
{
	return true;
}

bool ActionShutdown::ShowConfigDialog(CWindow parent)
{
	ActionShutdownEditor dlg(*this);
	return dlg.DoModal(parent) == IDOK;
}

ActionExecSessionPtr ActionShutdown::CreateExecSession() const
{
	return ActionExecSessionPtr(new ExecSession(*this));
}

void ActionShutdown::LoadFromS11nBlock(const ActionS11nBlock& block)
{
	if (!block.shutdown.Exists())
		return;

	const ActionShutdownS11nBlock& b = block.shutdown.GetValue();

	if (b.shutdownType.Exists())
		m_shutdownType = static_cast<ShutdownMethod::Type>(b.shutdownType.GetValue());
}

void ActionShutdown::SaveToS11nBlock(ActionS11nBlock& block) const
{
	ActionShutdownS11nBlock b;
	b.shutdownType.SetValue(m_shutdownType);

	block.shutdown.SetValue(b);
}

namespace
{
	const bool registered = ServiceManager::Instance().GetActionPrototypesManager().RegisterPrototype(
		new ActionShutdown);
}

//------------------------------------------------------------------------------
// ActionShutdown::ExecSession
//------------------------------------------------------------------------------

ActionShutdown::ExecSession::ExecSession(const ActionShutdown& action) : m_action(action)
{
}

void ActionShutdown::ExecSession::Run(const AsyncCall::CallbackPtr& completionCall)
{
	ScopeExitFunction scopeExit(boost::bind(&AsyncCall::AsyncRunInMainThread, completionCall));

	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
		return;

	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES) NULL, 0);

	if (GetLastError() != ERROR_SUCCESS)
		return;

	if (m_action.GetType() == ShutdownMethod::hibernate ||
		m_action.GetType() == ShutdownMethod::suspend)
	{
		if (!SetSuspendState(m_action.GetType() == ShutdownMethod::hibernate, TRUE, FALSE))
		{
			if (!SetSystemPowerState(m_action.GetType() == ShutdownMethod::suspend, TRUE))
				return;
		}
	}
	else
	{
		UINT flags = EWX_FORCEIFHUNG;

		switch (m_action.GetType())
		{
		case ShutdownMethod::poweroff:
			flags |= EWX_POWEROFF;
			break;

		case ShutdownMethod::shutdown:
			flags |= EWX_SHUTDOWN;
			break;
		}

		if (!ExitWindowsEx(flags, 0))
			return;
	}

	tkp.Privileges[0].Attributes = 0;
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES) NULL, 0);
}

const IAction* ActionShutdown::ExecSession::GetParentAction() const
{
	return &m_action;
}

void ActionShutdown::ExecSession::Init(IActionListExecSessionFuncs&)
{
}

bool ActionShutdown::ExecSession::GetCurrentStateDescription(std::wstring& /*descr*/) const
{
	return false;
}

//------------------------------------------------------------------------------
// ActionShutdownEditor
//------------------------------------------------------------------------------

ActionShutdownEditor::ActionShutdownEditor(ActionShutdown& action) : m_action(action)
{
}

BOOL ActionShutdownEditor::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	m_typeCombo = GetDlgItem(IDC_COMBO_SHUTDOWN);

	std::vector<std::pair<std::wstring, int>> comboItems;

	comboItems.push_back(std::make_pair(ShutdownMethod::Label(ShutdownMethod::shutdown),
		ShutdownMethod::shutdown));

	int selectedItem = m_action.GetType();

	SYSTEM_POWER_CAPABILITIES spc = {0};
	GetPwrCapabilities(&spc);

	if (spc.SystemS5)
	{
		comboItems.push_back(std::make_pair(ShutdownMethod::Label(ShutdownMethod::poweroff),
			ShutdownMethod::poweroff));
	}
	else if (selectedItem == ShutdownMethod::poweroff)
	{
		selectedItem = ShutdownMethod::shutdown;
	}

	if (spc.SystemS1 || spc.SystemS2 || spc.SystemS3)
	{
		comboItems.push_back(std::make_pair(ShutdownMethod::Label(ShutdownMethod::suspend),
			ShutdownMethod::suspend));
	}
	else if (selectedItem == ShutdownMethod::poweroff)
	{
		selectedItem = ShutdownMethod::shutdown;
	}

	if (spc.SystemS4)
	{
		comboItems.push_back(std::make_pair(ShutdownMethod::Label(ShutdownMethod::hibernate),
			ShutdownMethod::hibernate));
	}
	else if (selectedItem == ShutdownMethod::poweroff)
	{
		selectedItem = ShutdownMethod::shutdown;
	}

	ComboHelpers::InitCombo(m_typeCombo, comboItems, selectedItem);

	CenterWindow(GetParent());

	// dark mode
	AddDialog(m_hWnd);
	AddControls(m_hWnd);

	return TRUE;
}

void ActionShutdownEditor::OnCloseCmd(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (nID == IDOK)
	{
		m_action.SetType(ComboHelpers::GetSelectedItem<ShutdownMethod::Type>(m_typeCombo));
	}

	EndDialog(nID);
}