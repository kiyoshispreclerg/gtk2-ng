/* GDK - The GIMP Drawing Kit
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/* X-DENSITY, client side: a compositor asks a window to render, for a
 * while, at a higher density into an extra pixmap it then samples instead
 * of magnifying the window (a sharp zoom), while the window's geometry
 * never changes. Protocol: xnsguard TESTS/X-DENSITY.md.
 *
 * Three CARDINAL[32] properties on the window:
 *   _X_DENSITY_REQUESTED [num, den]  compositor -> client
 *   _X_DENSITY_SCALE     [num, den]  client -> compositor, density in effect
 *   _X_DENSITY_PIXMAP    [xid]       client -> compositor, the dense pixmap
 * and a manager selection, _X_DENSITY_MANAGER_S<screen>, owned by a
 * compositor while it consumes the protocol. Without an owner, density is
 * ignored altogether, _X_DENSITY_REQUESTED included.
 *
 * The rendering itself is generic, see _gdk_window_set_density().
 * Here: reading requests, following the manager, and publishing the
 * pixmap. Only native windows that get _X_DENSITY_REQUESTED take part,
 * which in practice means toplevels and popups: with client-side windows,
 * a GTK window's widgets all live in its one X window.
 *
 * With nothing asking, nothing here happens beyond interning the manager
 * atom and selecting XFixes events for it on the root window.
 */

#include "config.h"

#include <math.h>
#include <X11/Xatom.h>
#ifdef HAVE_XFIXES
#include <X11/extensions/Xfixes.h>
#endif

#include "gdkx.h"
#include "gdkdisplay-x11.h"
#include "gdkscreen-x11.h"
#include "gdkprivate-x11.h"
#include "gdkinternals.h"
#include "gdkalias.h"

/* a compositor asking for more than this is not doing a zoom */
#define MAX_DENSITY 8

typedef struct
{
  gint num, den;         /* density in effect, 1/1 when none */
  GdkPixmap *announced;  /* pixmap published in _X_DENSITY_PIXMAP, kept
                          * alive until a new one replaces it */
} DensityX11;

static GQuark quark_density_x11;

static Atom
density_manager_atom (GdkScreen *screen)
{
  gchar *name = g_strdup_printf ("_X_DENSITY_MANAGER_S%d", gdk_screen_get_number (screen));
  Atom atom = gdk_x11_get_xatom_by_name_for_display (gdk_screen_get_display (screen), name);

  g_free (name);
  return atom;
}

static gboolean
density_manager_present (GdkScreen *screen)
{
  return XGetSelectionOwner (GDK_SCREEN_XDISPLAY (screen),
                             density_manager_atom (screen)) != None;
}

/* _X_DENSITY_REQUESTED, 1/1 when absent or malformed */
static void
density_read_request (GdkWindow *window,
                      gint      *num,
                      gint      *den)
{
  GdkDisplay *display = gdk_drawable_get_display (window);
  Atom type;
  gint format;
  gulong nitems, after;
  guchar *data = NULL;

  *num = *den = 1;

  gdk_error_trap_push ();
  if (XGetWindowProperty (GDK_DISPLAY_XDISPLAY (display), GDK_WINDOW_XID (window),
                          gdk_x11_get_xatom_by_name_for_display (display, "_X_DENSITY_REQUESTED"),
                          0, 2, False, XA_CARDINAL,
                          &type, &format, &nitems, &after, &data) == Success &&
      type == XA_CARDINAL && format == 32 && nitems == 2)
    {
      /* format 32 property data comes back as longs */
      glong n = ((glong *) data)[0], d = ((glong *) data)[1];

      if (n > 0 && d > 0 && n >= d && n <= MAX_DENSITY * d)
        {
          *num = n;
          *den = d;
        }
    }
  if (data)
    XFree (data);
  gdk_error_trap_pop ();
}

static void
density_set_cardinals (GdkWindow   *window,
                       const gchar *property,
                       glong       *values,
                       gint         n)
{
  GdkDisplay *display = gdk_drawable_get_display (window);

  XChangeProperty (GDK_DISPLAY_XDISPLAY (display), GDK_WINDOW_XID (window),
                   gdk_x11_get_xatom_by_name_for_display (display, property),
                   XA_CARDINAL, 32, PropModeReplace, (guchar *) values, n);
}

static void
density_x11_free (gpointer data)
{
  DensityX11 *density = data;

  if (density->announced)
    g_object_unref (density->announced);
  g_free (density);
}

/* New contents landed in the dense pixmap (after the drawing requests,
 * so the properties below reach the server after the contents) */
