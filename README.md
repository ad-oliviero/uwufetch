# UwUFetch

A meme system info tool for (almost) all your Linux/Unix-based systems, based on the nyan/UwU trend on r/linuxmasterrace.

## Currently supported distros

### Full support (Both ASCII art + images are provided for the given distribution)

Nyalpine, Nyarch Linuwu, ArcOwO, Nyartix Linuwu, Debinyan, endevaOwO, Fedowa, GentOwO, GnUwU gUwUix, Miwint, Myanjawo, OwOpenSUSE, Pop OwOs, RaspNyan, Swackwawe, sOwOlus, UwUntu, and OwOid; Plus Nyandroid.

### Partial support (Either no ASCII art, or no image is provided)

AmogOwOS, KDE NeOwOn, nixOwOs, xuwulinux; Plus FweeBSD, OwOpenBSD and macOwOS; Plus WinyandOwOws.

## Building and installation

### Requisites

- [freecolor](http://www.rkeene.org/oss/freecolor/) to get ram usage on FreeBSD.

- [viu](https://github.com/atanunq/viu) (optional) to use images instead of ascii art (see [How to use images](#how-to-use-images) below).

- [lshw](https://github.com/lyonel/lshw) (optional) for better accuracy on GPU info.

### Via package manager

Right now, the package is only available on the AUR:

[![uwufetch](https://img.shields.io/aur/version/uwufetch?color=1793d1&label=uwufetch&logo=arch-linux&style=for-the-badge)](https://aur.archlinux.org/packages/uwufetch/)

[![uwufetch-git](https://img.shields.io/aur/version/uwufetch-git?color=1793d1&label=uwufetch-git&logo=arch-linux&style=for-the-badge)](https://aur.archlinux.org/packages/uwufetch-git/)

### From source

Build requisites:

- Make
- A C compiler

To install UwUfetch from the source, type these commands in the terminal:

```shell
git clone https://github.com/TheDarkBug/uwufetch.git
cd uwufetch
make build
sudo make install       # for termux, use `make termux`
```

To uninstall:

```shell
cd uwufetch
sudo make uninstall
```

#### Available Make targets

```shell
make build              # builds uwufetch
make debug              # use for debug
make install            # installs uwufetch (needs root permissons)
make uninstall          # uninstalls uwufetch (needs root permissons)
make termux             # build and install for termux
make termux_uninstall   # uninstall for termux
```

## Images and copyright info

### How to use images

First of all, you will need `viu`, which you can install by following the [guide](https://github.com/atanunq/viu#installation).

`viu` supports [kitty](https://github.com/kovidgoyal/kitty) and [iTerm](https://iterm2.com/)'s image protocols.
If not supported by the current terminal, `viu` uses the fallback Unicode half-block mode (images will look "blocky"), that is the case in many terminal emulators (gnome-terminal, Konsole, etc.). See also: [viu's README](https://github.com/atanunq/viu#description).

### For copyright and logos info

See [COPYRIGHT.md](/res/COPYRIGHT.md).

## License

This program is provided under the [GPL-3.0 License](/LICENSE).

## Contributing

All kinds of contributions are welcome, but before contributing please read [CONTRIBUTING.md](/CONTRIBUTING.md).
