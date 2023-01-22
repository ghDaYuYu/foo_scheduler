#pragma once

#include "model.h"
#include "event.h"
#include "action.h"
#include "action_list.h"

class PrefPageModel : boost::noncopyable
{
public:
	explicit PrefPageModel(const ModelState& modelState);

	const ModelState& GetState() const;

	typedef boost::signals2::signal<void ()> ModelResetSignal;
	boost::signals2::connection ConnectModelResetSlot(const ModelResetSignal::slot_type& slot);

	void Reset();

	typedef boost::signals2::signal<void ()> ModelChangedSignal;
	boost::signals2::connection ConnectModelChangedSlot(const ModelChangedSignal::slot_type& slot);

	//////////////////////////////////////////////////////////////////////////
	// Scheduler status

	bool IsSchedulerEnabled() const;
	void SetSchedulerEnabled(bool enabled);

	//////////////////////////////////////////////////////////////////////////
	// Events

	typedef boost::signals2::signal<void (Event*)> EventAddedSignal;
	typedef boost::signals2::signal<void (Event*)> EventUpdatedSignal;
	typedef boost::signals2::signal<void (Event*)> EventRemovedSignal;

	boost::signals2::connection ConnectEventAddedSlot(const EventAddedSignal::slot_type& slot);
	boost::signals2::connection ConnectEventUpdatedSlot(const EventUpdatedSignal::slot_type& slot);
	boost::signals2::connection ConnectEventRemovedSlot(const EventRemovedSignal::slot_type& slot);

	std::vector<Event*> GetEvents();

	void AddEvent(std::unique_ptr<Event>&& pEvent);
	void UpdateEvent(Event* pEvent);
	void RemoveEvent(Event* pEvent);

	bool CanMoveEventUp(const Event* pEvent) const;
	bool CanMoveEventDown(const Event* pEvent) const;

	void MoveEventUp(const Event* pEvent);
	void MoveEventDown(const Event* pEvent);
	void SwapEvent(const Event* pEvent, const size_t left, const size_t right);

	//////////////////////////////////////////////////////////////////////////
	// Actions

	typedef boost::signals2::signal<void (ActionList*)> ActionListAddedSignal;
	typedef boost::signals2::signal<void (ActionList*)> ActionListRemovedSignal;
	typedef boost::signals2::signal<void (ActionList*)> ActionListUpdatedSignal;

	std::vector<ActionList*> GetActionLists();

	ActionList* GetActionListByGUID(const GUID& guid);

	void AddActionList(std::unique_ptr<ActionList>&& pActionList);
	void RemoveActionList(ActionList* pActionList);
	void UpdateActionList(ActionList* pActionList);

	boost::signals2::connection ConnectActionListAddedSlot(const ActionListAddedSignal::slot_type& slot);
	boost::signals2::connection ConnectActionListRemovedSlot(const ActionListRemovedSignal::slot_type& slot);
	boost::signals2::connection ConnectActionListUpdatedSlot(const ActionListUpdatedSignal::slot_type& slot);

	typedef boost::signals2::signal<void (ActionList*, IAction*)> ActionAddedSignal;
	typedef boost::signals2::signal<void (ActionList*, IAction*)> ActionUpdatedSignal;
	typedef boost::signals2::signal<void (ActionList*, IAction*)> ActionRemovedSignal;

	void AddActionToActionList(ActionList* pActionList, std::unique_ptr<IAction> pAction);
	void UpdateAction(ActionList* pActionList, IAction* pAction);
	void RemoveAction(ActionList* pActionList, IAction* pAction);

	boost::signals2::connection ConnectActionAddedSlot(const ActionAddedSignal::slot_type& slot);
	boost::signals2::connection ConnectActionUpdatedSlot(const ActionUpdatedSignal::slot_type& slot);
	boost::signals2::connection ConnectActionRemovedSlot(const ActionRemovedSignal::slot_type& slot);

private:
	void MoveEvent(const Event* pEvent, bool up);

private:
	ModelState m_modelState;

	EventAddedSignal m_eventAddedSignal;
	EventUpdatedSignal m_eventUpdatedSignal;
	EventRemovedSignal m_eventRemovedSignal;

	ActionListAddedSignal m_actionListAddedSignal;
	ActionListRemovedSignal m_actionListRemovedSignal;
	ActionListUpdatedSignal m_actionListUpdatedSignal;

	ModelChangedSignal m_modelChangedSignal;

	ActionAddedSignal m_actionAddedSignal;
	ActionUpdatedSignal m_actionUpdatedSignal;
	ActionRemovedSignal m_actionRemovedSignal;

	ModelResetSignal m_modelResetSignal;
};