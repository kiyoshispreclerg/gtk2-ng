#ifndef __GTK_PIXDATA_UTILS_H__
#define __GTK_PIXDATA_UTILS_H__

#include <gdk-pixbuf/gdk-pixbuf.h>

G_BEGIN_DECLS

GdkPixbuf *_gtk_pixbuf_new_from_pixdata_stream (const guint8  *stream,
                                                gsize          stream_length,
                                                GError       **error);

guint8    *_gtk_pixdata_serialize_from_pixbuf  (GdkPixbuf     *pixbuf,
                                                gsize         *length_p);

G_END_DECLS

#endif /* __GTK_PIXDATA_UTILS_H__ */
