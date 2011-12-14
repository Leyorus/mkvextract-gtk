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

#include "MkvExtractor.h"

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>



MkvExtractor::MkvExtractor(std::string filePath) {
	this->filePath = fixedFileName(filePath);
    std::string mkvinfo_out = getRawMkvInfo(filePath);
	this->tracks_infos = parseTracksInfos(mkvinfo_out);
}

const std::string TRACK_STRING = "A track";
const std::string CODEC_ID_STRING = "Codec ID: ";
const std::string TRACK_TYPE_STRING = "Track type: ";
const std::string TRACK_NUMBER_STRING = "Track number: ";
const std::string LANGUAGE_STRING = "Language: ";

std::string toString(int number) {
   std::stringstream ss;
   ss << number;
   return ss.str();
}

int toInteger(const std::string &str) {
	std::stringstream ss(str);
	int n;
	ss >> n;
	return n;
}

std::string exec(const char* cmd) {
    FILE* pipe = popen(cmd, "r");
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

std::string substring_toend(std::string& s, int from) {
	int end_of_line = s.find("\n", from);
	return s.substr(from, end_of_line - from);
}

std::string parse(std::string& s, int track_number, std::string key) {
	int current_track_number = 0;
	int current_track_pos = 0;
	int key_pos;

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
	int next_track_pos = s.find(TRACK_NUMBER_STRING, current_track_pos);

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

std::string fixedFileName(std::string filename) {
	return "\"" + filename + "\"";
}

int extractNumberOfTracks(std::string raw_infos) {
	int numberOfTracks = 0;
    int found = raw_infos.find(TRACK_STRING, 0) + TRACK_STRING.size();
    while(found != std::string::npos){
        numberOfTracks++;
        found = raw_infos.find(TRACK_STRING, found + 1);
    }
    return numberOfTracks;
}

std::string MkvExtractor::makeExtractCommandLine(const std::vector<int>& tracks_to_extract) {
	// Creation of the command line to use for extraction
    std::string cmdline_extraction("mkvextract tracks " + this->filePath);
    for(int i = 0;i < tracks_to_extract.size();i++){
        cmdline_extraction += " " + toString(tracks_to_extract.at(i)) + ":Track" + toString(tracks_to_extract.at(i)) + "_" + this->tracks_infos.at(tracks_to_extract.at(i)-1).language;
    }
    return cmdline_extraction;
}

std::vector<int> fixTrackNumberList(std::vector<int> tracks_to_extract, int trueNumberOfTrack) {
	// Clean bad track number
	std::vector<int> ret = tracks_to_extract;
	std::vector<int>::iterator i = ret.begin();
	std::cout << "true number of track =" << trueNumberOfTrack << std::endl;
	while( i != ret.end()) {
		if (*i <= 0 || *i > trueNumberOfTrack) {
			std::cout << "Track number " << *i << " is out of range. Removing it from extraction list..." << std::endl;
			ret.erase(i);
		} else {
			i++;
		}
	}
	return ret;
}


std::vector<track_info_t> MkvExtractor::parseTracksInfos(std::string raw_infos) {
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

void MkvExtractor::extractTracks(const std::vector<int> tracks_to_extract) {
	std::vector<int> fixed_tracks_to_extract = fixTrackNumberList(tracks_to_extract, this->tracks_infos.size());
	if (fixed_tracks_to_extract.size() != 0) {
		std::string cmdline_extraction = makeExtractCommandLine(fixed_tracks_to_extract);
	    std::cout << "Running command line : " + cmdline_extraction << std::endl;
	    system(cmdline_extraction.c_str());
	} else {
		std::cout << "No track to extract" << std::endl;
	}
}

std::string MkvExtractor::getRawMkvInfo(std::string filePath) {
	std::string cmdline("mkvinfo " + fixedFileName(filePath));
	return exec(cmdline.c_str());
}

int MkvExtractor::getNumberOfTracks(std::string raw_infos) {
    return extractNumberOfTracks(raw_infos);
}

