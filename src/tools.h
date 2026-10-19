// Mem Reduct - Tools menu
// Runs external .ps1 / .bat / .cmd / .reg files from a "tools" folder
// next to memreduct.exe. Deliberately uses plain Win32 so it survives
// routine.h API churn.

#pragma once

#include "routine.h"

#define TOOLS_DIRECTORY L"tools"
#define TOOLS_MAX 64

typedef struct _TOOL_ITEM
{
	WCHAR path[MAX_PATH];
	WCHAR name[MAX_PATH];
} TOOL_ITEM, *PTOOL_ITEM;

// Re-scan the tools folder (cheap; call every time a menu is about to open).
VOID _app_tools_refresh ();

ULONG _app_tools_getcount ();

// Full path of the tools folder (creates it if missing). Caller does NOT free.
LPCWSTR _app_tools_getdirectory ();

// Empty hsubmenu and fill it with one item per tool (base_id + index),
// plus a separator and an "Open tools folder" item (open_folder_id).
VOID _app_tools_fillmenu (
	_In_ HMENU hsubmenu,
	_In_ UINT base_id,
	_In_ UINT open_folder_id
);

// TRUE if ctrl_id is one of the dynamic tool ids.
BOOLEAN _app_tools_isid (
	_In_ INT ctrl_id,
	_In_ UINT base_id
);

// Confirm (optionally) and launch the tool for ctrl_id.
VOID _app_tools_run (
	_In_ HWND hwnd,
	_In_ INT ctrl_id,
	_In_ UINT base_id
);

// Open the tools folder in Explorer.
VOID _app_tools_openfolder ();
