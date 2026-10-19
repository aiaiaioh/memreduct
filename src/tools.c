// Mem Reduct - Tools menu
// Copyright (c) 2026 <your name here>, GPL-3.0 (same as Mem Reduct)

#include "routine.h"

#include "main.h"
#include "rapp.h"
#include "tools.h"

#include "resource.h"

#include <shellapi.h>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

static TOOL_ITEM tools_arr[TOOLS_MAX] = {0};
static ULONG tools_count = 0;
static WCHAR tools_dir[MAX_PATH] = {0};

// ---------------------------------------------------------------------------

static INT __cdecl _app_tools_compare (
	_In_ LPCVOID ptr1,
	_In_ LPCVOID ptr2
)
{
	PTOOL_ITEM a = (PTOOL_ITEM)ptr1;
	PTOOL_ITEM b = (PTOOL_ITEM)ptr2;

	return StrCmpLogicalW (a->name, b->name);
}

static BOOLEAN _app_tools_issupported (
	_In_ LPCWSTR file_name
)
{
	LPCWSTR ext;

	ext = PathFindExtensionW (file_name);

	if (!ext || !*ext)
		return FALSE;

	return (
		_wcsicmp (ext, L".ps1") == 0 ||
		_wcsicmp (ext, L".bat") == 0 ||
		_wcsicmp (ext, L".cmd") == 0 ||
		_wcsicmp (ext, L".reg") == 0
	);
}

// "Remove-Temp-Files.ps1" -> "Remove Temp Files"
static VOID _app_tools_makedisplayname (
	_In_ LPCWSTR file_name,
	_Out_writes_ (MAX_PATH) LPWSTR out_name
)
{
	wcscpy_s (out_name, MAX_PATH, file_name);
	PathRemoveExtensionW (out_name);

	for (LPWSTR p = out_name; *p; p++)
	{
		if (*p == L'-' || *p == L'_')
			*p = L' ';

		// '&' is the menu accelerator prefix
		if (*p == L'&')
			*p = L'+';
	}
}

// ---------------------------------------------------------------------------

LPCWSTR _app_tools_getdirectory ()
{
	PR_STRING app_dir;

	if (!tools_dir[0])
	{
		app_dir = _r_app_getdirectory ();

		if (app_dir)
		{
			PathCombineW (tools_dir, app_dir->buffer, TOOLS_DIRECTORY);
		}
	}

	if (tools_dir[0])
		CreateDirectoryW (tools_dir, NULL); // no-op if it exists

	return tools_dir;
}

VOID _app_tools_refresh ()
{
	WIN32_FIND_DATAW find_data;
	WCHAR pattern[MAX_PATH];
	LPCWSTR dir;
	HANDLE hfind;

	tools_count = 0;

	dir = _app_tools_getdirectory ();

	if (!dir || !dir[0])
		return;

	PathCombineW (pattern, dir, L"*");

	hfind = FindFirstFileExW (pattern, FindExInfoBasic, &find_data, FindExSearchNameMatch, NULL, 0);

	if (hfind == INVALID_HANDLE_VALUE)
		return;

	do
	{
		if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;

		if (!_app_tools_issupported (find_data.cFileName))
			continue;

		if (tools_count >= TOOLS_MAX)
			break;

		PathCombineW (tools_arr[tools_count].path, dir, find_data.cFileName);
		_app_tools_makedisplayname (find_data.cFileName, tools_arr[tools_count].name);

		tools_count += 1;
	}
	while (FindNextFileW (hfind, &find_data));

	FindClose (hfind);

	if (tools_count > 1)
		qsort (tools_arr, tools_count, sizeof (TOOL_ITEM), &_app_tools_compare);
}

ULONG _app_tools_getcount ()
{
	return tools_count;
}

// ---------------------------------------------------------------------------

VOID _app_tools_fillmenu (
	_In_ HMENU hsubmenu,
	_In_ UINT base_id,
	_In_ UINT open_folder_id
)
{
	// drop the resource placeholder / stale items
	while (GetMenuItemCount (hsubmenu) > 0)
		RemoveMenu (hsubmenu, 0, MF_BYPOSITION);

	if (tools_count == 0)
	{
		AppendMenuW (hsubmenu, MF_STRING | MF_GRAYED, 0, _r_locale_getstring (IDS_TOOLS_EMPTY));
	}
	else
	{
		for (ULONG i = 0; i < tools_count; i++)
		{
			LPCWSTR ext = PathFindExtensionW (tools_arr[i].path);
			WCHAR label[MAX_PATH + 16];

			// "Remove Temp Files\t.ps1" - right-aligned hint of what kind of file it is
			swprintf_s (label, RTL_NUMBER_OF (label), L"%s\t%s", tools_arr[i].name, ext ? ext : L"");

			AppendMenuW (hsubmenu, MF_STRING, base_id + i, label);
		}
	}

	AppendMenuW (hsubmenu, MF_SEPARATOR, 0, NULL);
	AppendMenuW (hsubmenu, MF_STRING, open_folder_id, _r_locale_getstring (IDS_TOOLS_OPENFOLDER));
}

