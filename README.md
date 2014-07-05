Ryzom Rendering Service
=========================



Compiling
---------

This is meant to be compiled as sub-project.

Include 'ADD_SUBDIRECTORY(rrs-server)' to 'code/CMakeLists.txt' file and copy these sources there or use symlink.

cmake option WITH_BMSITE_RRS=ON must be enabled.

Example

	cmake <ryzom-sources-path>/code \
		-DCMAKE_BUILD_TYPE=Release -DWITH_BMSITE_RRS=ON \
		-DWITH_STATIC=ON -DWITH_STATIC_DRIVERS=ON \
		-DWITH_RYZOM_TOOLS=OFF -DWITH_RYZOM_SERVER=OFF -DWITH_RYZOM_CLIENT=OFF \
		-DWITH_NEL_TOOLS=OFF -DWITH_NEL_SAMPLES=OFF -DWITH_NEL_TESTS=OFF \
		-DWITH_LOGIC=OFF -DWITH_PACS=OFF -DWITH_SOUND=OFF -DWITH_GUI=OFF

Compiling in clean virtual machine (ubuntu 12.04 precise) requires packages

	build-essential cmake mercurial libxml2-dev libfreetype6-dev
	libpng12-0-dev libgl1-mesa-swx11-dev

And sources from [RyzomCore](https://bitbucket.org/ryzom/ryzomcore)

Running
-------

Running in virtual machine (ubuntu 12.04 precise) requires packages

	vnc4server libgl1-mesa-swx11 libxml2 libfreetype6 libpng12-0

If there is a need for window manager, then `blackbox` is nice and small.

Start vnc server normally ('vncserver' from command line). You can then modify
$HOME/.vnc/xstartup script to run 'rrs-server.sh'.

Example scripts to download .bnp files and run service are in 'bin' directory

Using
-----

There is php client avaiable from [here](https://github.com/nimetu/rrs_client.git).

License
--------
	Copyright (c) 2014 Meelis Mägi <nimetu@gmail.com>

	RyzomRenderingService is free software; you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	RyzomRenderingService is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with this program; if not, write to the Free Software Foundation,
	Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
