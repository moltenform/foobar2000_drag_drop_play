#include "stdafx.h"
#include "window.h"

#include "config.h"



/**************************
Providing information about a component.

We can provide some information about our component that 
users will be able to view under Preferences > Components.
**************************/
DECLARE_COMPONENT_VERSION(
	// component name
	EXTENSIONNAME,
	// component version
	"1.0-beta",
	// about text
	"Plugin for Foobar2000 audio player that plays music instantly after drag and drop\n"
	"for foobar2000\n"
	"\n"
	"contributors:\n"
	"Ben Fisher"
);


// Our component will show its window at initialization time, if
// the window is enabled.
class initquit_dragdropplay : public initquit {
	virtual void on_init() {
		// Show the window, if it is enabled.
		if (cfg_enabled)
			CDragDropPlayWindow::ShowWindow();
	}

	virtual void on_quit() {
		// Do nothing.
		// The window placement variable will automatically store the 
		// position and size of our window, if it is currently visible.
	}
};

static initquit_factory_t< initquit_dragdropplay > foo_initquit;

/**************************
Providing menu commands.
**************************/
class mainmenu_commands_dragdropplay : public mainmenu_commands {
	// Return the number of commands we provide.
	virtual t_uint32 get_command_count() {
		return 1;
	}

	// All commands are identified by a GUID.
	virtual GUID get_command(t_uint32 p_index) {
		static const GUID guid_main_dragdropplay = { 0x79e63d8a, 0x856d, 0x47c3, { 0x9f, 0xa, 0xd8, 0x8a, 0xf3, 0x25, 0x26, 0x4f } };

		if (p_index == 0)
			return guid_main_dragdropplay;
		return pfc::guid_null;
	}

	// Set p_out to the name of the n-th command.
	// This name is used to identify the command and determines
	// the default position of the command in the menu.
	virtual void get_name(t_uint32 p_index, pfc::string_base & p_out) {
		if (p_index == 0)
			p_out = EXTENSIONNAME;
	}

	// Set p_out to the description for the n-th command.
	virtual bool get_description(t_uint32 p_index, pfc::string_base & p_out) {
		if (p_index == 0)
			p_out = "Toggles " EXTENSIONNAME " window.";
		else
			return false;
		return true;
	}

	// Every set of commands needs to declare which group it belongs to.
	virtual GUID get_parent()
	{
		return mainmenu_groups::view;
	}

	// Execute n-th command.
	// p_callback is reserved for future use.
	virtual void execute(t_uint32 p_index, service_ptr_t<service_base> p_callback) {
		if (p_index == 0 && core_api::assert_main_thread()) {
			if (cfg_enabled)
				// Hide and disable the window.
				CDragDropPlayWindow::HideWindow();
			else
				// Show and enable the window.
				CDragDropPlayWindow::ShowWindow();
		}
	}

	// The standard version of this command does not support checked or disabled
	// commands, so we use our own version.
	virtual bool get_display(t_uint32 p_index, pfc::string_base & p_text, t_uint32 & p_flags)
	{
		p_flags = 0;
		if (is_checked(p_index))
			p_flags |= flag_checked;
		get_name(p_index,p_text);
		return true;
	}

	// Return whether the n-th command is checked.
	bool is_checked(t_uint32 p_index) {
		if (p_index == 0)
			return cfg_enabled;
		return false;
	}
};

// We need to create a service factory for our menu item class,
// otherwise the menu commands won't be known to the system.
static mainmenu_commands_factory_t< mainmenu_commands_dragdropplay > foo_menu;
