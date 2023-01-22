#include "pch.h"
#include "event.h"
#include "action_list.h"

Event::Event() : m_enabled(true), m_actionListGUID(pfc::guid_null)
{
	::UuidCreate(&m_eventGUID);
}

Event::Event(const Event& rhs) : m_enabled(rhs.m_enabled), m_actionListGUID(rhs.m_actionListGUID),
	m_eventGUID(rhs.GetEventGUID())
{

}

Event::~Event()
{
}

bool Event::IsEnabled() const
{
	return m_enabled;
}

void Event::Enable(bool enable)
{
	m_enabled = enable;
}

const GUID& Event::GetEventGUID() const
{
	return m_eventGUID;
}

void Event::NewEventGUID() {
	::UuidCreate(&m_eventGUID);
}

const GUID& Event::GetActionListGUID() const
{
	return m_actionListGUID;
}

void Event::SetActionListGUID(const GUID& guid)
{
	m_actionListGUID = guid;
}

void Event::Load(const EventS11nBlock& block)
{
	block.enabled.GetValueIfExists(m_enabled);
	block.eventGUID.GetValueIfExists(m_eventGUID);
	block.actionListGUID.GetValueIfExists(m_actionListGUID);

	LoadFromS11nBlock(block);
}

void Event::Save(EventS11nBlock& block) const
{
	block.enabled.SetValue(m_enabled);
	block.eventGUID.SetValue(m_eventGUID);
	block.actionListGUID.SetValue(m_actionListGUID);

	SaveToS11nBlock(block);
}
