# GTK2-NG — Modern Community Fork of GTK+ 2.24

**Unofficial, community-maintained fork keeping GTK2 alive and usable on modern 64-bit Linux systems.**

This fork focuses on modern compatibility while preserving stability, performance, ABI compatibility, and original behavior as much as possible. X11-first, no forced Wayland migration.

Backed by the **devuanusers.com** community.

## What has already been done
- Added the two GTK2 patches currently used by the AUR maintainers
- Implemented several modernizations from stefan11111’s GTK2 fork
- Fixed all compiler warnings with modern toolchains
- Made GTK2 compile cleanly with GCC 14/15 and Clang 21
- Tested on Devuan, Arch, and Gentoo
- Improved the file chooser:
  - Classic list view still available
  - New icon view added
  - Easy switching between list and icon view
  - Better thumbnail and icon handling
  - Keeps responsiveness and avoids unnecessary overhead
- Conservative approach — no ABI breaks where possible

## Future plans
- Backport further improvements from stefan11111’s GTK2 fork (Xlibre contributor)
- Backport useful new features from Ardour’s GTK2 fork (YTK) without breaking ABI
- Continue fixing bugs and modern compiler issues
- Improve performance and responsiveness
- Further file chooser improvements:
  - Best-effort thumbnail generation for icon view
  - Asynchronous loading for large/network folders
  - Optional preview panel
- Find a better project name
- Add libppd support for legacy PPD-based printer options with newer CUPS

## Build Instructions (Linux)

### Safe test install (strongly recommended)
```bash
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
./configure --prefix=/usr --sysconfdir=/etc --localstatedir=/var --with-xinput=yes
make -j$(nproc)
sudo make install
```

## Dependencies (Debian/Devuan/Ubuntu)
```bash
sudo apt install build-essential autoconf automake libtool pkg-config \
    libglib2.0-dev libpango1.0-dev libatk1.0-dev libcairo2-dev libfontconfig1-dev \
    libx11-dev libxext-dev libxrender-dev libxinerama-dev libxi-dev libxcursor-dev \
    libxrandr-dev libxfixes-dev libxcomposite-dev libxdamage-dev
```

(Adjust for Arch, Gentoo, etc.)

## How to Contribute
Join the main discussion here:  
https://devuanusers.com/thread-gtk2-revival-thread--80

Test apps, report issues, submit patches!

## License
LGPL v2.1 or later (same as original GTK+)
