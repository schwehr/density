// $Revision$  $Author$  $Date$
/*
    Copyright (C) 2004  Kurt Schwehr

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#ifndef _KDS_PMAG_L_H_
#define _KDS_PMAG_L_H_

#include <vector>

/// \file
/// \brief provide the SVec data type.  See \a k15_s or the Tauxe book.


/// This class is used for the 6 or 7 value s vector.  Probably a bad name.
typedef std::vector<float> SVec; // should always size()==6

#endif
