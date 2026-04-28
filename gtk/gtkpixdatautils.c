#include "config.h"

#include "gtkpixdatautils.h"
#include "gtkintl.h"

#include <string.h>

#define GTK_PIXDATA_MAGIC "GdkP"
#define GTK_PIXDATA_HEADER_LENGTH 24

#define GTK_PIXDATA_ENCODING_RAW       0x01000000
#define GTK_PIXDATA_ENCODING_RLE       0x02000000
#define GTK_PIXDATA_SAMPLE_WIDTH_8     0x00010000
#define GTK_PIXDATA_COLOR_TYPE_RGB     0x00000001
#define GTK_PIXDATA_COLOR_TYPE_RGBA    0x00000002

static guint32
pixdata_get_uint32 (const guint8 *data)
{
  return ((guint32) data[0] << 24) |
         ((guint32) data[1] << 16) |
         ((guint32) data[2] << 8) |
         ((guint32) data[3]);
}

static void
pixdata_put_uint32 (guint8  *data,
                    guint32  value)
{
  data[0] = (value >> 24) & 0xff;
  data[1] = (value >> 16) & 0xff;
  data[2] = (value >> 8) & 0xff;
  data[3] = value & 0xff;
}

static gboolean
pixdata_type_get_info (guint32   pixdata_type,
                       gboolean *rle_p,
                       gint     *bytes_per_pixel_p)
{
  guint32 encoding;
  guint32 sample_width;
  guint32 color_type;

  encoding = pixdata_type & 0xff000000;
  sample_width = pixdata_type & 0x00ff0000;
  color_type = pixdata_type & 0x000000ff;

  if (sample_width != GTK_PIXDATA_SAMPLE_WIDTH_8)
    return FALSE;

  if (color_type == GTK_PIXDATA_COLOR_TYPE_RGB)
    *bytes_per_pixel_p = 3;
  else if (color_type == GTK_PIXDATA_COLOR_TYPE_RGBA)
    *bytes_per_pixel_p = 4;
  else
    return FALSE;

  if (encoding == GTK_PIXDATA_ENCODING_RAW)
    *rle_p = FALSE;
  else if (encoding == GTK_PIXDATA_ENCODING_RLE)
    *rle_p = TRUE;
  else
    return FALSE;

  return TRUE;
}

static void
pixdata_set_error (GError **error)
{
  g_set_error_literal (error,
                       GDK_PIXBUF_ERROR,
                       GDK_PIXBUF_ERROR_CORRUPT_IMAGE,
                       _("Invalid embedded pixbuf data"));
}

