#pragma once

#include "event_s11n_block.h"
#include "event_visitor.h"

class ActionList;

class Event
{
public:
	virtual ~Event();

	bool IsEnabled() const;
	void Enable(bool enable);

	const GUID& GetEventGUID() const;
	void NewEventGUID();
	const GUID& GetActionListGUID() const;
	void SetActionListGUID(const GUID& guid);

	void Load(const EventS11nBlock& block);
	void Save(EventS11nBlock& block) const;

public:
	virtual GUID GetPrototypeGUID() const = 0;
	virtual int GetPriority() const = 0;
	virtual std::wstring GetName() const = 0;

	virtual std::wstring GetDescription() const = 0;

	// Returns true, if edit has been accepted.
	virtual bool ShowConfigDialog(CWindow parent, class PrefPageModel* pPrefPageModel) = 0;

	// Called after firing an event. In this function event may be deleted.
	virtual void OnSignal() = 0;

	virtual std::unique_ptr<Event> Clone() const = 0;
	virtual std::unique_ptr<Event> CreateFromPrototype() const = 0;

    virtual void ApplyVisitor(IEventVisitor& visitor) = 0;

private: // These functions must be overridden but not accessible.
	virtual void LoadFromS11nBlock(const EventS11nBlock& block) = 0;
	virtual void SaveToS11nBlock(EventS11nBlock& block) const = 0;

protected:
	Event();
	Event(const Event& rhs);
	
private:
	bool m_enabled;
	GUID m_eventGUID;
	GUID m_actionListGUID;
};

// For boost::ptr_vector.
inline Event* new_clone(const Event& e)
{
	return e.Clone().release();
}

