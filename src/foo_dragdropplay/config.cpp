#include "stdafx.h"
#include "config.h"

/**************************
Storing settings.

Here settings refer to any information that needs to be
remembered across sessions, like window positions and such.
foobar2000 provides configuration variables to do this,
which come in two different flavors: private and public.
Private configuration variables are derived from cfg_var in
the pfc library; they can only be accessed in the component
that declares them. Public configuration variables are 
dervided from config_object; they can be accessed
from any component. Public configuration variables can be
used from any thread, since they have builtin synchronization,
whereas access to private configuration variables has to be
synchronized by the component where necessary.

We won't use threads in this component, and we don't want
our settings to be publicly available, so we will use the
cfg_var subclasses from the SDK.

Note: Variables are internally identified by a GUID you
pass as the first parameter to the constructor. The second
parameter of the contructor is the default value. Some types
of configuration variables have an implicit default value,
so their constructor takes only one parameter.
**************************/


// We will provide a window that can be toggled on and off
// by the user, so here come the settings for this.

// boolean variable
// Stores whether the window is enabled.
static const GUID guid_cfg_enabled = { 0xdaa7975e, 0x5147, 0x4252, { 0xa8, 0xb8, 0x65, 0x94, 0x0, 0x2e, 0x60, 0x69 } };
cfg_bool cfg_enabled(guid_cfg_enabled, false);

// window placement variable
// Stores the position of the window.
// This variable type is provided by the SDK helpers library.
static const GUID guid_cfg_popup_window_placement = { 0xc2c12570, 0xda02, 0x4832, { 0xae, 0xfd, 0x68, 0x65, 0x80, 0x7a, 0xdc, 0x17 } };
cfg_window_placement cfg_popup_window_placement(guid_cfg_popup_window_placement);

// We will draw some text on our window. Therefore, we
// need a font. Letting the user choose the font is better
// than choosing one ourselves.

// helper function to get the font used for message boxes as uLOGFONT structure
static t_font_description get_def_font()
{
	NONCLIENTMETRICS ncmetrics = { 0 };
	ncmetrics.cbSize = sizeof(ncmetrics);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncmetrics), &ncmetrics, 0);
	return t_font_description::g_from_font(CreateFontIndirect(&ncmetrics.lfMessageFont));
}

// Stores the font used for drawing text on the window.
static const GUID guid_cfg_font = { 0x44983f90, 0x7632, 0x4a3f, { 0x8d, 0x2a, 0xe4, 0x6e, 0x7a, 0xdc, 0x8f, 0x9 } };
cfg_struct_t<t_font_description> cfg_font(guid_cfg_font, get_def_font());

/**************************
Advanced preferences.

Since we want to use a title formatting string for the displayed
information, it would be nice if the user could change it. The
"Advanced" preferences page is normally used for rarely accessed
settings. Here, we use it as a cheap way to avoid having to implement
a user interface for the configuration. The downside is that we
will not be notified when the string changes, so changes will only
become visible when redrawing is triggered through other means.

The advanced preferences is organized as so-called branches and
settings. We will put our single setting in the "Display" branch.
**************************/

static const GUID guid_advconfig_string_format = { 0xabd92014, 0xdebc, 0x4844, { 0xa6, 0x15, 0xa, 0x5c, 0x60, 0xb9, 0x72, 0xab } };
advconfig_string_factory g_advconfig_string_format(
	// display name
	EXTENSIONNAME " title formatting string",
	// GUID of our setting
	guid_advconfig_string_format,
	// GUID of the parent branch
	advconfig_branch::guid_branch_display,
	// sorting priority (we leave it at 0.0)
	0.0,
	// initial value
	"[%album artist% - ]['['%album%[ CD%discnumber%][ #%tracknumber%]']' ]%title%[ '//' %track artist%]"
);
