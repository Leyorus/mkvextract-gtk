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
#include <unistd.h>
#include <map>


MkvExtractor::MkvExtractor(std::string filePath) {
	this->filePath = filePath;
	this->fileExtensionsMap = createFileExtensionsMap();
    std::string mkvinfo_out = getRawMkvInfo(filePath);
	this->tracks_infos = parseTracksInfos(mkvinfo_out);
}

std::map<std::string, std::string> MkvExtractor::createFileExtensionsMap() {
	std::map<std::string, std::string> m;
	m["V_MPEG1"] = "mpg";
	m["A_AAC"] = "aac";
	m["A_AC3"] = "ac3";
	m["A_DTS"] = "dts";
	m["A_FLAC"] = "flac";
	m["A_APE"] = "ape";
	m["A_QUICKTIME"] = "qdm";
	m["A_TTA1"] = "tta";
	m["A_WAVPACK4"] = "wv";
	m["A_VORBIS"] = "ogg";
	m["A_REAL"] = "ra";
	m["V_MPEG2"] = "mpg";
	m["V_REAL"] = "rmvb";
	m["V_MS/VFW/FOURCC"] = "avi";
	m["V_MPEG4/ISO/AVC"] = "h264";
	m["S_VOBSUB"] = "sub";
	m["A_MPEG/L3"] = "mp3";
	m["A_MPEG/L2"] = "mp2";
	m["A_MPEG/L1"] = "mpa";
	m["A_PCM/INT/LIT"] = "wav";
	m["S_HDMV/PGS"] = "sup";
	m["S_TEXT/UTF8"] = "srt";
	m["S_TEXT/SSA"] = "ssa";
	m["S_TEXT/ASS"] = "ass";
	m["S_TEXT/USF"] = "usf";
	return m;
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

std::string MkvExtractor::exec(std::string cmd) {
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

std::string MkvExtractor::substring_toend(std::string& s, int from) {
	int end_of_line = s.find("\n", from);
	return s.substr(from, end_of_line - from);
}

std::string MkvExtractor::parse(std::string& s, unsigned int track_number, std::string key) {
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

std::vector<std::string> MkvExtractor::makeExtractCommandLine(std::map<int, std::string> tracks_to_extract, bool usable) {
	std::vector<std::string> ret;
	ret.push_back("mkvextract");
	ret.push_back("tracks");
	if(usable) {
		ret.push_back("\"" + filePath + "\"");
	    for(std::map<int, std::string>::iterator i = tracks_to_extract.begin(); i != tracks_to_extract.end();i++){
	        ret.push_back(toString(i->first) + ":" + "\""+ i->second + "\"");
	    }
	} else {
		ret.push_back(filePath);
	    for(std::map<int, std::string>::iterator i = tracks_to_extract.begin(); i != tracks_to_extract.end();i++){
	        ret.push_back(toString(i->first) + ":" + i->second);
	    }
	}
    return ret;
}

std::string MkvExtractor::getExtractCommandLine(std::map<int, std::string> tracks_to_extract) {
	std::vector<std::string> cmd = makeExtractCommandLine(tracks_to_extract, true);
	std::string ret = "";

	for (int i=0; i < (cmd.size()-1) ; i++) {
		ret += cmd.at(i) + " ";
	}
	ret += cmd.at(cmd.size()-1);
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

std::string MkvExtractor::getDefaultFileName (track_info_t info){
	return "Track" + info.num + "_" + info.language + getDefaultFileNameExtension(info);
}

std::string MkvExtractor::getDefaultFileNameExtension(track_info_t info) {
	std::map<std::string, std::string>::iterator it;
	it = this->fileExtensionsMap.find(info.codec);
	if (it != this->fileExtensionsMap.end()) {
		return "." + it->second;
	} else {
		return "";
	}
}

void MkvExtractor::extractTracks(const std::map<int, std::string> tracks_to_extract) {
	if (tracks_to_extract.size() != 0) {
		std::vector<std::string> cmdline = makeExtractCommandLine(tracks_to_extract, false);

	    int i;
	    char **argv = new char*[tracks_to_extract.size()+1];
	    std::vector<std::string>::iterator it;
	    for(i = 0, it = cmdline.begin(); it != cmdline.end(); ++it, ++i)
	    {
	       argv[i] = const_cast<char*>((*it).c_str());
	    }
	    argv[i] = 0;

	    execvp(argv[0], argv);
	    delete[] argv;

	} else {
		std::cout << "No track to extract" << std::endl;
	}
}

std::string MkvExtractor::getRawMkvInfo(std::string filePath) {
	std::string cmdline("mkvinfo \"" + filePath + "\"");
	return exec(cmdline.c_str());
}

int MkvExtractor::getNumberOfTracks(std::string raw_infos) {
    return extractNumberOfTracks(raw_infos);
}

