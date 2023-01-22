#include "pch.h"
#include "service_manager.h"


ServiceManager& ServiceManager::Instance()
{
	static ServiceManager instance;
	return instance;
}

ServiceManager::ServiceManager()
	: m_rootController()
	, m_model()
	, m_menuItemEventsManager(m_model)
	, m_playerEventsManager(m_model)
	, m_dateTimeEventsManager(m_model)
	, m_timersManager()
	, m_eventPrototypesManager()
	, m_actionPrototypesManager()
{
}

RootController& ServiceManager::GetRootController()
{
	return m_rootController;
}

Model& ServiceManager::GetModel()
{
	return m_model;
}

MenuItemEventsManager& ServiceManager::GetMenuItemEventsManager()
{
	return m_menuItemEventsManager;
}

PlayerEventsManager& ServiceManager::GetPlayerEventsManager()
{
	return m_playerEventsManager;
}

DateTimeEventsManager& ServiceManager::GetDateTimeEventsManager()
{
	return m_dateTimeEventsManager;
}

TimersManager& ServiceManager::GetTimersManager()
{
	return m_timersManager;
}

PrototypesManager<Event>& ServiceManager::GetEventPrototypesManager()
{
	return m_eventPrototypesManager;
}

PrototypesManager<IAction>& ServiceManager::GetActionPrototypesManager()
{
	return m_actionPrototypesManager;
}

EventListWindow* ServiceManager::GetEventListWindow()
{
	return m_eventListWindow;
}

void ServiceManager::SetEventListWindow(EventListWindow* eventListWindow)
{
	m_eventListWindow = eventListWindow;
}