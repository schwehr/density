#!/bin/bash

#     This program is free software; you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation; either version 2 of the License, or
#     (at your option) any later version.

#     This program is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.

#     You should have received a copy of the GNU General Public License
#     along with this program; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

# Pull apart rosenbaum-ams-stripped into logical groups so that we

export PATH=${PATH}:.


if [ ! -f  g1-fluidized.dat ]; then
   splitdepth.py 19.5 rosenbaum-ams-stripped.dat g1-m.dat g2-5.dat
   splitdepth.py 30 g2-5.dat g2-m.dat g3-5.dat
   splitdepth.py 80 g3-5.dat g3-m.dat g4-5.dat
   splitdepth.py 104 g4-5.dat g4-m.dat g5-m.dat
   mv g1-m.dat g1-fluidized.dat # 19.0-12.5 m
   mv g2-m.dat g2-undeformed.dat # 27.7-20.4m
   mv g3-m.dat g3-sheared.dat # 85.2-78.1m
   mv g4-m.dat g4-little-def.dat # 103.6-99.6m # same as minimally?
   mv g5-m.dat g5-intermediate.dat # 119.7-115.5m
   rm -f g[0-5]-[0-5].dat
fi
