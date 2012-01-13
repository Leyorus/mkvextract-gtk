/*
===========================================================================

mkvextract-gtk
Copyright (C) 2011 Leyorus <leyorus@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see  <http://www.gnu.org/licenses/>.
===========================================================================
*/

#ifndef DEF_MKVINFOPARSER
#define DEF_MKVINFOPARSER

#include <vector>
#include <string>
#include "Common.h"

namespace Core {
class MkvInfoParser {
public:
	static std::vector<track_info_t> parseTracksInfos(std::string mkvFileName);
};

}
#endif

