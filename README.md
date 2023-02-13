# UwUFetch

A meme system info tool for (almost) all your Linux/Unix-based systems, based on the nyan/UwU trend on r/linuxmasterrace.


## Contributing

All kinds of contributions are welcome, but before contributing **please** read [CONTRIBUTING.md](/CONTRIBUTING.md).

## Currently supported distros

### Full support (Both ASCII art + images are provided for the given distribution)

AmogOwOS, Nyalpine, Nyarch Linuwu, ArcOwO, Nyartix Linuwu, Debinyan, endevaOwO, Fedowa, GentOwO, GnUwU gUwUix, Miwint, Myanjawo, OwOpenSUSE, Pop OwOs, RaspNyan, Swackwawe, sOwOlus, UwUntu, and OwOid; Plus Nyandroid.

### Partial support (Either no ASCII art, or no image is provided)

Dewepyn, FemboyOWOS, KDE NeOwOn, nixOwOs, xuwulinux, Wocky Linuwu; Plus FweeBSD, OwOpenBSD, macOwOS and iOwOS; Plus WinyandOwOws.

## Building and installation

[![Build and Run](https://github.com/TheDarkBug/uwufetch/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/TheDarkBug/uwufetch/actions/workflows/c-cpp.yml)

### Requisites

- [freecolor](http://www.rkeene.org/oss/freecolor/) to get ram usage on FreeBSD.

- [musl libc](https://www.musl-libc.org/) if you are on a non-gnu system (such as [alpine linux](https://pkgs.alpinelinux.org/package/edge/main/x86_64/musl-dev))

- [xwininfo](https://github.com/freedesktop/xorg-xwininfo) to get screen resolution.

- [viu](https://github.com/atanunq/viu) (optional) to use images instead of ascii art (see [How to use images](#how-to-use-images) below).

- [lshw](https://github.com/lyonel/lshw) (optional) for better accuracy on GPU info.

### Via package manager

Arch (Official Repos)

[![uwufetch](https://img.shields.io/archlinux/v/community/x86_64/uwufetch?label=uwufetch&logo=arch-linux&style=for-the-badge)](https://archlinux.org/packages/community/x86_64/uwufetch/)


From the AUR

[![uwufetch-git](https://img.shields.io/aur/version/uwufetch-git?color=1793d1&label=uwufetch-git&logo=arch-linux&style=for-the-badge)](https://aur.archlinux.org/packages/uwufetch-git/)

From [Pacstall](https://github.com/pacstall/pacstall#installing)

```bash
pacstall -I uwufetch
```

### From source

Build requisites:

- Make
- A C compiler
  - A iOS patched SDK (if you build UwUfetch under iOS device)

To install UwUfetch from the source, type these commands in the terminal:

```shell
git clone https://github.com/TheDarkBug/uwufetch.git
cd uwufetch
make build # add "CFLAGS+=-D__IPHONE__" if you are building for iOS
sudo make install
```

To uninstall:

```shell
cd uwufetch
sudo make uninstall
```

#### Available Make targets

```shell
make build              # builds uwufetch and libfetch
make lib                # builds only libfetch
make debug              # use for debug
make install            # installs uwufetch (needs root permissons)
make uninstall          # uninstalls uwufetch (needs root permissons)
make clean              # removes all build output
make man                # compiles man page
make man_debug          # compiles man page and shows 'man' output
```

## Images and copyright info

### How to use images

Notice: images are currently disabled under iOS due to lack of a one command in UwUfetch code

First of all, you will need `viu`, which you can install by following the [guide](https://github.com/atanunq/viu#installation).

`viu` supports [kitty](https://github.com/kovidgoyal/kitty) and [iTerm](https://iterm2.com/)'s image protocols.
If not supported by the current terminal, `viu` uses the fallback Unicode half-block mode (images will look "blocky"), that is the case in many terminal emulators (gnome-terminal, Konsole, etc.). See also: [viu's README](https://github.com/atanunq/viu#description).

## LibFetch

### How to use

The first thing needed is to compile the library ([same instructions as the simple binary](https://github.com/TheDarkBug/uwufetch#from-source)).
Now you can use the library just as in the example:

#### **`superfetch.c`**

```c
#include <fetch.h>
#include <stdio.h>

int main() {
    printf("%s\n", get_info().cpu_model);
}
```

And then compile and run with

```bash
$ gcc superfetch.c -lfetch -o superfetch
$ ./superfetch

Intel(R) Core(TM) i7-1065G7 CPU @ 1.30GHz
```

## Issues

### `MOWODEL` showing `To Be Filled By O.E.M.`

This happens when your computer hasn't had any [OEM info filled in](https://www.investopedia.com/terms/o/oem.asp) (habitually by the manufacturer).
While you could fill it yourself with your own custom info too, you can also disable the part of uwufetch which display this line.
Edit [`.config/uwufetch/config`] and add `host=false`.

### For copyright and logos info

See [COPYRIGHT.md](/res/COPYRIGHT.md).

## License

This program is provided under the [GPL-3.0 License](/LICENSE).
