/*
===========================================================================

mkvextract-gtk
Copyright (C) 2011-2019 Leyorus <leyorusdev@gmail.com>

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

#ifndef DEF_MKVINFOEXTRACTOR
#define DEF_MKVINFOEXTRACTOR

#include <vector>
#include <string>
#include <map>
#include "Common.h"

namespace Core {

class MkvExtractor {
public:
	static void extractTracks(std::string inputFilePath, const std::map<int, std::string> tracks_to_extract);
	static std::string getExtractCommandLine(std::string inputFilePath, std::map<int, std::string> tracks_to_extract);
	static std::string getDefaultFileName (track_info_t info);

private:
	static std::vector<std::string> makeExtractCommandLine(std::string inputFilePath, std::map<int, std::string> tracks_to_extract, bool usable);
	const static std::map<std::string, std::string> createFileExtensionsMap();
	static std::string getDefaultFileNameExtension(track_info_t info);
};

}
#endif
