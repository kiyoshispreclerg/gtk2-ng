# GTK2-NG — Modern Community Fork of GTK+ 2.24

**Unofficial, community-maintained fork keeping GTK2 alive and usable on modern 64-bit Linux systems.**

This fork focuses on modern compatibility while preserving stability, performance, ABI compatibility, and original behavior as much as possible. X11-first, no forced Wayland migration.

Backed by the **devuanusers.com** community.

## Highlights

- Conservative GTK+ 2.24 maintenance with ABI compatibility as a priority
- Existing GTK2 applications should continue to run without recompilation
- X11-first approach with no forced Wayland migration
- Modern build-system cleanup for current Autoconf, Automake, Libtool and gtk-doc
- Compatibility fixes for current GCC and Clang versions
- Selected safe patches from AUR, BSDs, Gentoo, Fedora and GTK2 maintenance forks
- Internal Y2038-related time-handling improvements where ABI-safe
- Modernized private-data handling in many GTK/GDK classes
- Fixed many compiler and deprecation warnings without removing features
- Improved GtkFileChooser with optional icon view
- Classic list view remains available
- Better icon handling and reduced unnecessary overhead in the file chooser
- Preserved GTK2 theming, theme engines, input modules and print backends
- Preserved legacy configuration and module lookup behaviour
- Continued focus on performance, low overhead and responsiveness
- Community-driven maintenance backed by devuanusers.com

## Roadmap

- Continue removing deprecated internal API usage where safe
- Improve file chooser responsiveness for large and network directories
- Add best-effort thumbnail generation and optional preview support
- Improve compatibility with traditional GTK2 themes and engines
- Add XDG-aware config, theme and icon lookup paths while keeping legacy paths
- Review Ardour's YTK GTK2 fork for portable, ABI-safe improvements
- Review further GTK2 maintenance forks for useful fixes
- Preserve CUPS/PPD printing workflows and add libppd support for newer CUPS
- Keep GTK2 useful on modern Unix-like systems without breaking existing apps
- Choose a better long-term project name

## Build Instructions (Linux)

### Safe test install (strongly recommended)
```bash
autoreconf -fvi
./configure --prefix=/tmp/gtk2-ng-test --with-xinput=yes
make -j$(nproc)
make install
```

### Run apps against the test build
```bash
export GTK2_TESTDIR=/tmp/gtk2-ng-test

export LD_LIBRARY_PATH="$GTK2_TESTDIR/lib:$LD_LIBRARY_PATH"
export PKG_CONFIG_PATH="$GTK2_TESTDIR/lib/pkgconfig:$PKG_CONFIG_PATH"
export GTK_EXE_PREFIX="$GTK2_TESTDIR"
export GTK_DATA_PREFIX="$GTK2_TESTDIR"
export GTK_PATH="$GTK2_TESTDIR/lib/gtk-2.0"

leafpad   # or any other GTK2 app
```

### System-wide install
```bash
autoreconf -fvi
./configure --prefix=/usr --sysconfdir=/etc --localstatedir=/var --with-xinput=yes
make -j$(nproc)
sudo make install
```

## Dependencies (Debian/Devuan/Ubuntu)
```bash
sudo apt install \
  build-essential git \
  autoconf automake libtool pkg-config gtk-doc-tools gobject-introspection \
  libglib2.0-dev libpango1.0-dev libatk1.0-dev libcairo2-dev libgdk-pixbuf-2.0-dev \
  libfontconfig1-dev libfreetype-dev libharfbuzz-dev libfribidi-dev libpng-dev \
  libx11-dev libxext-dev libxrender-dev libxrandr-dev libxcursor-dev \
  libxinerama-dev libxi-dev libxfixes-dev libxcomposite-dev libxdamage-dev \
  libcups2-dev gettext autopoint
```

## Dependencies (Arch/Artix/Manjaro)
```bash
sudo pacman -S --needed \
  base-devel git \
  autoconf automake libtool pkgconf make gcc binutils \
  gtk-doc gobject-introspection \
  glib2 glib2-devel \
  pango cairo atk gdk-pixbuf2 \
  fontconfig freetype2 harfbuzz fribidi libpng pixman \
  libx11 libxext libxrender libxrandr libxcursor libxinerama libxi libxfixes \
  libxcomposite libxdamage libxft \
  cups
```

## Dependencies (Gentoo)
```bash
emerge --ask --oneshot \
  dev-vcs/git \
  sys-devel/gcc sys-devel/binutils dev-build/make \
  dev-build/autoconf dev-build/automake dev-build/libtool \
  dev-util/pkgconf dev-util/ccache \
  dev-util/gtk-doc dev-libs/gobject-introspection \
  dev-libs/glib dev-libs/atk \
  x11-libs/pango x11-libs/cairo x11-libs/gdk-pixbuf \
  media-libs/fontconfig media-libs/freetype \
  media-libs/harfbuzz dev-libs/fribidi media-libs/libpng x11-libs/pixman \
  x11-libs/libX11 x11-libs/libXext x11-libs/libXrender x11-libs/libXrandr \
  x11-libs/libXcursor x11-libs/libXinerama x11-libs/libXi x11-libs/libXfixes \
  net-print/cups
```

## GTK2-ng requires Cairo with Xlib support. If configure reports that cairo-xlib is missing, enable the X USE flag for Cairo (Gentoo)
```bash
echo 'x11-libs/cairo X' >> /etc/portage/package.use/gtk2-ng
emerge --ask --oneshot x11-libs/cairo
```

## If Pango or Freetype dependency resolution requires Harfbuzz support, enable it for Freetype: (Gentoo)
```bash
echo 'media-libs/freetype harfbuzz' >> /etc/portage/package.use/gtk2-ng
emerge --ask --oneshot media-libs/freetype x11-libs/pango
```

## How to Contribute
Join the main discussion here:  
https://devuanusers.com/thread-gtk2-revival-thread--80

Test apps, report issues, submit patches!

## License
LGPL v2.1 or later (same as original GTK+)
