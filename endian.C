// $Revision$  $Author$  $Date$
/*
    Copyright (C) 2004  Kurt Schwehr

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include <cstdio>
#include <cstdlib>
using namespace std;
int main(void) {
  int s;
  char *c=(char *)&s;
  c[0]=0x00; c[1]=0x01; c[2]=0x02; c[3]=0x03;
  if (0x00010203==s) {printf("BIGENDIAN"   );return(EXIT_SUCCESS);}
  if (0x03020100==s) {printf("LITTLEENDIAN");return(EXIT_SUCCESS);}
  printf("ERROR!!!  I can not cope with this beast... HELP!!!!\n");
  return (EXIT_FAILURE);
}
