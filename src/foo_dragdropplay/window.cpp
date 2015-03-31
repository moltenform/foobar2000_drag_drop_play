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
		CW_USEDEFAULT, CW_USEDEFAULT, 200, 200);
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

	case WM_CONTEXTMENU:
		{
			OnContextMenu((HWND)wParam, CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
			lResult = 0;
			return TRUE;
		}

	case WM_SETFOCUS:
		{
			OnSetFocus((HWND)wParam);
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

	// Register ourselves as play_callback to get notified about playback events.
	static_api_ptr_t<play_callback_manager>()->register_callback(this,
		flag_on_playback_new_track |
		flag_on_playback_dynamic_info_track |
		flag_on_playback_stop,
		false);

	return 0;
}

void CDragDropPlayWindow::OnDestroy() {
	m_selection.release();

	static_api_ptr_t<message_loop>()->remove_message_filter(this);

	static_api_ptr_t<play_callback_manager>()->unregister_callback(this);

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
	// Get currently playing track.
	static_api_ptr_t<play_control> pc;
	metadb_handle_ptr handle;

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
	} else if (!IsRectEmpty(&ps.rcPaint)) {
		// Use double buffering for local drawing.
		CMemoryDC dc(ps.hdc, ps.rcPaint);
		Draw(dc, ps.rcPaint);
	}
}

void CDragDropPlayWindow::Draw(HDC hdc, CRect rcPaint) {
	// We will paint the background in the default window color.
	HBRUSH hBrush = GetSysColorBrush(COLOR_WINDOW);
	FillRect(hdc, rcPaint, hBrush);

	HICON hIcon = static_api_ptr_t<ui_control>()->get_main_icon();
	if (hIcon != NULL) {
		DrawIconEx(hdc, 2, 2, hIcon, 32, 32, 0, hBrush, DI_NORMAL);
	}

	try
	{
		static_api_ptr_t<play_control> pc;
		metadb_handle_ptr handle;;
		if (pc->get_now_playing(handle)) {
			pfc::string8 format;
			g_advconfig_string_format.get_static_instance().get_state(format);
			service_ptr_t<titleformat_object> script;
			static_api_ptr_t<titleformat_compiler>()->compile_safe(script, format);

			pfc::string_formatter text;
			pc->playback_format_title_ex(handle, NULL, text, script, NULL, play_control::display_level_titles);
			HFONT hFont = m_font;
			if (hFont == NULL)
				hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			SelectObject(hdc, hFont);
			SetTextAlign(hdc, TA_TOP | TA_LEFT);
			uExtTextOut(hdc, 32+4, 2, ETO_CLIPPED, rcPaint, text, text.length(), 0);
		}
	}
	catch (const std::exception & exc) {
		console::formatter() << "Exception occurred while drawing " EXTENSIONNAME " window:\n" << exc;
	}
}

