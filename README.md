lifx-api
========

C++ API/SDK for Lifx bulbs

This is a simple C++ system in its early stages that allows the user to interact with Lifx bulbs.

Current features:
* bulb discovery
* getting basic info (name, color)
* setting color and brightness

Issues:
* assumes little-endian byte ordering
* incomplete

TODO:
* support for other OS
* refactor to move away from MSVC towards a generic build system
* create an architecture that allows constant connection to the bulbs and immediate sending of data (or cache bulb addresses)

Protocol info taken from:
https://github.com/magicmonkey/lifxjs/blob/master/Protocol.md
Some of the information on that Protocol site seems inaccurate, I have changed these in my implementation.
