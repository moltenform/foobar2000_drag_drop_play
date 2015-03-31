#include "stdafx.h"
#include "window.h"

#include "config.h"

CDragDropPlayWindow CDragDropPlayWindow::g_instance;

void CDragDropPlayWindow::ShowWindow() {
	if (!g_instance.IsWindow()) {
		cfg_enabled = (g_instance.Create(core_api::get_main_window()) != NULL);
	}
}

void CDragDropPlayWindow::HideWindow() {
	// Set window state to disabled.
	cfg_enabled = false;

	// Destroy the window.
	g_instance.Destroy();
}

HWND CDragDropPlayWindow::Create(HWND p_hWndParent) {
	return super::Create(core_api::get_main_window(),
		TEXT(EXTENSIONNAME),
		WS_POPUP | WS_THICKFRAME | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
		WS_EX_TOOLWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 730, 70);
}

BOOL CDragDropPlayWindow::ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT & lResult) {
	switch (uMsg) {
	case WM_CREATE:
	{
		lResult = OnCreate(reinterpret_cast<LPCREATESTRUCT>(lParam));
		return TRUE;
	}

	case WM_DESTROY:
	{
		OnDestroy();
		lResult = 0;
		return TRUE;
	}

	case WM_CLOSE:
	{
		OnClose();
		lResult = 0;
		return TRUE;
	}

	case WM_KEYDOWN:
	{
		OnKeyDown((TCHAR)wParam, (UINT)lParam & 0xfff, (UINT)((lParam >> 16) & 0xffff));
		lResult = 0;
		return TRUE;
	}

	case WM_LBUTTONDOWN:
	{
		OnLButtonDown(wParam, CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
		lResult = 0;
		return TRUE;
	}

	case WM_PAINT:
	{
		OnPaint((HDC)wParam);
		lResult = 0;
		return TRUE;
	}

	case WM_PRINTCLIENT:
	{
		OnPrintClient((HDC)wParam, (UINT)lParam);
		lResult = 0;
		return TRUE;
	}
	}

	// The framework will call DefWindowProc() for us.
	return FALSE;
}

LRESULT CDragDropPlayWindow::OnCreate(LPCREATESTRUCT pCreateStruct) {
	if (DefWindowProc(m_hWnd, WM_CREATE, 0, (LPARAM)pCreateStruct) != 0) return -1;

	// If "Remember window positions" is enabled, this will
	// restore the last position of our window. Otherwise it
	// won't do anything.
	cfg_popup_window_placement.on_window_creation(m_hWnd);

	// Initialize the font.
	m_font = cfg_font.get_value().create();

	// Acquire a ui_selection_holder that allows us to notify other components
	// of the selected tracks in our window, when it has the focus.
	// Also used for keyboard shortcut processing (see pretranslate_message).
	m_selection = static_api_ptr_t<ui_selection_manager>()->acquire();

	// Register ourselves as message_filter, so we can process keyboard shortcuts
	// and (possibly) dialog messages.
	static_api_ptr_t<message_loop>()->add_message_filter(this);

	// Register ourselves for playlist events
	static_api_ptr_t<playlist_manager>()->register_callback(this,
		flag_on_items_added |
		flag_on_items_reordered);

	RedrawWindow();

	return 0;
}

void CDragDropPlayWindow::OnDestroy() {
	m_selection.release();

	static_api_ptr_t<message_loop>()->remove_message_filter(this);

	static_api_ptr_t<playlist_manager>()->unregister_callback(this);

	// Notify the window placement variable that our window
	// was destroyed. This will also update the variables value
	// with the current window position and size.
	cfg_popup_window_placement.on_window_destruction(m_hWnd);
}

void CDragDropPlayWindow::OnClose() {
	// Hide and disable the window.
	HideWindow();
}

void CDragDropPlayWindow::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
	if (nChar == VK_ESCAPE) {
		// Hide and disable the window.
		HideWindow();
	}
}