static void
density_pushed (GdkWindow *window,
                GdkPixmap *pixmap,
                gboolean   complete,
                gpointer   data)
{
  DensityX11 *density = g_object_get_qdata (G_OBJECT (window), quark_density_x11);
  glong values[2];

  /* a freshly (re)allocated pixmap is only announced once all of the
   * window has been drawn into it */
  if (!density || !complete || GDK_WINDOW_DESTROYED (window))
    return;

  /* a Pixmap raises no Damage of its own: rewriting the property with the
   * same XID is the compositor's "new frame" signal (X-DENSITY.md 4) */
  values[0] = GDK_PIXMAP_XID (pixmap);
  density_set_cardinals (window, "_X_DENSITY_PIXMAP", values, 1);

  if (density->announced != pixmap)
    {
      values[0] = density->num;
      values[1] = density->den;
      density_set_cardinals (window, "_X_DENSITY_SCALE", values, 2);

      /* the compositor may have been sampling the old one until now */
      if (density->announced)
        g_object_unref (density->announced);
      density->announced = g_object_ref (pixmap);
    }
}

/* Bring the window's density in line with its request and the manager */
static void
density_update_window (GdkWindow *window)
{
  DensityX11 *density;
  gint num = 1, den = 1;

  if (GDK_WINDOW_DESTROYED (window) || !_gdk_window_has_impl (window))
    return;

  if (density_manager_present (gdk_drawable_get_screen (window)))
    density_read_request (window, &num, &den);

  if (!quark_density_x11)
    quark_density_x11 = g_quark_from_static_string ("gdk-window-density-x11");
  density = g_object_get_qdata (G_OBJECT (window), quark_density_x11);

  if (num == den)
    {
      glong values[2] = { 1, 1 };

      if (!density)
        return;

      /* draw into the real window again before dropping the pixmap */
      _gdk_window_set_density (window, 1, NULL, NULL);
      gdk_error_trap_push ();
      XDeleteProperty (GDK_WINDOW_XDISPLAY (window), GDK_WINDOW_XID (window),
                       gdk_x11_get_xatom_by_name_for_display (gdk_drawable_get_display (window),
                                                              "_X_DENSITY_PIXMAP"));
      density_set_cardinals (window, "_X_DENSITY_SCALE", values, 2);
      gdk_error_trap_pop ();
      g_object_set_qdata (G_OBJECT (window), quark_density_x11, NULL);
      return;
    }

  if (!density)
    {
      density = g_new0 (DensityX11, 1);
      g_object_set_qdata_full (G_OBJECT (window), quark_density_x11,
                               density, density_x11_free);
    }
  else if (density->num == num && density->den == den)
    return;

  density->num = num;
  density->den = den;
  /* the new pixmap gets announced with the new scale once it is drawn */
  _gdk_window_set_density (window, (gdouble) num / den, density_pushed, NULL);
}

void
_gdk_x11_density_init_screen (GdkScreen *screen)
{
#ifdef HAVE_XFIXES
  GdkDisplayX11 *display_x11 = GDK_DISPLAY_X11 (gdk_screen_get_display (screen));

  /* follow the manager appearing and going away (compositing toggled or
   * the compositor restarted after we opened) */
  if (display_x11->have_xfixes)
    XFixesSelectSelectionInput (GDK_SCREEN_XDISPLAY (screen),
                                GDK_WINDOW_XID (gdk_screen_get_root_window (screen)),
                                density_manager_atom (screen),
                                XFixesSetSelectionOwnerNotifyMask |
                                XFixesSelectionWindowDestroyNotifyMask |
                                XFixesSelectionClientCloseNotifyMask);
#endif
}

void
_gdk_x11_density_property_notify (GdkWindow      *window,
                                  XPropertyEvent *xevent)
{
  if (xevent->atom == gdk_x11_get_xatom_by_name_for_display (gdk_drawable_get_display (window),
                                                             "_X_DENSITY_REQUESTED"))
    density_update_window (window);
}

void
_gdk_x11_density_owner_change (GdkScreen *screen,
                               Atom       selection)
{
  GList *toplevels, *l;

  if (selection != density_manager_atom (screen))
    return;

  /* with the manager gone, nobody samples the pixmaps any more: back to
   * 1/1 right away instead of leaving the window looking frozen (5.2);
   * with a new one, honour requests already on the windows */
  toplevels = gdk_screen_get_toplevel_windows (screen);
  for (l = toplevels; l; l = l->next)
    density_update_window (l->data);
  g_list_free (toplevels);
}
