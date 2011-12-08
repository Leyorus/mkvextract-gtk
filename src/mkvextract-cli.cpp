/*
===========================================================================

mkvextract-cli
Copyright (C) Leyorus <leyorus@gmail.com>

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

#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include <vector>

#include "MkvExtractor.h"



std::vector<int> parseUserTrackNumberList(std::string &tracks_numbers) {
	std::vector<int> ret;
	int current = 0;
	int next = 0;

	while (current != std::string::npos && current < tracks_numbers.size()) {
		next = tracks_numbers.find(',', current);

		if (next == std::string::npos) {
			ret.push_back(toInteger(tracks_numbers.substr(current, tracks_numbers.size() - current)));
			current = next;
		} else {
			ret.push_back(toInteger(tracks_numbers.substr(current, next - current)));
			current = next + 1;
		}
	}
	return ret;
}

std::vector<int> askUserForTrackNumberToExtract() {
	std::cout << "Which track do you want to extract : ";
	std::string tracks_to_extract_str;
	std::cin >> tracks_to_extract_str;
	return parseUserTrackNumberList(tracks_to_extract_str);
}

void printTracksInfos(std::vector<track_info_t> & tracks) {
    std::cout << "Number of tracks = " << tracks.size() << std::endl;
    for(std::vector<track_info_t>::iterator i = tracks.begin();i != tracks.end();i++){
        std::cout << i->num << " : ";
        std::cout << i->type << " : ";
        std::cout << i->language << " : ";
        std::cout << i->codec << std::endl;
    }
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		std::cout << "Wrong parameter" << std::endl;
		exit(1);
	}
	std::string mkvfilename(argv[1]);
	MkvExtractor mkv(mkvfilename);

	std::vector<track_info_t> tracks = mkv.getTracksInfos();
	printTracksInfos(tracks);

	std::vector<int> tracks_to_extract = askUserForTrackNumberToExtract();

	mkv.extractTracks(tracks_to_extract);

	return 0;
}
