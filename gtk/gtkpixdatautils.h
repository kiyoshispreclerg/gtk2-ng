#ifndef __GTK_PIXDATA_UTILS_H__
#define __GTK_PIXDATA_UTILS_H__

#include <gdk-pixbuf/gdk-pixbuf.h>

G_BEGIN_DECLS

GdkPixbuf *_gtk_pixbuf_new_from_pixdata_stream (const guint8  *stream,
                                                gsize          stream_length,
                                                GError       **error);

G_END_DECLS

#endif /* __GTK_PIXDATA_UTILS_H__ */
