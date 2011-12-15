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

#include <vector>
#include <string>
#include <map>


typedef struct track_info_t{
	std::string num;
	std::string type;
	std::string codec;
	std::string language;
	track_info_t() {
		num = "0";
		type = "unknown";
		codec = "unknown";
		language = "unknown";
	}
}track_info_t;


class MkvExtractor {
public:
	MkvExtractor(){};
	MkvExtractor(std::string filePath);
	std::vector<track_info_t> getTracksInfos() {
		return tracks_infos;
	}
	void extractTracks(const std::map<int, std::string> tracks_to_extract);
	std::string getExtractCommandLine(std::map<int, std::string> tracks_to_extract);
	std::string getDefaultFileName (track_info_t info);

private:
	std::string filePath;
	std::vector<track_info_t> tracks_infos;
	std::string getRawMkvInfo(std::string filePath);
	int getNumberOfTracks(std::string raw_infos);
	std::vector<track_info_t> parseTracksInfos(std::string raw_infos);
	std::vector<std::string> makeExtractCommandLine(std::map<int, std::string> tracks_to_extract, bool usable);

};

std::string toString(int number);
int toInteger(const std::string &str);

std::string exec(std::string cmd);

std::string substring_toend(std::string& s, int n_pos);

std::string parse(std::string& s, int track_number, std::string key);

std::vector<track_info_t> parseTracksInfos(std::string & mkvinfo_out);

std::vector<track_info_t> getMkvTracksInfos(std::string & mkvfilename);
