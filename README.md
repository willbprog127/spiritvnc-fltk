__SpiritVNC - FLTK__

SpiritVNC - FLTK is an FLTK-based multi-view VNC client for Unix-like systems, including Linux, macOS and FreeBSD.
SpiritVNC features VNC-through-SSH, reverse (listening) VNC connections and timed scanning of
connected viewers.

2016-2021 Will Brokenbourgh
https://www.pismotek.com/brainout/

To God be the glory! :-D :heart:

SpiritVNC - FLTK has a 3-Clause BSD License

![SpiritVNC - FLTK screenshot](https://www.pismotek.com/media/spiritvnc-fltk-2020-11.png)



- - - -

__Dependencies__

You will need both the libraries and development packages of the following:
- fltk 1.3.4, 1.3.5 or newer
- libvncserver / libvncclient (if separate, you only need libvncclient)
- libssh2 (NOT libssh)

[MacPorts](https://www.macports.org) is highly recommended for installing dependencies on macOS.

The 'pkg-config' program must be installed for building

- - -

__Building__

SpiritVNC - FLTK now requires a compiler that supports C++11.

'cd' into the directory that has the Makefile, then issue the following command:

On most Linux distros and macOS:
```sh
$ make [debug]
$ sudo make install   # (don't use this command when building on macOS)
```

On FreeBSD, OpenBSD, OpenIndiana and others:
```sh
$ gmake [debug]
$ sudo gmake install
```
