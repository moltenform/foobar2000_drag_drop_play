#include "stdafx.h"
#include "TutorialWindow.h"

#include "config.h"

/**************************
This tutorial is meant to complement to normal SDK documentation,
it does not replace it. In particular, you should read the SDK readme
before or while studying this tutorial.
**************************/

/**************************
Providing information about a component.

We can provide some information about our component that 
users will be able to view under Preferences > Components.
**************************/


DECLARE_COMPONENT_VERSION(
	// component name
	TUTORIAL,
	// component version
	"0.4",
	// about text, use \n to separate multiple lines
	// If you don't want any about text, set this parameter to NULL.
	"Component development tutorial\n"
	"for foobar2000 v0.9.5.2\n"
	"\n"
	"contributors:\n"
	"Holger Stenger"
);




/**************************
Component initialization and shutdown.

Global (static) variables can be assigned an initial value
when loading the DLL by adding an initializer to their
declarations. However, configuration variables don't "know"
their value at that point of time and services aren't available
yet.

On the other hand, a component may need to perform some kind of
cleanup when the application is shutting down. Doing cleanup
when the DLL is unloading may be too late, since services are no
longer available. One example would be storing the final value
of some setting (window position!) in a configuration variable.

So a properly behaving should component should fully initialze
itself when all components have been loaded, and should perform
any necessary cleanup before any component is unloaded. For this
purpose exists the initquit callback.
**************************/


// Our component will show its window at initialization time, if
// the window is enabled.

class initquit_tutorial1 : public initquit {
	virtual void on_init() {
		// Show the window, if it is enabled.
		if (cfg_enabled)
			CTutorialWindow::ShowWindow();
	}

	virtual void on_quit() {
		// Do nothing.
		// The window placement variable will automatically store the 
		// position and size of our window, if it is currently visible.
	}
};

static initquit_factory_t< initquit_tutorial1 > foo_initquit;





/**************************
Providing menu commands.

Menu commands are one of the most important means for users
to interact with a general purpose component. (You may argue
that a component may have its own window with that the user
interacts most of the time, but how does the user open this
window? See?)

There are two basic types of menu commands: main menu and
context menu commands. This tutorial only uses main menu
commands.
**************************/

class mainmenu_commands_tutorial1 : public mainmenu_commands {
	// Return the number of commands we provide.
	virtual t_uint32 get_command_count() {
		return 1;
	}

	// All commands are identified by a GUID.
	virtual GUID get_command(t_uint32 p_index) {
		static const GUID guid_main_tutorial1 = { 0x908174ba, 0xf4a5, 0x4dc2, { 0xa6, 0x8e, 0xfe, 0x85, 0x3f, 0x18, 0xda, 0x28 } };

		if (p_index == 0)
			return guid_main_tutorial1;
		return pfc::guid_null;
	}

	// Set p_out to the name of the n-th command.
	// This name is used to identify the command and determines
	// the default position of the command in the menu.
	virtual void get_name(t_uint32 p_index, pfc::string_base & p_out) {
		if (p_index == 0)
			p_out = TUTORIAL;
	}

	// Set p_out to the description for the n-th command.
	virtual bool get_description(t_uint32 p_index, pfc::string_base & p_out) {
		if (p_index == 0)
			p_out = "Toggles " TUTORIAL " window.";
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
				CTutorialWindow::HideWindow();
			else
				// Show and enable the window.
				CTutorialWindow::ShowWindow();
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
static mainmenu_commands_factory_t< mainmenu_commands_tutorial1 > foo_menu;