void CDragDropPlayWindow::OnLButtonDown(UINT nFlags, CPoint point) {
	
}

void CDragDropPlayWindow::OnPaint(HDC hdc) {
	CPaintDC dc(m_hWnd);
	PaintContent(dc.m_ps);
}

void CDragDropPlayWindow::OnPrintClient(HDC hdc, UINT uFlags) {
	PAINTSTRUCT ps = { 0 };
	ps.hdc = hdc;
	GetClientRect(m_hWnd, &ps.rcPaint);
	ps.fErase = FALSE;
	PaintContent(ps);
}

void CDragDropPlayWindow::PaintContent(PAINTSTRUCT &ps) {
	if (GetSystemMetrics(SM_REMOTESESSION)) {
		// Do not use double buffering, if we are running on a Remote Desktop Connection.
		// The system would have to transfer a bitmap everytime our window is painted.
		Draw(ps.hdc, ps.rcPaint);
	}
	else if (!IsRectEmpty(&ps.rcPaint)) {
		// Use double buffering for local drawing.
		CMemoryDC dc(ps.hdc, ps.rcPaint);
		Draw(dc, ps.rcPaint);
	}
}

void CDragDropPlayWindow::Draw(HDC hdc, CRect rcPaint) {
	// We will paint the background in the default window color.
	HBRUSH hBrush = GetSysColorBrush(COLOR_WINDOW);
	FillRect(hdc, rcPaint, hBrush);

	try
	{
		HFONT hFont = m_font;
		if (hFont == NULL)
			hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		SelectObject(hdc, hFont);
		SetTextAlign(hdc, TA_TOP | TA_LEFT);
			
		const char* hardcodedtext[] = { 
			"You can now drag/drop a song onto a playlist and it will start right away.",
			"To turn this off, close this window.",
			"https://github.com/downpoured/foobar2000_drag_drop_play"
		};
		for (int i = 0; i < _countof(hardcodedtext); i++)
		{
			uExtTextOut(hdc, 10, 2 + i * 30, ETO_CLIPPED, rcPaint, hardcodedtext[i], strlen(hardcodedtext[i]), 0);
		}
	}
	catch (const std::exception & exc) {
		console::formatter() << "Exception occurred while drawing " EXTENSIONNAME " window:\n" << exc;
	}

	if (_needsUpdate)
	{
		_needsUpdate = false;

		if ((GetKeyState(VK_CAPITAL) & 0x0001) == 0)
		{
			// it is 0 based index. we need to open the window before anything works.
			static_api_ptr_t<playlist_manager> playlistman;

			// remove all tracks but the last one.
			t_size length = playlistman->activeplaylist_get_item_count();
			if (length >= 2)
			{
				bit_array_bittable whichToRemove(length + 1);
				for (int i = 0; i < (int)(length - 1); i++)
				{
					whichToRemove.set(i, true);
				}

				playlistman->activeplaylist_remove_items(whichToRemove);
			}

			// begin playing the remaining track.
			t_size length2 = playlistman->activeplaylist_get_item_count();
			if (length2 >= 1)
			{
				playlistman->activeplaylist_execute_default_action(0);
			}
		}
	}
}


bool CDragDropPlayWindow::pretranslate_message(MSG * p_msg) {
	// Process keyboard shortcuts
	if (static_api_ptr_t<keyboard_shortcut_manager_v2>()->pretranslate_message(p_msg, m_hWnd)) return true;

	// If you use a dialog or a window with child controls, you can uncomment the following line.
	//if (::IsDialogMessage(m_hWnd, p_msg)) return true;

	return false;
}


void CDragDropPlayWindow::on_items_added(t_size p_playlist, t_size p_start, const pfc::list_base_const_t<metadb_handle_ptr> & p_data, const bit_array & p_selection)
{
	//inside any of these methods, you can call playlist APIs to get exact info about what happened (but only methods that read playlist state, not those that modify it)
	_needsUpdate = true;
	RedrawWindow();
}
