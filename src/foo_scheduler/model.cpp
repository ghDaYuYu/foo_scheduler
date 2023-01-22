#include "pch.h"
#include "model.h"
#include "s11n_blocks.h"
#include "action_list_s11n_block.h"
#include "event_s11n_block.h"
#include "service_manager.h"

Model::Model()
{
}


struct ModelS11nBlock : public S11nBlocks::Block<ModelS11nBlock>
{
	S11nBlocks::RepeatedField<EventS11nBlock, 1> events;
	S11nBlocks::RepeatedField<ActionListS11nBlock, 2> actionLists;
	S11nBlocks::RepeatedField<int, 3> eventWindowColumnsWidths;
	S11nBlocks::Field<bool, 4> schedulerEnabled;

	template<class Archive>
	void RegisterFields(Archive& ar)
	{
		ar.RegisterFields(events)(actionLists)(eventWindowColumnsWidths)(schedulerEnabled);
	}
};

void Model::Load(foobar_stream_reader& stream)
{
	ModelS11nBlock block;

	try
	{
		block.ParseFromStream(stream);
	}
	catch (S11nBlocks::Exception&)
	{
		return;
	}

	if (block.events.Exists())
	{
		for (int i = 0; i < block.events.GetSize(); ++i)
		{
			const EventS11nBlock& eventBlock = block.events.GetAt(i);

			// protoGUID is a required field, unnecessary to check if it exists.
			Event* pPrototype = ServiceManager::Instance().GetEventPrototypesManager().
				GetPrototypeByGUID(eventBlock.protoGUID.GetValue());

			if (!pPrototype)
				continue;

			auto pEvent(pPrototype->Clone());
			pEvent->Load(eventBlock);

			m_modelState.events.push_back(std::move(pEvent));
		}
	}

	if (block.actionLists.Exists())
	{
		for (int i = 0; i < block.actionLists.GetSize(); ++i)
		{
			std::unique_ptr<ActionList> pActionList(new ActionList);
			pActionList->LoadFromS11nBlock(block.actionLists.GetAt(i));
			m_modelState.actionLists.push_back(std::move(pActionList));
		}
	}

	if (block.eventWindowColumnsWidths.Exists())
	{
		for (int i = 0; i < block.eventWindowColumnsWidths.GetSize(); ++i)
			m_eventsWindowColumnsWidths.push_back(block.eventWindowColumnsWidths.GetAt(i));
	}

	block.schedulerEnabled.GetValueIfExists(m_modelState.schedulerEnabled);
}


void Model::Save(foobar_stream_writer& stream) const
{
	ModelS11nBlock block;

	for (std::size_t i = 0; i < m_modelState.events.size(); ++i)
	{
		EventS11nBlock eventBlock;
		eventBlock.eventGUID.SetValue(m_modelState.events[i].GetEventGUID());
		eventBlock.protoGUID.SetValue(m_modelState.events[i].GetPrototypeGUID());

		m_modelState.events[i].Save(eventBlock);
		block.events.Add(eventBlock);
	}

	for (std::size_t i = 0; i < m_modelState.actionLists.size(); ++i)
	{
		ActionListS11nBlock alBlock;
		m_modelState.actionLists[i].SaveToS11nBlock(alBlock);
		block.actionLists.Add(alBlock);
	}

	for (std::size_t i = 0; i < m_eventsWindowColumnsWidths.size(); ++i)
		block.eventWindowColumnsWidths.Add(m_eventsWindowColumnsWidths[i]);

	block.schedulerEnabled.SetValue(m_modelState.schedulerEnabled);

	try
	{
		block.SerializeToStream(stream);
	}
	catch (S11nBlocks::Exception&)
	{

	}
}

std::vector<Event*> Model::GetEvents()
{
	std::vector<Event*> result(m_modelState.events.size());
	std::transform(m_modelState.events.begin(), m_modelState.events.end(), result.begin(), &boost::lambda::_1);
	return result;
}

void Model::UpdateEvent(Event* pEvent)
{
	m_eventUpdatedSignal(pEvent);
}

boost::signals2::connection Model::ConnectEventUpdatedSlot(const EventUpdatedSignal::slot_type& slot)
{
	return m_eventUpdatedSignal.connect(slot);
}

std::vector<ActionList*> Model::GetActionLists()
{
	std::vector<ActionList*> result(m_modelState.actionLists.size());
	std::transform(m_modelState.actionLists.begin(), m_modelState.actionLists.end(), result.begin(), &boost::lambda::_1);
	return result;
}

void Model::RemoveEvent(Event* pEvent)
{
	auto it = std::find_if(m_modelState.events.begin(), m_modelState.events.end(), &boost::lambda::_1 == pEvent);
	_ASSERTE(it != m_modelState.events.end());

	ModelState::EventsContainer::auto_type pReleasedEvent = m_modelState.events.release(it);
	m_eventRemovedSignal(pReleasedEvent.get());
}

boost::signals2::connection Model::ConnectEventRemovedSlot(const EventRemovedSignal::slot_type& slot)
{
	return m_eventRemovedSignal.connect(slot);
}

ActionList* Model::GetActionListByGUID(const GUID& guid)
{
	auto it = std::find_if(m_modelState.actionLists.begin(), m_modelState.actionLists.end(),
		boost::bind(&ActionList::GetGUID, _1) == guid);

	if (it == m_modelState.actionLists.end())
		return 0;

	return &(*it);
}

const ModelState& Model::GetState() const
{
	return m_modelState;
}

void Model::SetState(const ModelState& state)
{
	m_modelState = state;
	m_modelStateChanged();
}

boost::signals2::connection Model::ConnectModelStateChangedSlot(const ModelStateChanged::slot_type& slot)
{
	return m_modelStateChanged.connect(slot);
}

const std::vector<int>& Model::GetEventsWindowColumnsWidths() const
{
	return m_eventsWindowColumnsWidths;
}

void Model::SetEventsWindowColumnsWidths(const std::vector<int>& cw)
{
	m_eventsWindowColumnsWidths = cw;
}

bool Model::IsSchedulerEnabled() const
{
	return m_modelState.schedulerEnabled;
}