void CDragDropPlayWindow::OnContextMenu(HWND hWnd, CPoint point) {
	// We need some IDs for the context menu.
	enum {
		// ID for "Choose font..."
		ID_FONT = 1,
		// The range ID_CONTEXT_FIRST through ID_CONTEXT_LAST is reserved
		// for menu entries from menu_manager.
		ID_CONTEXT_FIRST,
		ID_CONTEXT_LAST = ID_CONTEXT_FIRST + 1000,
	};

	// Create new popup menu.
	HMENU hMenu = CreatePopupMenu();

	// Add our "Choose font..." command.
	AppendMenu(hMenu, MF_STRING, ID_FONT, TEXT("Choose font..."));

	// Get the currently playing track.
	metadb_handle_list items;
	static_api_ptr_t<play_control> pc;
	metadb_handle_ptr handle;
	if (pc->get_now_playing(handle)) {
		// Insert it into a list.
		items.add_item(handle);
	}

	// Create a menu_manager that will build the context menu.
	service_ptr_t<contextmenu_manager> cmm;
	contextmenu_manager::g_create(cmm);
	// Query setting for showing keyboard shortcuts.
	const bool show_shortcuts = config_object::g_get_data_bool_simple(standard_config_objects::bool_show_keyboard_shortcuts_in_menus, false);
	// Set up flags for contextmenu_manager::init_context.
	unsigned flags = show_shortcuts ? contextmenu_manager::FLAG_SHOW_SHORTCUTS : 0;
	// Initialize menu_manager for using a context menu.
	cmm->init_context(items, flags);
	// If the menu_manager has found any applicable commands,
	// add them to our menu (after a separator).
	if (cmm->get_root()) {
		uAppendMenu(hMenu, MF_SEPARATOR, 0, 0);
		cmm->win32_build_menu(hMenu, ID_CONTEXT_FIRST, ID_CONTEXT_LAST);
	}

	// Use menu helper to gnereate mnemonics.
	menu_helpers::win32_auto_mnemonics(hMenu);

	// Get the location of the mouse pointer.
	// WM_CONTEXTMENU provides position of mouse pointer in argument lp,
	// but this isn't reliable (for example when the user pressed the
	// context menu key on the keyboard).
	CPoint pt;
	GetCursorPos(pt);
	// Show the context menu.
	int cmd = TrackPopupMenu(hMenu, TPM_NONOTIFY | TPM_RETURNCMD | TPM_RIGHTBUTTON, 
		pt.x, pt.y, 0, m_hWnd, 0);

	// Check what command has been chosen. If cmd == 0, then no command
	// was chosen.
	if (cmd == ID_FONT) {
		// Show font configuration.
		t_font_description font = cfg_font;
		if (font.popup_dialog(m_hWnd)) {
			cfg_font = font;
			m_font = font.create();
			::RedrawWindow(m_hWnd, 0, 0, RDW_INVALIDATE|RDW_UPDATENOW);
		}
	} else if (cmd >= ID_CONTEXT_FIRST && cmd <= ID_CONTEXT_LAST ) {
		// Let the menu_manager execute the chosen command.
		cmm->execute_by_id(cmd - ID_CONTEXT_FIRST);
	}

	// contextmenu_manager instance is released automatically, as is the metadb_handle we used.

	// Finally, destroy the popup menu.
	DestroyMenu(hMenu);
}

void CDragDropPlayWindow::OnSetFocus(HWND hWndOld) {
	metadb_handle_list items;
	metadb_handle_ptr track;
	if (static_api_ptr_t<playback_control>()->get_now_playing(track)) {
		items.add_item(track);
	}
	set_selection(items);
}

void CDragDropPlayWindow::set_selection(metadb_handle_list_cref p_items) {
	// Only notify other components about changes in our selection,
	// if our window is the active one.
	if (::GetFocus() == m_hWnd && m_selection.is_valid()) {
		m_selection->set_selection_ex(p_items, contextmenu_item::caller_now_playing);
	}
}

bool CDragDropPlayWindow::pretranslate_message(MSG * p_msg) {
	// Process keyboard shortcuts
	if (static_api_ptr_t<keyboard_shortcut_manager_v2>()->pretranslate_message(p_msg, m_hWnd)) return true;

	// If you use a dialog or a window with child controls, you can uncomment the following line.
	//if (::IsDialogMessage(m_hWnd, p_msg)) return true;

	return false;
}

void CDragDropPlayWindow::on_playback_new_track(metadb_handle_ptr p_track) {
	RedrawWindow();
	set_selection(pfc::list_single_ref_t<metadb_handle_ptr>(p_track));
}

void CDragDropPlayWindow::on_playback_stop(play_control::t_stop_reason reason) {
	RedrawWindow();
	set_selection(metadb_handle_list());
}

void CDragDropPlayWindow::on_playback_dynamic_info_track(const file_info & p_info) {
	RedrawWindow();
}
