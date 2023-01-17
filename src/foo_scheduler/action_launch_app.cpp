#include "pch.h"
#include "action_launch_app.h"
#include "service_manager.h"

//------------------------------------------------------------------------------
// ActionLaunchApp
//------------------------------------------------------------------------------

ActionLaunchApp::ActionLaunchApp()
{

}

std::wstring ActionLaunchApp::GetCommandLine() const
{
	return m_commandLine;
}

void ActionLaunchApp::SetCommandLine(const std::wstring& val)
{
	m_commandLine = val;
}

GUID ActionLaunchApp::GetPrototypeGUID() const
{
	// {cb3bb06a-cfef-4abb-8456-24b5f5d784dd} 
	static const GUID result = 
	{ 0xcb3bb06a, 0xcfef, 0x4abb, { 0x84, 0x56, 0x24, 0xb5, 0xf5, 0xd7, 0x84, 0xdd } };

	return result;
}

int ActionLaunchApp::GetPriority() const
{
	return 90;
}

std::wstring ActionLaunchApp::GetName() const
{
	return L"Launch application";
}

IAction* ActionLaunchApp::Clone() const
{
	return new ActionLaunchApp(*this);
}

std::wstring ActionLaunchApp::GetDescription() const
{
	return boost::str(boost::wformat(L"Launch \"%1%\"") % m_commandLine);
}

bool ActionLaunchApp::HasConfigDialog() const
{
	return true;
}

bool ActionLaunchApp::ShowConfigDialog(CWindow parent)
{
	ActionLaunchAppEditor dlg(*this);
	return dlg.DoModal(parent) == IDOK;
}

ActionExecSessionPtr ActionLaunchApp::CreateExecSession() const
{
	return ActionExecSessionPtr(new ExecSession(*this));
}

void ActionLaunchApp::LoadFromS11nBlock(const ActionS11nBlock& block)
{
	if (!block.launchApp.Exists())
		return;

	const ActionLaunchAppS11nBlock& b = block.launchApp.GetValue();

	if (b.commandLine.Exists())
		m_commandLine = pfc::stringcvt::string_wide_from_utf8(b.commandLine.GetValue()).get_ptr();
}

void ActionLaunchApp::SaveToS11nBlock(ActionS11nBlock& block) const
{
	ActionLaunchAppS11nBlock b;

	b.commandLine.SetValue(pfc::stringcvt::string_utf8_from_wide(m_commandLine.c_str()).toString());
	block.launchApp.SetValue(b);
}

namespace
{
	const bool registered = ServiceManager::Instance().GetActionPrototypesManager().RegisterPrototype(
		new ActionLaunchApp);
}

//------------------------------------------------------------------------------
// ActionLaunchApp::ExecSession
//------------------------------------------------------------------------------

ActionLaunchApp::ExecSession::ExecSession(const ActionLaunchApp& action) : m_action(action)
{

}

void ActionLaunchApp::ExecSession::Run(const AsyncCall::CallbackPtr& completionCall)
{
	std::wstring cmdLine = L"shell32.dll,ShellExec_RunDLL " + m_action.GetCommandLine();
	ShellExecute(core_api::get_main_window(), L"open", L"RunDLL32.exe", cmdLine.c_str(), NULL, SW_SHOWDEFAULT);

	AsyncCall::AsyncRunInMainThread(completionCall);
}

const IAction* ActionLaunchApp::ExecSession::GetParentAction() const
{
	return &m_action;
}

void ActionLaunchApp::ExecSession::Init(IActionListExecSessionFuncs&)
{
}

bool ActionLaunchApp::ExecSession::GetCurrentStateDescription(std::wstring& /*descr*/) const
{
	return false;
}

//------------------------------------------------------------------------------
// ActionLaunchAppEditor
//------------------------------------------------------------------------------

ActionLaunchAppEditor::ActionLaunchAppEditor(ActionLaunchApp& action) : m_action(action)
{

}

BOOL ActionLaunchAppEditor::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	SetDlgItemText(IDC_EDIT_CMD_LINE, m_action.GetCommandLine().c_str());

	CenterWindow(GetParent());

	// dark mode
	AddDialog(m_hWnd);
	AddControls(m_hWnd);

	return TRUE;
}

void ActionLaunchAppEditor::OnCloseCmd(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (nID == IDOK)
	{
		CString s;
		GetDlgItemText(IDC_EDIT_CMD_LINE, s);

		m_action.SetCommandLine(s.GetString());
	}

	EndDialog(nID);
}


void ActionLaunchAppEditor::OnOpenFile(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	CString s;
	GetDlgItemText(IDC_EDIT_CMD_LINE, s);

	CFileDialog dlg(TRUE, NULL, s, OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
		L"All files (*.*)\0*.*\0");

	if (dlg.DoModal(m_hWnd) == IDOK)
	{
		SetDlgItemText(IDC_EDIT_CMD_LINE, dlg.m_ofn.lpstrFile);
	}
}