# About

[![Test](https://github.com/thiagokokada/python-dpms/workflows/Test/badge.svg)](https://github.com/thiagokokada/python-dpms)

This are Python bindings to the **DPMS X11** extension module for controlling
your monitor power savings state.

The DPMS interface lets you control the power level of your monitor
(`On`, `Standby`, `Suspend`, or `Off`).  It is a simple interface that lets you
get/set timeouts of inactivity to enter these states or you can force it to
enter any of the states.

See `example.py` on how to use it. This file should be sufficient on how to use
it. It first shows how to query all the different settings and then shows how
to set them.

See also `man xset` for a program that lets you modify the DPMS state from the
command line.

These are bindings to the DPMS X11 extension module and as such only work from
within an X11 session. They do not work from a console outside of X11.

# Install

Install libXext and python development packages installed. For example, on
RedHat/Fedora:

    # yum install libXext-devel python-devel

On Ubuntu:

    # apt-get install libxext6-dev python-dev

On Arch Linux:

    # pacman -S libxext python

On NixOS, use the provided `shell.nix` file:

    # This will setup everything, no need to build/install later
    $ nix-shell

Then run:

    $ python setup.py build

Then as root, run:

    # python setup.py install

If you prefer, you can also build it inside a virtualenv, without root:

    $ virtualenv venv
    $ source venv/bin/activate
    $ python setup.py install

That's it. Now run:

    $ python example.py

to test it out. Also try the `turnoff.py` script:

    $ python turnoff.py

It should turn off your monitor after a 1 second wait. Press any key to wake it
back up.

# Compatibility

This is a fork of [python-dpms](https://github.com/dirjud/python-dpms),
porting it to Python 3. It also include some bugfixes.
