#pragma once

#include "window_helper.h"

// This class implements our window. 
// It uses a helper class from window_helper.h that emulates
// ATL/WTL conventions. The custom helper class is used to
// allow to be compiled when ATL/WTL is not
// available, for example on Visual C++ 2008 Express Edition.
// The message_filter is used to process keyboard shortcuts.
// To be notified about playback status changes, we need a play
// callback. Those callbacks ar registered and unregistered in
// foobar2000 0.9. Since all callback methods are guaranteed to
// be called in the context of the main thread, we can derive
// our window class from play_callback and register 'this'.
class CDragDropPlayWindow :
	public CSimpleWindowImpl<CDragDropPlayWindow>,
	private message_filter,
	private playlist_callback
{
public:
	typedef CSimpleWindowImpl<CDragDropPlayWindow> super;

	static void ShowWindow();
	static void HideWindow();

	// Dispatches window messages to the appropriate handler functions.
	BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT & lResult);

	// Message handler functions.
	// The function signatures are intended to be compatible with the MSG_WM_* macros in WTL.
	LRESULT OnCreate(LPCREATESTRUCT pCreateStruct);
	void OnDestroy();
	void OnClose();
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnPaint(HDC hdc);
	void OnPrintClient(HDC hdc, UINT uFlags);

	// Helper to handle double buffering when appropriate.
	void PaintContent(PAINTSTRUCT &ps);

	// Real drawing is done here.
	void Draw(HDC hdc, CRect rcPaint);

	// helpers methods
	HWND Create(HWND hWndParent);
	inline void RedrawWindow() {::RedrawWindow(m_hWnd, 0, 0, RDW_INVALIDATE);}

private:
	// This is a singleton class.
	CDragDropPlayWindow() : _needsUpdate(false) {}
	~CDragDropPlayWindow() {}

	static CDragDropPlayWindow g_instance;
	bool _needsUpdate;

	// message_filter methods
	virtual bool pretranslate_message(MSG * p_msg);

	// playlist_callback methods (the ones we're interested in)
	virtual void on_items_added(t_size p_playlist, t_size p_start, const pfc::list_base_const_t<metadb_handle_ptr> & p_data, const bit_array & p_selection);//inside any of these methods, you can call playlist APIs to get exact info about what happened (but only methods that read playlist state, not those that modify it)

	// playlist_callback methods (the rest)
	virtual void on_items_reordered(t_size p_playlist, const t_size * p_order, t_size p_count) { }//changes selection too; doesnt actually change set of items that are selected or item having focus, just changes their order
	virtual void on_items_removing(t_size p_playlist, const bit_array & p_mask, t_size p_old_count, t_size p_new_count) { }//called before actually removing them
	virtual void on_items_removed(t_size p_playlist, const bit_array & p_mask, t_size p_old_count, t_size p_new_count) { }
	virtual void on_items_selection_change(t_size p_playlist, const bit_array & p_affected, const bit_array & p_state) { }
	virtual void on_item_focus_change(t_size p_playlist, t_size p_from, t_size p_to) { }//focus may be -1 when no item has focus; reminder: focus may also change on other callbacks
	virtual void on_items_modified(t_size p_playlist, const bit_array & p_mask) { }
	virtual void on_items_modified_fromplayback(t_size p_playlist, const bit_array & p_mask, play_control::t_display_level p_level) { }
	virtual void on_items_replaced(t_size p_playlist, const bit_array & p_mask, const pfc::list_base_const_t<t_on_items_replaced_entry> & p_data) { }
	virtual void on_item_ensure_visible(t_size p_playlist, t_size p_idx) { }
	virtual void on_playlist_activate(t_size p_old, t_size p_new) { }
	virtual void on_playlist_created(t_size p_index, const char * p_name, t_size p_name_len) { }
	virtual void on_playlists_reorder(const t_size * p_order, t_size p_count) { }
	virtual void on_playlists_removing(const bit_array & p_mask, t_size p_old_count, t_size p_new_count) { }
	virtual void on_playlists_removed(const bit_array & p_mask, t_size p_old_count, t_size p_new_count) { }
	virtual void on_playlist_renamed(t_size p_index, const char * p_new_name, t_size p_new_name_len) { }
	virtual void on_default_format_changed() { }
	virtual void on_playback_order_changed(t_size p_new_index) { }
	virtual void on_playlist_locked(t_size p_playlist, bool p_locked) { }

private:
	CFont m_font;

	// This is used to notify other components of the selection
	// in our window. In this overly simplistic case, our selection
	// will be empty, when playback is stopped. Otherwise it will
	// contain the playing track.
	ui_selection_holder::ptr m_selection;
};
