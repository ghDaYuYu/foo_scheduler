#pragma once

#define PLUGIN_NAME "Scheduler mod"

#define COMPONENT_VERSION_MAJOR 4
#define COMPONENT_VERSION_MINOR 21
#define COMPONENT_VERSION_PATCH 0
#define COMPONENT_VERSION_SUB_PATCH 0

#define MAKE_STRING(text) #text

//#define FIX_VER
//#define BETA_VER

#ifdef FIX_VER
#define MAKE_COMPONENT_VERSION(major,minor,patch) MAKE_STRING(major) "." MAKE_STRING(minor) "." MAKE_STRING(patch)
#define FOO_SCHEDULER_VERSION MAKE_COMPONENT_VERSION(COMPONENT_VERSION_MAJOR,COMPONENT_VERSION_MINOR,COMPONENT_VERSION_PATCH)
#define MAKE_DLL_VERSION(major,minor,patch,subpatch) MAKE_STRING(major) "." MAKE_STRING(minor) "." MAKE_STRING(patch) "." MAKE_STRING(subpatch)
#else
#ifdef BETA_VER
#define MAKE_COMPONENT_VERSION(major,minor) MAKE_STRING(major) "." MAKE_STRING(minor) " Beta 2"
#else
#define MAKE_COMPONENT_VERSION(major,minor) MAKE_STRING(major) "." MAKE_STRING(minor)
#endif
#define FOO_SCHEDULER_VERSION MAKE_COMPONENT_VERSION(COMPONENT_VERSION_MAJOR,COMPONENT_VERSION_MINOR)
#define MAKE_DLL_VERSION(major,minor) MAKE_STRING(major) "." MAKE_STRING(minor)
#endif

#define MAKE_API_SDK_VERSION(sdk_ver, sdk_target) MAKE_STRING(sdk_ver) " " MAKE_STRING(sdk_target)

#ifdef FIX_VER
#define DLL_VERSION_NUMERIC COMPONENT_VERSION_MAJOR, COMPONENT_VERSION_MINOR, COMPONENT_VERSION_PATCH, COMPONENT_VERSION_SUB_PATCH
#define DLL_VERSION_STRING MAKE_DLL_VERSION(COMPONENT_VERSION_MAJOR,COMPONENT_VERSION_MINOR,COMPONENT_VERSION_PATCH,COMPONENT_VERSION_SUB_PATCH)
#else
#define DLL_VERSION_NUMERIC COMPONENT_VERSION_MAJOR, COMPONENT_VERSION_MINOR
#define DLL_VERSION_STRING MAKE_DLL_VERSION(COMPONENT_VERSION_MAJOR,COMPONENT_VERSION_MINOR)
#endif

//fb2k ver
#define PLUGIN_FB2K_SDK MAKE_API_SDK_VERSION(FOOBAR2000_SDK_VERSION, FOOBAR2000_TARGET_VERSION)

#define COMPONENT_NAME "foo_scheduler_mod"
#define COMPONENT_YEAR "2023"

#define PLUGIN_DLLFILENAME COMPONENT_NAME ".dll"
#define PLUGIN_VERSION FOO_SCHEDULER_VERSION

// To support correct upgrade from the 3rd version of the plugin.
#define PLUGIN_CFG_GLOBAL_V4_19 0x0040
#define PLUGIN_CFG_GLOBAL_V4_19_FLDS 6
#define PLUGIN_CFG_GLOBAL_VERSION 0x0041
#define PLUGIN_CFG_GLOBAL_VERSION_FLDS 7

