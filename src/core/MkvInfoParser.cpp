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

#include "yamka/yamkaElt.h"
#include "yamka/yamkaPayload.h"
#include "yamka/yamkaStdInt.h"
#include "yamka/yamkaFileStorage.h"
#include "yamka/yamkaEBML.h"
#include "yamka/yamkaMatroska.h"

using namespace Yamka;


namespace Core {


struct Reader : public IDelegateLoad
{

  uint64 load(FileStorage & storage,
		      uint64 payloadBytesToRead,
              uint64 eltId,
              IPayload & payload)
  {
	  switch (eltId) {
	  case EbmlDoc::THead::kId:
//		  std::cout << "header" << std::endl;
		  break;
	  case MatroskaDoc::TSegment::kId:
		  return 0;
		  break;
	  case Segment::TTracks::kId:
//		  std::cout << "track" << std::endl;
		  return payload.load(storage, payloadBytesToRead, NULL);
		  break;
	  case Segment::TAttachment::kId:
		  break;
	  case Segment::TChapters::kId:
		  break;
	  default:
		  break;
	  }

    // skip reading (to shorten file load time):
    storage.file_.seek(payloadBytesToRead, File::kRelativeToCurrent);
    return payloadBytesToRead;
  }
};


std::string getTrackTypeName(const unsigned long int type) {

    enum MatroskaTrackType
    {
      kTrackTypeVideo = 1,
      kTrackTypeAudio = 2,
      kTrackTypeComplex = 3,
      kTrackTypeLogo = 0x10,
      kTrackTypeSubtitle = 0x11,
      kTrackTypeButtons = 0x12,
      kTrackTypeControl = 0x20
    };

	switch (type){
	case kTrackTypeVideo:
		return "video";
		break;
	case kTrackTypeAudio:
		return "audio";
		break;
	case kTrackTypeComplex:
		return "complex";
		break;
	case kTrackTypeLogo:
		return "logo";
		break;
	case kTrackTypeSubtitle:
		return "subtitles";
		break;
	case kTrackTypeButtons:
		return "buttons";
		break;
	case kTrackTypeControl:
		return "control";
		break;

	default:
		return "";
		break;
	}
	return "";
}


std::vector<track_info_t> MkvInfoParser::parseTracksInfos(std::string mkvFileName) {
	std::vector<track_info_t> tracks;

	FileStorage srcFile(mkvFileName, File::kReadOnly);
	if (!srcFile.file_.isOpen()) {
		std::cerr << "Error : unable to open the input file " << mkvFileName << std::endl;
	}

	uint64 srcFileSize = srcFile.file_.size();
	MatroskaDoc infos;

	Reader loader;

	uint64 bytesRead = 0;
	bool file_isok = infos.loadSeekHead(srcFile, srcFileSize);

	if (file_isok) {
		file_isok = infos.loadViaSeekHead(srcFile, &loader, false);
	}

	if (!file_isok || infos.segments_.empty()) {
		std::cerr << "Error : the input file is not a valid matroska file : "
				<< mkvFileName << std::endl;
	} else {
		MatroskaDoc::TSegment & segment = infos.segments_.back();
		typedef std::deque<Segment::TTracks::TPayload::TTrack> TTrackList;
		TTrackList trackList = segment.payload_.tracks_.payload_.tracks_;

		for (TTrackList::iterator it = trackList.begin(); it != trackList.end();
				it++) {
			track_info_t track;
			track.num = toString(it->payload_.trackNumber_.payload_.get() - 1); // (-1) used for compatiblity with mkvextract utility (to solve later)
			track.type = getTrackTypeName(
					it->payload_.trackType_.payload_.get());
			track.codec = it->payload_.codecID_.payload_.get();
			track.language = it->payload_.language_.payload_.get();
			tracks.push_back(track);
		}
	}

	// closing file handles:
	infos = MatroskaDoc();
	srcFile = FileStorage();

	return tracks;
}



}
