#pragma once

#include "date_time_event_s11n_block.h"
#include "menu_item_event_s11n_block.h"
#include "player_event_s11n_block.h"

struct EventS11nBlock : public S11nBlocks::Block<EventS11nBlock>
{
	S11nBlocks::Field<bool, 1> enabled;
	S11nBlocks::Field<GUID, 2> actionListGUID;

	S11nBlocks::Field<GUID, 3, true> protoGUID; // Required field.

	S11nBlocks::Field<DateTimeEventS11nBlock, 4> dateTimeEvent;
	S11nBlocks::Field<PlayerEventS11nBlock, 5> playerEvent;
	S11nBlocks::Field<MenuItemEventS11nBlock, 6> menuItemEvent;

	S11nBlocks::Field<GUID, 7> eventGUID;

	template<class Archive>
	void RegisterFields(Archive& ar)
	{
		ar.RegisterFields(enabled)(actionListGUID)(protoGUID)(dateTimeEvent)(playerEvent)(menuItemEvent)(eventGUID);
	}
};