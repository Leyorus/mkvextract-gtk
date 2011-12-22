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

#include "MkvInfoParser.h"
#include "Common.h"
#include <stdio.h>

namespace Core {

const std::string TRACK_STRING = "A track";
const std::string CODEC_ID_STRING = "Codec ID: ";
const std::string TRACK_TYPE_STRING = "Track type: ";
const std::string TRACK_NUMBER_STRING = "Track number: ";
const std::string LANGUAGE_STRING = "Language: ";

std::string exec(std::string cmd) {
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while(!feof(pipe)) {
        if(fgets(buffer, 128, pipe) != NULL)
                result += buffer;
    }
    pclose(pipe);
    return result;
}

std::string MkvInfoParser::getRawMkvInfo(std::string filePath) {
	std::string cmdline("mkvinfo \"" + filePath + "\"");
	return exec(cmdline.c_str());
}


std::string substring_toend(std::string& s, int from) {
	int end_of_line = s.find("\n", from);
	return s.substr(from, end_of_line - from);
}

std::string parse(std::string& s, unsigned int track_number, std::string key) {
	unsigned int current_track_number = 0;
	size_t current_track_pos = 0;
	size_t key_pos;

	//looking for asked track position
	do {
		current_track_pos = s.find(TRACK_NUMBER_STRING, current_track_pos) + TRACK_NUMBER_STRING.size();
		if (current_track_pos != std::string::npos) {
			current_track_number = toInteger(substring_toend(s, current_track_pos));
		} else {
			return "unknown";
		}
	} while (current_track_number != track_number);

	// looking for the next track (if there is any)
	size_t next_track_pos = s.find(TRACK_NUMBER_STRING, current_track_pos);

	if (next_track_pos != std::string::npos) {
		key_pos = s.find(key, current_track_pos) + key.size();
		if (key_pos != std::string::npos) {
			if (key_pos < next_track_pos) {
				return substring_toend(s, key_pos);
			} else {
				return "unknown";
			}
		} else {
			return "keypos >= next_track_pos";
		}
	} else {
		key_pos = s.find(key, current_track_pos) + key.size();
		return substring_toend(s, key_pos);
	}
	return "unknown";
}

int extractNumberOfTracks(std::string raw_infos) {
	unsigned int numberOfTracks = 0;
	size_t found = raw_infos.find(TRACK_STRING, 0) + TRACK_STRING.size();
    while(found != std::string::npos){
        numberOfTracks++;
        found = raw_infos.find(TRACK_STRING, found + 1);
    }
    return numberOfTracks;
}

std::vector<track_info_t> MkvInfoParser::parseTracksInfos(std::string mkvFileName) {
	std::string raw_infos = getRawMkvInfo(mkvFileName);
	std::vector<track_info_t> tracks;

    for (int track_number=1; track_number<= extractNumberOfTracks(raw_infos); track_number++) {
        track_info_t track;
        track.num = toString(track_number);
        track.type = parse(raw_infos, track_number, TRACK_TYPE_STRING);
        track.codec = parse(raw_infos, track_number, CODEC_ID_STRING);
        track.language = parse(raw_infos, track_number, LANGUAGE_STRING);
        tracks.push_back(track);
    }
    return tracks;
}

}
