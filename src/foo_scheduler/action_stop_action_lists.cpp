#include "pch.h"
#include "action_stop_action_lists.h"
#include "service_manager.h"

//------------------------------------------------------------------------------
// ActionStopActionLists
//------------------------------------------------------------------------------

GUID ActionStopActionLists::GetPrototypeGUID() const
{
	// {016F5CEA-E834-4A01-A3BD-CEC3303911C8}
	static const GUID result =
	{ 0x16f5cea, 0xe834, 0x4a01, { 0xa3, 0xbd, 0xce, 0xc3, 0x30, 0x39, 0x11, 0xc8 } };

	return result;
}

int ActionStopActionLists::GetPriority() const
{
	return 70;
}

std::wstring ActionStopActionLists::GetName() const
{
	return L"Stop all tasks but this";
}

IAction* ActionStopActionLists::Clone() const
{
	return new ActionStopActionLists(*this);
}

std::wstring ActionStopActionLists::GetDescription() const
{
	return GetName();
}

bool ActionStopActionLists::HasConfigDialog() const
{
	return false;
}

bool ActionStopActionLists::ShowConfigDialog(CWindow parent)
{
	_ASSERTE(false);
	return false;
}

ActionExecSessionPtr ActionStopActionLists::CreateExecSession() const
{
	return ActionExecSessionPtr(new ExecSession(*this));
}

void ActionStopActionLists::LoadFromS11nBlock(const ActionS11nBlock& block)
{

}

void ActionStopActionLists::SaveToS11nBlock(ActionS11nBlock& block) const
{

}

namespace
{
	const bool registered = ServiceManager::Instance().GetActionPrototypesManager().RegisterPrototype(
		new ActionStopActionLists);
}

//------------------------------------------------------------------------------
// ActionStopActionLists::ExecSession
//------------------------------------------------------------------------------

ActionStopActionLists::ExecSession::ExecSession(const ActionStopActionLists& action) : m_action(action)
{
}

void ActionStopActionLists::ExecSession::Run(const AsyncCall::CallbackPtr& completionCall)
{
	ServiceManager::Instance().GetRootController().RemoveAllExecSessionsBut(&m_alesFuncs->GetActionListExecSession());

	AsyncCall::AsyncRunInMainThread(completionCall);
}

const IAction* ActionStopActionLists::ExecSession::GetParentAction() const
{
	return &m_action;
}

void ActionStopActionLists::ExecSession::Init(IActionListExecSessionFuncs& alesFuncs)
{
	m_alesFuncs = &alesFuncs;
}

bool ActionStopActionLists::ExecSession::GetCurrentStateDescription(std::wstring& /*descr*/) const
{
	return false;
}