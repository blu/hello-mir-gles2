##hello-mir-gles2

This is a very basic computer graphics example using the EGL, GLESv2, and
Ubuntu MIR system tools.

It combines code from Joe Groff's OpenGL tutorial with code from Daniel 
van Vugt, of Canonical Inc, from Ubuntu MIR's demos.

*http://duriansoftware.com/joe/An-intro-to-modern-OpenGL.-Chapter-2:-Hello-World:-The-Slideshow.html
*http://bazaar.launchpad.net/~mir-team/mir/utopic/view/head:/examples/eglflash.c

This allows us to explore the following:

0. Handle GL context with EGL and Ubuntu's MIR, instead of X11's GLX.

1. GLESv2 instead of 'standard' OpenGL (EGL support for 'Real' OpenGL
   is not quite yet ready for prime-time as of writing, mid 2014)

2. Since GLESv2 deprecates alot of the same stuff as OpenGL3, this allows
   to test a kind of 'updated' non-deprecated GL system

3. A system without GLEW, since GLEW support for GLES and EGL is experimental
   and not 'standard' on most systems as of writing (mid 2014)

4. Write a MIR client program without dragging in all of the other 
   extraneous build files inside of Ubuntu's 'examples' build system

##How do I build?

You need to install MIR first.

*http://unity.ubuntu.com/mir/installing_prebuilt_on_pc.html

Then you need some other tools:

    sudo apt-get install build-essential pkg-config cmake

Now do this

    git clone http://github.com/donbright/hello-mir-gles2
    cd hello-mir-gles2
    mkdir bin
    cd bin
    cmake ..
    make

##MIR? how do you run this program under MIR?

See http://unity.ubuntu.com/mir/using_mir_on_pc.html

For a non-X11 do this:

* Log in to X11. Don't log out.
* Switch to VT1 (Ctrl+Alt+F1)
* $ sudo mir_demo_server_basic
* Switch to VT2 (Ctrl+Alt+F2)
* $ sudo chmod 777 /tmp/mir_socket
* $ sudo ./hello_mir_gles2
* Switch back to VT1 to watch program output (Ctrl+Alt+F1)
* Switch back to VT2, its probably corrupted now
* Switch back to X11 to clear screen (alt-right, alt-right, repeatedly)

##does it work properly?

Not quite. You can get an image on screen, but it doesn't transform like 
Joe's example in 'ordinary' OpenGL 3.0.

##But I ran ldd on the binary and it shows its linked to X??? I thought this was supposed to be X-free?

EGL shipping with Ubuntu currently is linked to X. (mid-2014). For Proof 
run this:

    for i in `find /usr/lib/ | grep EGL.so`;
      do echo "---------" $i; ldd $i;
    done

##see also

*http://duriansoftware.com/joe/An-intro-to-modern-OpenGL.-Chapter-2:-Hello-World:-The-Slideshow.html
*http://stackoverflow.com/questions/10383113/differences-between-glsl-and-glsl-es-2
*http://stackoverflow.com/questions/23853345/qt-desktop-shader-compilation-issue
*http://shnatsel.blogspot.com/2013/03/why-your-desktop-wont-be-running.html

##copyright license

Joe's code is under a "Do Whatever You Like" License.

The Ubuntu MIR code is under a GPL license as follows:


```
/*
 * Copyright © 2013 Canonical Ltd.
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

Most of the Ubuntu MIR stuff also includes the following note on Author:

* Author: Daniel van Vugt <daniel.van.vugt@canonical.com>


