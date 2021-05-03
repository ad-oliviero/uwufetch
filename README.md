# UwUFetch

A meme system info tool for (almost) all your Linux/Unix-based systems, based on nyan/UwU trend on r/linuxmasterrace.

## Currently supported distros

#### Full support (Both Ascii art + images are provided for the given distribution)

Nyalpine, Nyarch Linuwu, Nyartix Linuwu, Debinyan, endOwO, Fedowa, GentOwO, GnUwU gUwUix, Miwint, Myanjawo, OwOpenSUSE, Pop OwOs, RaspNyan, UwUntu, and OwOid; Plus Nyandroid.

#### Partial support (Either no Ascii art, or no image is provided)

KDE NeOwOn, nixOwOs, Swackwawe, sOwOlus; Plus FweeBSD, OwOpenBSD and macOwOS; Plus WinyandOwOws.

## Building and installation

#### Requisites

[viu](https://github.com/atanunq/viu) to use images instead of ascii art.

[lshw](https://github.com/lyonel/lshw) to get gpu info.

##### Via package manager

Right now, the package is only available on the AUR:

[![uwufetch](https://img.shields.io/aur/version/uwufetch?color=1793d1&label=uwufetch&logo=arch-linux&style=for-the-badge)](https://aur.archlinux.org/packages/uwufetch/)

[![uwufetch-git](https://img.shields.io/aur/version/uwufetch-git?color=1793d1&label=uwufetch-git&logo=arch-linux&style=for-the-badge)](https://aur.archlinux.org/packages/uwufetch-git/)

##### Via source

Building requisites:

-   Make
-   A C compiler
-   `pandoc` to compile man pages

To install UwUfetch from the source, type these commands in the terminal:

```shell
git clone https://github.com/TheDarkBug/uwufetch.git
cd uwufetch
make build
make man
sudo make install
```

To uninstall:

```shell
cd uwufetch
sudo make uninstall
```

##### Make options:

```shell
make build              # builds uwufetch
make man                # builds the manpage (requires pandoc)
make debug              # use for debug
make install            # installs uwufetch (needs root permissons)
make uninstall          # uninstalls uwufetch (needs root permissons)
make termux             # build and install for termux
make termux_uninstall   # uninstall for termux
```

## Images and copyright info

### How to use images

First at all you need `viu`, to install it follow the [guide](https://github.com/atanunq/viu#installation).
Images are working in almost every terminal, for a better experience i recommend [kitty](https://github.com/kovidgoyal/kitty)

### For copyright and logos info

<font size=2>[COPYRIGHT.md](https://github.com/TheDarkBug/uwufetch/tree/main/res/COPYRIGHT.md)</font>

## License

This program is provided under the [GPL-3.0 License](https://github.com/TheDarkBug/uwufetch/LICENSE).

# Contributing

All kind of contribution are welcome, but before contributing please read [CONTRIBUTING.md](https://github.com/TheDarkBug/uwufetch/blob/main/CONTRIBUTING.md).
