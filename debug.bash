#!/bin/bash

# Source this file to get DebugEcho and more

# $Revision$  $Author$  $Date$

##############################################################################
#     Copyright (C) 2004  Kurt Schwehr
#
#     This program is free software; you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation; either version 2 of the License, or
#     (at your option) any later version.
#
#     This program is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
#
#     You should have received a copy of the GNU General Public License
#     along with this program; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
###############################################################################


######################################################################
# Debugging
######################################################################

declare -ri EXIT_FAILURE=1
declare -ri EXIT_SUCCESS=0


declare -ri TERSE=1
declare -ri TRACE=4
declare -ri VERBOSE=8
declare -ri BOMBASTIC=16

if [ -z "$VERBOSITY" ]; then
    declare -i debugLevel=4
else
    declare -i debugLevel=$VERBOSITY
fi

# Twisted way to get down to the fundamental script name.
tmp=${0##/*/}
tmp=${tmp%%.bash}
tmp=${tmp##*.}
tmp=${tmp##*/}
declare -r SCRIPT_NAME=$tmp

# $1 is the level to compare against debugLevel
# $2 is line number
# $3 is the string to echo to stdout.
DebugEcho()
{
    declare -ir val=$1
    if [ "$debugLevel" -ge "$val" ]; then
	#echo $2
	echo "${SCRIPT_NAME}.bash:$2: (`printf "%02d" $1`) $3"
    fi
}

#DebugEcho $TERSE     "Terse is on"
#DebugEcho $TRACE     "Trace is on"
DebugEcho $VERBOSE    $LINENO  "Verbose is on"
DebugEcho $BOMBASTIC  $LINENO  "Bombastic is on"

DebugEcho $TERSE $LINENO "debugLevel           = $debugLevel"



######################################################################
# Like perl's die command
######################################################################
die()
{
    declare -ir line=$1
    echo "ERROR: Command failed at line $line"
    exit $EXIT_FAILURE
}

