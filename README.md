# baker – P.I. Engineering X-Keys to OSC translator

**baker** receives button presses & releases from a [P.I. Engineering
X-Keys](https://xkeys.com/xkeys.html) keypad and translates them into OSC
messages.

Buttons can be configured to be double-press, toggle or be part of one or more
groups.

Each keypad has a unit ID (uid) associated with it, which is factory preset to
`0`. The uid can be changed and is used to distinguish between different
keypads, as well as to configure buttons for each one.

> NB: You can use the `set-uid` program to change keypad's uid.

Button configuration files are located in the `/etc/baker` directory (can be
overriden with `--conf-dir`) and are named after the keypad uid followed by the
`.conf` extension. In other words, to configure keypad with uid `0` – create a
file named `0.conf`, for keypad with uid `42` – create `42.conf`, and so on.

The configuration file consists of zero or more of the following lines:

```ini
double-press = <button> <button> ...
toggle = <button> <button> ...
group <id> = <button> <button> ...
```

- The `double-press` command followed by the equal sign (`=`) and a list of
  white-space separated button indices instructs **baker** to configure those
  buttons as double-press.

  When pressed once, a double-press button starts blinking. If pressed again while
  blinking, the button emits the `press` event followed by the `release` event.

- The `toggle` command followed by the equal sign (`=`) and a list of button
  indices instructs **baker** to configure the buttons as toggle.

  When pressed, a toggle button emits the `press` event and remains in active
  state. When pressed again while in active state, the button is "deactivated" and
  emits the `release` event.

- The `group` command followed by a group id, the equal sign (`=`) and a list of
  button indices instructs **baker** to configure the buttons as belonging to the
  same group.

  When a group button is pressed, it becomes active and emits the `press` event.
  If another button was already active in the group, it is "deactivated" first,
  emitting the `release` event. Only one button can be active in each group.

On each `press` and `release` event **baker** sends one of the following OSC
messages:

```
/remote/pie/<uid>/press <button>
/remote/pie/<uid>/release <button>
```

By default, **baker** sends messages to an OSC server on IP address `127.0.0.1`
and port `6260`. These can be changed with the `--address` and `--port` options
respectively.

In order to set these options, as well as the `--conf-dir` option, you can
override them in the `baker@.service` file. For example:

```shell
sudo systemctl edit baker@.service
```

and add the following lines:

```ini
[Service]
Environment="args=--address=10.0.42.123 --port=4567 --conf-dir=/foo/bar/baz"
```

## Installation

### Binary

Requires [libosc++](https://github.com/dimitry-ishenko-cpp/liboscpp) >= 1.0.

Debian/Ubuntu/etc:

```shell
$ p=baker
$ v=0.0
$ wget https://github.com/dimitry-ishenko-casparcg/${p}/releases/download/v${v}/${p}_${v}_amd64.deb
$ sudo apt install ./${p}_${v}_amd64.deb
```

RaspberryPi:

```shell
$ p=baker
$ v=0.0
$ wget https://github.com/dimitry-ishenko-casparcg/${p}/releases/download/v${v}/${p}_${v}_armhf.deb
$ sudo apt install ./${p}_${v}_armhf.deb
```

### From source

Stable version (requires [CMake](https://cmake.org/) >= 3.1,
[asio](https://think-async.com/Asio/) and
[libosc++-dev](https://github.com/dimitry-ishenko-cpp/liboscpp) >= 1.0):

```shell
$ p=baker
$ v=0.0
$ wget https://github.com/dimitry-ishenko-casparcg/${p}/releases/download/v${v}/${p}-${v}.tar.bz2
$ tar xjf ${p}-${v}.tar.bz2
$ mkdir ${p}-${v}/build
$ cd ${p}-${v}/build
$ cmake ..
$ make
$ sudo make install
```

Latest master (requires [git](https://git-scm.com/),
[CMake](https://cmake.org/) >= 3.1, [asio](https://think-async.com/Asio/) and
[libosc++-dev](https://github.com/dimitry-ishenko-cpp/liboscpp) >= 1.0):

```shell
$ p=baker
$ git clone --recursive https://github.com/dimitry-ishenko-casparcg/${p}.git
$ mkdir ${p}/build
$ cd ${p}/build
$ cmake ..
$ make
$ sudo make install
```

## Authors

* **Dimitry Ishenko** - dimitry (dot) ishenko (at) (gee) mail (dot) com

## License

This project is distributed under the GNU GPL license. See the
[LICENSE.md](LICENSE.md) file for details.