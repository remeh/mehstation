# mehstation


`mehstation` is a multi-launcher frontend fully driven by a gamepad/keyboard.

After having configured your platforms (Native, Emulators, ...) and the executables to launch on these platforms (binary, ROMs, ...), you will  have an interface usable to launch everything using a gamepad controller or a keyboard.

## Video

[![mehstation youtube video](http://img.youtube.com/vi/FxudzfhMTlU/0.jpg)](https://www.youtube.com/watch?v=FxudzfhMTlU)

## Screenshots

![Platforms list](http://c.remy.io/9YHrbWZm)
![Executables list](http://c.remy.io/oH0ki0Su)
![Configuration](http://c.remy.io/UWI1B1Ue)

## Features

  * Clean and beautiful interface
  * Two kind of view, configurable per platform:
    * Games view displaying screenshots, playing videos and showing description of executables
    * App mode focused on starting traditional apps (VLC, Firefox, Steam, etc.). Also more suitable for Raspberry Pi.
  * Multi-platform: Linux, Windows, (not tested on Mac yet).
  * Easy configuration and scraping with the dedicated app: [mehstation-config](https://github.com/remeh/mehstation-config)
  * Command-line configuration and scraping also available: [mehtadata](https://github.com/remeh/mehtadata)
  * Import configuration from EmulationStation
  * Favorite your executables, launch random executables, etc.
  * Automatic detection and visual mapping of gamepads.
  * Sounds (can be disabled in settings)

## Configuration

To configure your mehstation, the easiest solution is to use the dedicated graphical tool shipped with mehstation called [mehstation-config](https://github.com/remeh/mehstation-config). The usage of this configuration tool is documented in the [mehstation wiki](https://github.com/remeh/mehstation/wiki).

However, the configuration sometimes need to be done in command-line. This is why there is also [mehtadata](https://github.com/remeh/mehtadata), a configuration and scraping command-line tool .

## Developer infos

mehstation is developed in C with SDL2, glib, ffmpeg and SQLite3.
Please free to contribute and to send any pull requests, comments or issues.

If some of you want to contribute but don't know what to start with, [TODO](https://github.com/remeh/mehstation/blob/master/TODO.md) is a good start.

## How to compile

The dependencies are:

```
cmake
glib
SDL2, SDL2_ttf, SDL2_image
SQLite3
ffmpeg
```

Manual:

```
cmake .
make -j
sudo make install
```

You can now type `mehstation` to start the app.
Please create an issue if you have a problem compiling mehstation.

For further infos, see the [compilation chapters in the wiki](https://github.com/remeh/mehstation/wiki).


## License

mehstation, created by Rémy 'remeh' Mathieu, is under the terms of the MIT License.
