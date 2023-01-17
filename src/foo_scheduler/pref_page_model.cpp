#include "pch.h"
#include "pref_page_model.h"

#include "date_time_event.h"
#include "player_event.h"
#include "menu_item_event.h"

#include "action_start_playback.h"

PrefPageModel::PrefPageModel(const ModelState& modelState) : m_modelState(modelState)
{
}

void PrefPageModel::AddEvent(std::unique_ptr<Event>&& pEvent)
{
	Event* pE = pEvent.get();
	m_modelState.events.push_back(std::move(pEvent));
	m_eventAddedSignal(pE);
	m_modelChangedSignal();
}

boost::signals2::connection PrefPageModel::ConnectEventAddedSlot(const EventAddedSignal::slot_type& slot)
{
	return m_eventAddedSignal.connect(slot);
}

std::vector<Event*> PrefPageModel::GetEvents()
{
	std::vector<Event*> result(m_modelState.events.size());
	std::transform(m_modelState.events.begin(), m_modelState.events.end(), result.begin(), &boost::lambda::_1);
	return result;
}

void PrefPageModel::UpdateEvent(Event* pEvent)
{
	m_eventUpdatedSignal(pEvent);
	m_modelChangedSignal();
}

boost::signals2::connection PrefPageModel::ConnectEventUpdatedSlot(const EventUpdatedSignal::slot_type& slot)
{
	return m_eventUpdatedSignal.connect(slot);
}

std::vector<ActionList*> PrefPageModel::GetActionLists()
{
	std::vector<ActionList*> result(m_modelState.actionLists.size());
	std::transform(m_modelState.actionLists.begin(), m_modelState.actionLists.end(), result.begin(), &boost::lambda::_1);
	return result;
}

void PrefPageModel::RemoveEvent(Event* pEvent)
{
	auto it = std::find_if(m_modelState.events.begin(), m_modelState.events.end(), &boost::lambda::_1 == pEvent);
	_ASSERTE(it != m_modelState.events.end());

	ModelState::EventsContainer::auto_type pReleasedEvent = m_modelState.events.release(it);
	m_eventRemovedSignal(pReleasedEvent.get());
	
	m_modelChangedSignal();
}

boost::signals2::connection PrefPageModel::ConnectEventRemovedSlot(const EventRemovedSignal::slot_type& slot)
{
	return m_eventRemovedSignal.connect(slot);
}

void PrefPageModel::AddActionList(std::unique_ptr<ActionList>&& pActionList)
{
	ActionList* pAL = pActionList.get();
	m_modelState.actionLists.push_back(std::move(pActionList));
	
	m_actionListAddedSignal(pAL);
	m_modelChangedSignal();
}

boost::signals2::connection PrefPageModel::ConnectActionListAddedSlot(
	const ActionListAddedSignal::slot_type& slot)
{
	return m_actionListAddedSignal.connect(slot);
}

void PrefPageModel::RemoveActionList(ActionList* pActionList)
{
	auto it = std::find_if(m_modelState.actionLists.begin(), m_modelState.actionLists.end(), &boost::lambda::_1 == pActionList);
	_ASSERTE(it != m_modelState.actionLists.end());

	for (std::size_t i = 0; i < m_modelState.events.size(); ++i)
		if (m_modelState.events[i].GetActionListGUID() == pActionList->GetGUID())
		{
			m_modelState.events[i].SetActionListGUID(pfc::guid_null);
			m_eventUpdatedSignal(&m_modelState.events[i]);
		}

	ModelState::ActionListsContainer::auto_type pReleasedActionList = m_modelState.actionLists.release(it);
	m_actionListRemovedSignal(pReleasedActionList.get());

	m_modelChangedSignal();
}

boost::signals2::connection PrefPageModel::ConnectActionListRemovedSlot(const ActionListRemovedSignal::slot_type& slot)
{
	return m_actionListRemovedSignal.connect(slot);
}

void PrefPageModel::UpdateActionList(ActionList* pActionList)
{
	for (std::size_t i = 0; i < m_modelState.events.size(); ++i)
		if (m_modelState.events[i].GetActionListGUID() == pActionList->GetGUID()) {
			m_eventUpdatedSignal(&m_modelState.events[i]);
		}

	m_actionListUpdatedSignal(pActionList);
	m_modelChangedSignal();
}

boost::signals2::connection PrefPageModel::ConnectActionListUpdatedSlot(const ActionListUpdatedSignal::slot_type& slot)
{
	return m_actionListUpdatedSignal.connect(slot);
}

