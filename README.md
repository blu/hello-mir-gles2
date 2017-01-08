##hello-mir-gles2

This is a rudimentary primer of GLES2 via EGL on Ubuntu Mir. It's also an example of self-hosted development on Ubuntu Touch.

It derives from [Don Bright's Mir/GLES2 adaptation](https://github.com/donbright/hello-mir-gles2) of [Joe Groff's OpenGL tutorial](http://duriansoftware.com/joe/An-intro-to-modern-OpenGL.-Chapter-2:-Hello-World:-The-Slideshow.html), with [Mir/EGL code from Daniel van Vugt](http://bazaar.launchpad.net/~mir-team/mir/utopic/view/head:/examples/eglflash.c), of Canonical Inc.

This primer aims to:

* Do GLES2 via EGL on Ubuntu Mir in the most minimalistic way possible - no QtMir or any other toolkits.
* Explore the possibilities for self-hosted GLES2 development on Ubuntu Touch

##How to build on Ubuntu Touch

On a Ubuntu Touch box in Developer mode:

	$ sudo android-gadget-service enable writable
	$ sudo apt-get update
	$ sudo apt-get install build-essential
	$ sudo apt-get install libmirclient-dev

The build itself:

	$ git clone https://github.com/blu/hello-mir-gles2.git
	$ cd hello-mir-gles2
	$ ./build.sh
	$ sudo reboot

Voila, you should have a hello-gles icon in your Apps pane; the app itself sits under:

	/opt/click.ubuntu.com/hello-gles/

The primer runs in both normal and desktop mode, and loops endlessly. To stop the program press 'ESC' on a kbd (if you have one attached/paired) or left-side swipe to bring up Launcher and quit from there.
The system restart is necessary only the first time - subsequent builds don't need it - just build and then launch from the icon. Logs from a run session can be seen under:

	~/.cache/upstart/application-click-hello-gles_hello-gles_0.1.log

##Copyrights & licenses

Joe Groff's code is under a "Do Whatever You Like" license, and so is my part; I can only assume Don Bright shares that; Daniel van Vugt's code is under GPL3, though, which means the entire primer is effectively GPL3:

```
/*
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
```
