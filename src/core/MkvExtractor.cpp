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

#include "MkvExtractor.h"

#include <iostream>
#include <unistd.h>
#include <map>

#include "Common.h"
#include "MkvInfoParser.h"


namespace Core {

const std::map<std::string, std::string> MkvExtractor::createFileExtensionsMap() {
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

std::vector<std::string> MkvExtractor::makeExtractCommandLine(std::string inputFilePath, std::map<int, std::string> tracks_to_extract, bool usable) {
	std::vector<std::string> ret;
	ret.push_back("mkvextract");
	ret.push_back("tracks");
	if(usable) {
		ret.push_back("\"" + inputFilePath + "\"");
	    for(std::map<int, std::string>::iterator i = tracks_to_extract.begin(); i != tracks_to_extract.end();i++){
	        ret.push_back(toString(i->first) + ":" + "\""+ i->second + "\"");
	    }
	} else {
		ret.push_back(inputFilePath);
	    for(std::map<int, std::string>::iterator i = tracks_to_extract.begin(); i != tracks_to_extract.end();i++){
	        ret.push_back(toString(i->first) + ":" + i->second);
	    }
	}
    return ret;
}

std::string MkvExtractor::getExtractCommandLine(std::string inputFilePath, std::map<int, std::string> tracks_to_extract) {
	std::vector<std::string> cmd = makeExtractCommandLine(inputFilePath, tracks_to_extract, true);
	std::string ret = "";

	for (size_t i=0; i < (cmd.size()-1) ; i++) {
		ret += cmd.at(i) + " ";
	}
	ret += cmd.at(cmd.size()-1);
	return ret;
}

std::string MkvExtractor::getDefaultFileName (track_info_t info){
	std::string ret = "Track" + info.num;
	ret+= "_" + info.type;
	if (info.language.size() != 0)
		ret += "_" + info.language;
	std::string extension = getDefaultFileNameExtension(info);
	if (extension.size() !=0){
		ret += "." + extension;
	}
	return ret;
}

std::string MkvExtractor::getDefaultFileNameExtension(track_info_t info) {
	std::map<std::string, std::string>::iterator it;
	std::map<std::string, std::string> extensions = createFileExtensionsMap();
	it = extensions.find(info.codec);
	if (it != extensions.end()) {
		return it->second;
	} else {
		return "";
	}
}

void MkvExtractor::extractTracks(std::string inputFilePath, const std::map<int, std::string> tracks_to_extract) {
	if (tracks_to_extract.size() != 0) {
		std::vector<std::string> cmdline = makeExtractCommandLine(inputFilePath, tracks_to_extract, false);

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

	} else { // No track to extract
	}
}

}
