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
static const GUID guid_cfg_enabled = { 0x1f352536, 0xe21d, 0x4bf5, { 0xa0, 0xe1, 0x60, 0xc7, 0xee, 0xb0, 0xad, 0x6a } };
cfg_bool cfg_enabled(guid_cfg_enabled, false);

// window placement variable
// Stores the position of the window.
// This variable type is provided by the SDK helpers library.
static const GUID guid_cfg_popup_window_placement = { 0x1dbb8f5f, 0xaf6d, 0x4649, { 0xa8, 0xb3, 0x94, 0x44, 0xaa, 0xf8, 0x22, 0xa4 } };
cfg_window_placement cfg_popup_window_placement(guid_cfg_popup_window_placement);


// helper function to get the font used for message boxes as uLOGFONT structure
t_font_description get_def_font()
{
	NONCLIENTMETRICS ncmetrics = { 0 };
	ncmetrics.cbSize = sizeof(ncmetrics);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncmetrics), &ncmetrics, 0);
	return t_font_description::g_from_font(CreateFontIndirect(&ncmetrics.lfMessageFont));
}
