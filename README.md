Ryzom Rendering Service
=========================

Compiling
---------

`docker-compose -f docker-compose-dev.yml build` will create container to compile rrs-server.

`docker-compose -f docker-compose-dev.yml run rrsdev` will clone, build, and copies rrs-server to app.

`rrsdev` container uses current folder as `/rrs` volume

Requirements
------------

These files needs to be copied under `app/data`

from `$HOME/.local/share/Ryzom/ryzom_live/data`
	characters_*.bnp
	construction.bnp
	data_common.bnp
	fauna_*.bnp
	leveldesign.bnp
	objects.bnp
	sfx.bnp

and from `$HOME/.local/share/Ryzom/ryzom_live/unpack`
	packedsheets.bnp

Running
-------

`docker-compose build` builds container for rrs-server to run in.

`docker-compose up -d` will start the container.

`rrs` container uses `app` folder as `/rrs` volume.

Copy `docker-compose.override.sample.yml` to `docker-compose.override.yml` and change IP/ports as needed.

Its now possible to open vnc connection to `127.0.0.1::25001` (vnc port on host is 25001).

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