#define PLUGIN_ABOUT \
"Plugin for scheduling actions like play, stop, system shutdown/wake up and many more." \
" The schedule for triggering actions can be based on time/date events, player events or user-defined menu items.\n\n" \
"Author: da yuyu\n" \
"Version: "FOO_SCHEDULER_VERSION"\n" \
"Compiled: "__DATE__ "\n" \
"fb2k SDK: "PLUGIN_FB2K_SDK"\n" \
"Original Component by Andrew Smolko.\n" \
"\n" \
"Copyright (C) 2006-2023 Andrew Smolko.\n" \
"All rights reserved.\n" \
"\n" \
"Acknowledgements:\n" \
"Thanks to Andrew Smolko for developing the foo_scheduler project (up to version 4.19).\n" \
"\n" \
"Changelog:\n" \
  "\n" \
  "= 4.21\n" \
  "* 'Action list' renamed 'Task'.\n" \
  "* Dark mode: fixed volume dialog.\n" \
  "* Dark mode: fixed date/time picker background color.\n" \
  "* Fixed time/date picker context menu.\n" \
  "\n" \
  "= 4.20\n" \
  "* Fixed empty event list running File > Scheduler mod > Preferences.\n" \
  "* Added 'Playlist from saved state' option to 'Change playlist' action.\n" \
  "\n" \
  "= 4.19.5\n" \
  "* Fixed preferences page font and layout.\n" \
  "\n" \
  "= 4.19.4\n" \
  "* Fixed single track playlists 'Wait until 1 track played'.\n" \
  "* Fixed status window item updates.\n" \
  "* Fixed restore status window screen position.\n" \
  "* Added 'Run' menu item to the event context menu.\n" \
  "* Added 'End of Files' option to 'Wait until N track played'.\n" \
  "\n" \
  "= 4.19.3\n" \
  "* DPI safe Status window placement.\n" \
  "* Added fb2k SDK and target version info to About component.\n" \
  "\n" \
  "= 4.19.2\n" \
  "* Added new option 'Active playlist' to 'Change playlist' action.\n" \
  "\n" \
  "= 4.19.1\n" \
  "* Component renamed 'Scheduler mod'.\n" \
  "* New 32/64bit installer.\n" \
  "* Event list drag & drop.\n" \
  "* Added date/time event context menu (timer initializer).\n" \
  "* Added dark mode.\n" \
  "\n" \
  "= 4.19\n" \
  "* Added 'Restart after completion' option to action list.\n" \
  "* Added 'Stop all action lists but this' action.\n" \
  "\n" \
  "= 4.18\n" \
  "* Fixed 'Wait until N tracks played' action execution right after 'Start playback' action.\n" \
  "\n" \
  "= 4.17\n" \
  "* Added action list and event duplicate functionality.\n" \
  "* Added 'Save playback state' action and updated 'Start playback' to start from saved state.\n" \
  "\n" \
  "= 4.14\n" \
  "* Recovered WinXP support.\n" \
  "\n" \
  "= 4.13\n" \
  "* Internal small fixes.\n" \
  "\n" \
  "= 4.12\n" \
  "* Fixed wrong firing time for repeated events.\n" \
  "\n" \
  "= 4.11\n" \
  "* 'Scheduler enabled' checkbox has been added.\n" \
  "\n" \
  "= 4.10\n" \
  "* Removed useless stop reason in player stop event.\n" \
  "* Dropped packing of executable to be able to analyze crash dumps.\n" \
  "* Rebuild with more stable versions of internal libraries.\n" \
  "\n" \
  "= 4.09\n" \
  "* Completely fixed keyboard shortcuts assignment problem.\n" \
  "\n" \
  "= 4.08\n" \
  "* Fixed tab order in 'Wait until N tracks played' configuration dialog.\n" \
  "* Fixed a bug with menu items, when keyboard shortcuts couldn't be assigned to them correctly. Please recreate your menu item events!\n" \
  "\n" \
  "= 4.07\n" \
  "* Added a new command 'Stop all action lists' to the status window and to the main menu.\n" \
  "* Header restyling to conform the style of preferences page.\n" \
  "* New action 'Wait until N tracks played'.\n" \
  "\n" \
  "= 4.06\n" \
  "* Next/previous track actions added.\n" \
  "\n" \
  "= 4.05\n" \
  "* Preferences window fix due to changes in fb2k 1.1.\n" \
  "\n" \
  "= 4.04\n" \
  "* Added items reordering in events list.\n" \
  "* The width of each column in events list window is stored in configuration.\n" \
  "\n" \
  "= 4.03\n" \
  "* Bug fix release.\n" \
  "\n" \
  "= 4.02\n" \
  "* Added more detailed config of 'On playback stop' event.\n" \
  "* Remaining time of 'Delay' and 'Set volume' actions is now displayed in status window.\n" \
  "\n" \
  "= 4.01\n" \
  "* Added keyboard context menu invocation in events list view and action lists view.\n" \
  "* Volume fade out bug fix.\n" \
  "* Volume fade in/out now uses logarithmic scale.\n" \
  "* Final action combo-box has been returned in date/time event.\n" \
  "* New 'Mute/unmute' action.\n" \
  "\n" \
  "= 4.0\n" \
  "The plugin is rewritten from scratch. Short list of changes:\n" \
  "* New 'OK, Cancel, Apply' preferences page style, introduced in fb2k 1.0.\n" \
  "* 'Reset page' button works correct now.\n" \
  "* Scheduler isn't stopped while its preferences page is opened. It's running all the time.\n" \
  "* Added new status window, where you can see pending date/time events and manage running action lists.\n" \
  "* Fixed crash with concurrently running action lists.\n" \
  "* Fixed incorrect handling of date/time events in some cases.\n" \
  "* Date/time events: completely new configuration dialog, added title support, 'remove/disable event' option removed.\n" \
  "* No more annoying message boxes, now only balloons are used.\n" \
  "* Menus, treeviews and listviews use native Windows Vista/7 style.\n" \
  "* New player events: on playback start, stop, pause, unpause, track change.\n" \
  "* New actions: Pause/Unpause, Change playlist, Launch application.\n" \
  "* For 'Delay' and 'Set volume' actions duration units can be selected.\n" \
  "* New File/Scheduler menu.\n" \
  "* No more configuration reset during version upgrade.\n"
