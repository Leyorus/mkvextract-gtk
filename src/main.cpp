#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include <sstream>
#include <vector>

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

const std::string TRACK_STRING = "A track";
const std::string CODEC_ID_STRING = "Codec ID: ";
const std::string TRACK_TYPE_STRING = "Track type: ";
const std::string TRACK_NUMBER_STRING = "Track number: ";
const std::string LANGUAGE_STRING = "Language: ";

std::string substring_toend(std::string& s, int n_pos) {
	int end_of_line = s.find("\n", n_pos);
	return s.substr(n_pos, end_of_line - n_pos);
}

std::string parse(std::string& s, std::string key, int *n_pos) {
	*n_pos = s.find(key, *n_pos) + key.size();
	return substring_toend(s, *n_pos);
}

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

typedef struct {
	std::string num;
	std::string type;
	std::string codec;
	std::string language;
}track_info_t;

void fixFileName(std::string &filename) {
	filename = "\"" + filename + "\"";
}

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

void cleanTrackNumberList(std::vector<int> &ret, int trueNumberOfTrack) {
	// Clean bad track number
	std::vector<int>::iterator i = ret.begin();
	while( i != ret.end()) {
		if (*i < 0 || *i > trueNumberOfTrack) {
			std::cout << "Track number " << *i << " is out of range. Removing it..." << std::endl;
			ret.erase(i);
		} else {
			i++;
		}
	}
}


int main(int argc, char *argv[])
{
	if (argc != 2) {
		std::cout << "Wrong parameter" << std::endl;
		exit(1);
	}
	std::string mkvfilename(argv[1]);
	fixFileName(mkvfilename);

	std::string cmdline("mkvinfo " + mkvfilename);
	std::string mkvinfo_out = exec(cmdline.c_str());

	int nb_tracks = 0;
	std::vector<track_info_t> tracks;

	int found = mkvinfo_out.find(TRACK_STRING, 0) + TRACK_STRING.size();
	while (found != std::string::npos) {
		// A new track has been found
		nb_tracks++;
		track_info_t track;
		track.num = parse(mkvinfo_out, TRACK_NUMBER_STRING, &found);
		track.type = parse(mkvinfo_out, TRACK_TYPE_STRING, &found);
		track.codec = parse(mkvinfo_out, CODEC_ID_STRING, &found);
		//		if (track.type != "video")
		//			track.language = parse(mkvinfo_out, LANGUAGE_STRING, &found);
		tracks.push_back(track);

		found = mkvinfo_out.find(TRACK_STRING, found + 1);
	}
	std::cout << "Number of tracks = " << nb_tracks << std::endl;



	for (std::vector<track_info_t>::iterator i = tracks.begin(); i
			!= tracks.end(); i++) {
		std::cout << i->num << " : ";
		std::cout << i->type << " : ";
		//		if (i->type != "video")
		//			std::cout << i->language << " : ";
		std::cout << i->codec << std::endl;
	}

	std::vector<int> tracks_to_extract = askUserForTrackNumberToExtract();
	cleanTrackNumberList(tracks_to_extract, nb_tracks);

	// Creation of the command line to use for extraction

	if (tracks_to_extract.size() != 0) {
		std::string cmdline_extraction("mkvextract tracks " + mkvfilename);
		for (int i = 0; i < tracks_to_extract.size(); i++) {
			cmdline_extraction += " " + toString(tracks_to_extract.at(i)) + ":Track" + toString(tracks_to_extract.at(i));
		}
		std::cout << "Running command line : " + cmdline_extraction << std::endl;
		system(cmdline_extraction.c_str());
	} else {
		std::cout << "No track to extract" << std::endl;
	}

	return 0;
}