boost::signals2::connection PrefPageModel::ConnectModelChangedSlot(const ModelChangedSignal::slot_type& slot)
{
	return m_modelChangedSignal.connect(slot);
}

const ModelState& PrefPageModel::GetState() const
{
	return m_modelState;
}

void PrefPageModel::AddActionToActionList(ActionList* pActionList, std::auto_ptr<IAction> pAction)
{
	IAction* pA = pAction.get();
	pActionList->AddAction(pAction);

	m_actionAddedSignal(pActionList, pA);
	m_modelChangedSignal();
}

boost::signals2::connection PrefPageModel::ConnectActionAddedSlot(const ActionAddedSignal::slot_type& slot)
{
	return m_actionAddedSignal.connect(slot);
}

void PrefPageModel::UpdateAction(ActionList* pActionList, IAction* pAction)
{
	m_actionUpdatedSignal(pActionList, pAction);
	m_modelChangedSignal();
}

boost::signals2::connection PrefPageModel::ConnectActionUpdatedSlot(const ActionUpdatedSignal::slot_type& slot)
{
	return m_actionUpdatedSignal.connect(slot);
}

void PrefPageModel::RemoveAction(ActionList* pActionList, IAction* pAction)
{
	ActionList::ActionsContainer::auto_type pReleasedAction = pActionList->RemoveAction(pAction);
	m_actionRemovedSignal(pActionList, pReleasedAction.get());
	m_modelChangedSignal();
}

boost::signals2::connection PrefPageModel::ConnectActionRemovedSlot(const ActionRemovedSignal::slot_type& slot)
{
	return m_actionRemovedSignal.connect(slot);
}

ActionList* PrefPageModel::GetActionListByGUID(const GUID& guid)
{
	auto it = std::find_if(m_modelState.actionLists.begin(), m_modelState.actionLists.end(),
		boost::bind(&ActionList::GetGUID, _1) == guid);

	if (it == m_modelState.actionLists.end())
		return 0;

	return &(*it);
}

void PrefPageModel::Reset()
{
	m_modelState.Reset();

	m_modelResetSignal();
}

boost::signals2::connection PrefPageModel::ConnectModelResetSlot(const ModelResetSignal::slot_type& slot)
{
	return m_modelResetSignal.connect(slot);
}

bool PrefPageModel::CanMoveEventUp(const Event* pEvent) const
{
	auto it =std::find_if(m_modelState.events.cbegin(), m_modelState.events.cend(),	&boost::lambda::_1 == pEvent);
	_ASSERTE(it != m_modelState.events.cend());

	return it != m_modelState.events.cbegin();
}

bool PrefPageModel::CanMoveEventDown(const Event* pEvent) const
{
	auto it = std::find_if(m_modelState.events.cbegin(), m_modelState.events.cend(), &boost::lambda::_1 == pEvent);
	_ASSERTE(it != m_modelState.events.cend());

	++it;

	return it != m_modelState.events.cend();
}

void PrefPageModel::MoveEventUp(const Event* pEvent)
{
	MoveEvent(pEvent, true);
}

void PrefPageModel::MoveEventDown(const Event* pEvent)
{
	MoveEvent(pEvent, false);
}

void PrefPageModel::SwapEvent(const Event* pEvent, const size_t left, const size_t right) {
	auto it = std::find_if(m_modelState.events.begin(), m_modelState.events.end(), &boost::lambda::_1 == pEvent);
	_ASSERTE(it != m_modelState.events.end());

	boost::ptr_vector<Event>::iterator lit = std::begin(m_modelState.events) + left;
	boost::ptr_vector<Event>::iterator rit = std::begin(m_modelState.events) + right;	
	boost::swap(lit, rit);

	m_modelChangedSignal();
}

void PrefPageModel::MoveEvent(const Event* pEvent, bool up)
{
	auto it = std::find_if(m_modelState.events.begin(), m_modelState.events.end(), &boost::lambda::_1 == pEvent);
	_ASSERTE(it != m_modelState.events.end());

	std::size_t pos = std::distance(m_modelState.events.begin(), it) + (up ? -1 : 1);
	ModelState::EventsContainer::auto_type pE = m_modelState.events.release(it);
	m_modelState.events.insert(m_modelState.events.begin() + pos, pE.release());

	m_modelChangedSignal();
}

bool PrefPageModel::IsSchedulerEnabled() const
{
	return m_modelState.schedulerEnabled;
}

void PrefPageModel::SetSchedulerEnabled(bool enabled)
{
	m_modelState.schedulerEnabled = enabled;
	m_modelChangedSignal();
}