BOOLEAN _app_tools_isid (
	_In_ INT ctrl_id,
	_In_ UINT base_id
)
{
	return (ctrl_id >= (INT)base_id && ctrl_id < (INT)(base_id + tools_count));
}

// ---------------------------------------------------------------------------

static VOID _app_tools_launch (
	_In_ HWND hwnd,
	_In_ PTOOL_ITEM tool
)
{
	SHELLEXECUTEINFOW sei = {0};
	WCHAR params[MAX_PATH * 2 + 128];
	LPCWSTR ext;
	LPCWSTR exe;
	LPCWSTR verb;

	ext = PathFindExtensionW (tool->path);

	if (_wcsicmp (ext, L".ps1") == 0)
	{
		// Windows PowerShell 5.1 is always present. Bypass applies to this process only.
		// Wrapping in -Command lets us pause so the window doesn't vanish on scripts with no output.
		exe = L"powershell.exe";
		swprintf_s (params, RTL_NUMBER_OF (params),
			L"-NoProfile -ExecutionPolicy Bypass -Command \"& \\\"%s\\\"; Write-Host ''; Pause\"",
			tool->path);
	}
	else if (_wcsicmp (ext, L".reg") == 0)
	{
		// no /s - let regedit show its "are you sure" prompt
		exe = L"regedit.exe";
		swprintf_s (params, RTL_NUMBER_OF (params), L"\"%s\"", tool->path);
	}
	else // .bat / .cmd
	{
		exe = L"cmd.exe";
		swprintf_s (params, RTL_NUMBER_OF (params), L"/d /c call \"%s\" & pause", tool->path);
	}

	// Mem Reduct normally runs elevated; if it doesn't, ask for elevation now
	// instead of letting the script silently fail half-way through.
	verb = _r_sys_iselevated () ? L"open" : L"runas";

	sei.cbSize = sizeof (sei);
	sei.fMask = SEE_MASK_FLAG_NO_UI;
	sei.hwnd = hwnd;
	sei.lpVerb = verb;
	sei.lpFile = exe;
	sei.lpParameters = params;
	sei.lpDirectory = _app_tools_getdirectory ();
	sei.nShow = SW_SHOWNORMAL;

	if (!ShellExecuteExW (&sei))
	{
		WCHAR buffer[MAX_PATH + 128];
		ULONG code = GetLastError ();

		// ERROR_CANCELLED (1223) = user declined the UAC prompt; not worth a dialog
		if (code == ERROR_CANCELLED)
			return;

		// deliberately _r_show_message, not _r_show_errormessage: its argument order
		// has changed between routine revisions and memreduct master.
		swprintf_s (buffer, RTL_NUMBER_OF (buffer), L"Could not start tool (error %lu):\r\n\r\n%s\r\n%s", code, tool->name, tool->path);

		_r_show_message (hwnd, MB_OK | MB_ICONSTOP, _r_locale_getstring (IDS_TOOLS), buffer);
	}
}

VOID _app_tools_run (
	_In_ HWND hwnd,
	_In_ INT ctrl_id,
	_In_ UINT base_id
)
{
	WCHAR buffer[MAX_PATH * 2 + 64];
	PTOOL_ITEM tool;
	ULONG idx;

	idx = (ULONG)(ctrl_id - (INT)base_id);

	if (idx >= tools_count)
		return;

	tool = &tools_arr[idx];

	swprintf_s (buffer, RTL_NUMBER_OF (buffer), L"%s\r\n\r\n%s", tool->name, tool->path);

	// same "don't ask again" pattern memreduct uses for IsShowReductConfirmation
	if (!_r_show_confirmmessage (hwnd, _r_locale_getstring (IDS_QUESTION_TOOL), buffer, L"IsShowToolsConfirmation", FALSE))
		return;

	_app_tools_launch (hwnd, tool);
}

VOID _app_tools_openfolder ()
{
	ShellExecuteW (NULL, L"open", _app_tools_getdirectory (), NULL, NULL, SW_SHOWNORMAL);
}
