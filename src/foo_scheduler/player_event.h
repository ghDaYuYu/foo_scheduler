#pragma once

#include "resource.h"
#include "event.h"
#include "player_event_s11n_block.h"
#include "popup_tooltip_message.h"

//------------------------------------------------------------------------------
// PlayerEvent
//------------------------------------------------------------------------------

namespace PlayerEventType
{
	enum Type
	{
		onPlaybackStart = 0,
		onPlaybackStop,
		onPlaybackPause,
		onPlaybackUnpause,
		onPlaybackNewTrack,

		numEvents
	};

	inline std::wstring Label(Type type)
	{
		switch (type)
		{
		case onPlaybackStart:
			return L"On playback start";

		case onPlaybackStop:
			return L"On playback stop";

		case onPlaybackPause:
			return L"On playback pause";

		case onPlaybackUnpause:
			return L"On playback unpause";

		case onPlaybackNewTrack:
			return L"On new track";
		}

		_ASSERTE(false);
		return std::wstring();
	}

} // namespace PlayerEventType

namespace PlayerEventStopReason
{
	enum Type
	{
		user = 0,
		eof,
		
		numReasons
	};

	inline std::wstring Label(Type type)
	{
		switch (type)
		{
		case user:
			return L"Stop command";

		case eof:
			return L"End-of-file signal";
		}

		_ASSERTE(false);
		return std::wstring();
	}

} // namespace PlayerEventStopReason

class PlayerEvent : public Event
{
public: // Event
	GUID GetPrototypeGUID() const override;
	int GetPriority() const override;
	std::wstring GetName() const override;
	std::wstring GetDescription() const override;
	bool ShowConfigDialog(CWindow parent, class PrefPageModel* pPrefPageModel) override;
	void OnSignal() override;
	std::unique_ptr<Event> Clone() const override;
	std::unique_ptr<Event> CreateFromPrototype() const override;
	void LoadFromS11nBlock(const EventS11nBlock& block) override;
	void SaveToS11nBlock(EventS11nBlock& block) const override;
    void ApplyVisitor(IEventVisitor& visitor) override;

public:
	PlayerEvent();

	enum EFinalAction
	{
		finalActionReenable,
		finalActionDisable,
		finalActionRemove
	};

	PlayerEventType::Type GetType() const;
	void SetType(PlayerEventType::Type type);

	typedef std::vector<PlayerEventStopReason::Type> StopReasons;
	
	const StopReasons& GetStopReasons() const;
	void SetStopReasons(const StopReasons& stopReasons);
	
	EFinalAction GetFinalAction() const;
	void SetFinalAction(EFinalAction finalAction);

private:
	PlayerEvent(const PlayerEvent& rhs);

private:
	PlayerEventType::Type m_type;
	StopReasons m_stopReasons;
	EFinalAction m_finalAction;
};

//------------------------------------------------------------------------------
// PlayerEventEditor
//------------------------------------------------------------------------------

class PlayerEventEditor : public CDialogImpl<PlayerEventEditor>, public fb2k::CDarkModeHooks
{
public:
	enum { IDD = IDD_PLAYER_EVENT_CONFIG };

	PlayerEventEditor(PlayerEvent* pEvent, PrefPageModel* pPrefPageModel);

private:
	BEGIN_MSG_MAP(PlayerEventEditor)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_HANDLER_EX(IDC_COMBO_EVENT, CBN_SELCHANGE, OnEventTypeSelChange)
		COMMAND_ID_HANDLER_EX(IDOK, OnClose)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnClose)
	END_MSG_MAP()

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);
	void OnClose(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnEventTypeSelChange(UINT uNotifyCode, int nID, CWindow wndCtl);

private:
	void CreateStopReasonsControl();
	void UpdateStopReasonsControlVisibility();

private:
	PlayerEvent* m_pEvent;
	PrefPageModel* m_pPrefPageModel;

	CComboBox m_eventType;
	CComboBox m_finalAction;
	CCheckListViewCtrl m_stopReasons;

	int m_stopReasonsDy;
	bool m_stopReasonsControlVisible;

	PopupTooltipMessage m_popupTooltipMsg;
};