GdkPixbuf *
_gtk_pixbuf_new_from_pixdata_stream (const guint8  *stream,
                                     gsize          stream_length,
                                     GError       **error)
{
  guint32 length;
  guint32 pixdata_type;
  guint32 rowstride;
  guint32 width;
  guint32 height;
  gint bytes_per_pixel;
  gboolean rle;
  gsize pixel_data_size;
  const guint8 *in;
  const guint8 *in_end;
  guint8 *pixels;
  guint8 *out;
  guint8 *out_end;

  g_return_val_if_fail (stream != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (stream_length < GTK_PIXDATA_HEADER_LENGTH)
    goto invalid;

  if (memcmp (stream, GTK_PIXDATA_MAGIC, 4) != 0)
    goto invalid;

  length = pixdata_get_uint32 (stream + 4);
  pixdata_type = pixdata_get_uint32 (stream + 8);
  rowstride = pixdata_get_uint32 (stream + 12);
  width = pixdata_get_uint32 (stream + 16);
  height = pixdata_get_uint32 (stream + 20);

  if (length < GTK_PIXDATA_HEADER_LENGTH || length > stream_length)
    goto invalid;

  if (!pixdata_type_get_info (pixdata_type, &rle, &bytes_per_pixel))
    goto invalid;

  if (width == 0 || height == 0 || rowstride == 0)
    goto invalid;

  if (width > G_MAXUINT / bytes_per_pixel)
    goto invalid;

  if (rowstride < width * bytes_per_pixel)
    goto invalid;

  if (height > G_MAXSIZE / rowstride)
    goto invalid;

  pixel_data_size = (gsize) rowstride * height;

  in = stream + GTK_PIXDATA_HEADER_LENGTH;
  in_end = stream + length;

  if (!rle)
    {
      if ((gsize) (in_end - in) != pixel_data_size)
        goto invalid;

      pixels = g_malloc (pixel_data_size);
      memcpy (pixels, in, pixel_data_size);
    }
  else
    {
      pixels = g_malloc (pixel_data_size);
      out = pixels;
      out_end = pixels + pixel_data_size;

      while (in < in_end && out < out_end)
        {
          guint run_length;
          gboolean repeated;

          repeated = (*in & 0x80) != 0;
          run_length = *in & 0x7f;
          in++;

          if (run_length == 0)
            {
              g_free (pixels);
              goto invalid;
            }

          if (repeated)
            {
              guint i;

              if (in_end - in < bytes_per_pixel ||
                  (gsize) (out_end - out) < (gsize) run_length * bytes_per_pixel)
                {
                  g_free (pixels);
                  goto invalid;
                }

              for (i = 0; i < run_length; i++)
                {
                  memcpy (out, in, bytes_per_pixel);
                  out += bytes_per_pixel;
                }

              in += bytes_per_pixel;
            }
          else
            {
              gsize run_size;

              run_size = (gsize) run_length * bytes_per_pixel;

              if ((gsize) (in_end - in) < run_size ||
                  (gsize) (out_end - out) < run_size)
                {
                  g_free (pixels);
                  goto invalid;
                }

              memcpy (out, in, run_size);
              in += run_size;
              out += run_size;
            }
        }

      if (out != out_end)
        {
          g_free (pixels);
          goto invalid;
        }
    }

  return gdk_pixbuf_new_from_data (pixels,
                                   GDK_COLORSPACE_RGB,
                                   bytes_per_pixel == 4,
                                   8,
                                   width,
                                   height,
                                   rowstride,
                                   (GdkPixbufDestroyNotify) g_free,
                                   NULL);

 invalid:
  pixdata_set_error (error);
  return NULL;
}

GdkPixbuf *
_gtk_pixbuf_new_from_inline_pixdata (const guint8  *stream,
                                     GError       **error)
{
  guint32 length;

  g_return_val_if_fail (stream != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (memcmp (stream, GTK_PIXDATA_MAGIC, 4) != 0)
    {
      pixdata_set_error (error);
      return NULL;
    }

  length = pixdata_get_uint32 (stream + 4);

  return _gtk_pixbuf_new_from_pixdata_stream (stream, length, error);
}

guint8 *
_gtk_pixdata_serialize_from_pixbuf (GdkPixbuf *pixbuf,
                                    gsize     *length_p)
{
  guint32 pixdata_type;
  gint width;
  gint height;
  gint rowstride;
  gint n_channels;
  gsize pixel_data_size;
  gsize total_length;
  guint8 *data;

  g_return_val_if_fail (GDK_IS_PIXBUF (pixbuf), NULL);
  g_return_val_if_fail (length_p != NULL, NULL);

  width = gdk_pixbuf_get_width (pixbuf);
  height = gdk_pixbuf_get_height (pixbuf);
  rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  n_channels = gdk_pixbuf_get_n_channels (pixbuf);

  if (n_channels != 3 && n_channels != 4)
    return NULL;

  if (height < 0 || rowstride < 0)
    return NULL;

  if ((gsize) height > G_MAXSIZE / (gsize) rowstride)
    return NULL;

  pixel_data_size = (gsize) rowstride * (gsize) height;
  total_length = GTK_PIXDATA_HEADER_LENGTH + pixel_data_size;

  if (total_length > G_MAXUINT32)
    return NULL;

  pixdata_type = GTK_PIXDATA_ENCODING_RAW |
                 GTK_PIXDATA_SAMPLE_WIDTH_8 |
                 (n_channels == 4 ? GTK_PIXDATA_COLOR_TYPE_RGBA : GTK_PIXDATA_COLOR_TYPE_RGB);

  data = g_malloc (total_length);

  memcpy (data, GTK_PIXDATA_MAGIC, 4);
  pixdata_put_uint32 (data + 4, total_length);
  pixdata_put_uint32 (data + 8, pixdata_type);
  pixdata_put_uint32 (data + 12, rowstride);
  pixdata_put_uint32 (data + 16, width);
  pixdata_put_uint32 (data + 20, height);

  memcpy (data + GTK_PIXDATA_HEADER_LENGTH,
          gdk_pixbuf_get_pixels (pixbuf),
          pixel_data_size);

  *length_p = total_length;

  return data;
}
