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

#include "MainWindow.h"
#include <iostream>
#include <sstream>
#include <pthread.h>
#include <gtkmm/filefilter.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <pty.h>

using namespace std;


MainWindow::MainWindow() :
			mainVBox(false, 10),
			inputFrame("Input file"),
			inputFileButton(Gtk::FILE_CHOOSER_ACTION_OPEN),
			outputFrame("Output folder"),
			outputFileButton(Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER),
			contentFrame(""),
			extractButton("Extract")
{

	this->set_title("MkvExtract-Gtk");
	this->signal_delete_event().connect(sigc::mem_fun(this, &MainWindow::onCloseButton));
	this->set_border_width(10);

	inputFrame.add(inputFileButton);
	inputFileButton.set_border_width(5);
	outputFileButton.set_border_width(5);
	mainVBox.pack_start(inputFrame, Gtk::PACK_SHRINK);
	Gtk::FileFilter mkvFileNameFilter;
	mkvFileNameFilter.set_name("MKV Files");
	mkvFileNameFilter.add_mime_type("video/x-matroska");
	inputFileButton.add_filter(mkvFileNameFilter);
	inputFileButton.signal_file_set().connect(sigc::mem_fun(this, &MainWindow::fileSet));

	outputFrame.add(outputFileButton);
	mainVBox.pack_start(outputFrame, Gtk::PACK_SHRINK);


	mainVBox.pack_start(contentFrame, Gtk::PACK_EXPAND_WIDGET);
	trackList.set_sensitive(false);

	contentFrame.add(trackList);

	 //Create the Tree model:
	refListStore = Gtk::ListStore::create(m_Columns);
	trackList.set_model(refListStore);
	trackList.set_size_request(400,150);
	trackList.set_border_width(20);
	trackList.append_column_editable("", m_Columns.m_col_selected);

	((Gtk::CellRendererToggle *) trackList.get_column_cell_renderer(0))->signal_toggled().connect(
			sigc::mem_fun(this, &MainWindow::onCheckboxClicked));

	trackList.append_column("ID", m_Columns.m_col_id);
	trackList.append_column("Type", m_Columns.m_col_type);
	trackList.append_column("Codec", m_Columns.m_col_codec);
	trackList.append_column("Language", m_Columns.m_col_language);
	trackList.append_column_editable("Output filename", m_Columns.m_col_outputFileName);

	extractButton.signal_clicked().connect(sigc::mem_fun(this, &MainWindow::onExtractButton));

	mainVBox.pack_start(progressBar, Gtk::PACK_SHRINK);
	mainVBox.pack_start(extractButton, Gtk::PACK_SHRINK);

	this->add(mainVBox);
	extractButton.set_can_focus(false);
	extractButton.set_sensitive(false);

	this->show_all();

	extracting = false;
	pthread_mutex_init(&isExtracting_mutex, 0);

	fileChoosen = false;
	trackSelected = false;
}

void MainWindow::printTracksInfos(std::vector<track_info_t> & tracks) {
	refListStore->clear();
    for(std::vector<track_info_t>::iterator i = tracks.begin();i != tracks.end();i++){
    	Gtk::TreeModel::Row row = *(refListStore->append());
    	row[m_Columns.m_col_selected] = false;
    	row[m_Columns.m_col_id] = toInteger(i->num);
    	row[m_Columns.m_col_type] = i->type;
    	row[m_Columns.m_col_codec] = i->codec;
    	row[m_Columns.m_col_language] = i->language;
    	row[m_Columns.m_col_outputFileName] = mkvExtractor.getDefaultFileName(*i);
    	tracksToExtract[toInteger(i->num)] = false;
    }
}

std::string dirName(std::string source)
{
    source.erase(std::find(source.rbegin(), source.rend(), '/').base(), source.end());
    return source;
}

void MainWindow::onCheckboxClicked(Glib::ustring str) {
	Gtk::TreeModel::Path path(str.c_str());
	Gtk::TreeModel::Row row = *(refListStore->get_iter(path));
	tracksToExtract[row[m_Columns.m_col_id]] = !tracksToExtract[row[m_Columns.m_col_id]];
	if (isATrackSelected()) {
		extractButton.set_sensitive(true);
	} else {
		if(!this->isExtracting()) {
			extractButton.set_sensitive(false);
		}
	}
}

bool MainWindow::isATrackSelected() {
	for (std::map<int,bool>::iterator i = tracksToExtract.begin(); i!=tracksToExtract.end(); i++) {
		if (i->second) { // the box is checked
			return true;
		}
	}
	return false; // no box has been checked
}

bool MainWindow::isExtracting() {
	pthread_mutex_lock(&this->isExtracting_mutex);
	bool ret = extracting;
	pthread_mutex_unlock(&this->isExtracting_mutex);
	return ret;
}

void MainWindow::setIsExtracting(bool isExtracting) {
	pthread_mutex_lock(&this->isExtracting_mutex);
	this->extracting = isExtracting;
	pthread_mutex_unlock(&this->isExtracting_mutex);
}

// return true if something to return
bool getLine(int fd, std::string &str) {
	char buffer;
	str = "";

	while (read(fd, &buffer, sizeof(buffer)) > 0) {
		// the file is not finished
		if (buffer == '\r') { // replace '\r' char by '\n'
			buffer = '\n';
		}
		str += buffer;
		if (buffer == '\n') {
			break;
		}
	}
	return str.size() != 0;
}

void* extractionThread_fun(void* args) {
	MainWindow *win = (MainWindow*) args;
	std::map<int, std::string> toExtract;
	std::map<int,bool> tracksToExtract = win->getUserSelection();
	for (std::map<int, bool>::iterator i = tracksToExtract.begin(); i != tracksToExtract.end(); i++) {
		if (i->second) {
			toExtract[i->first] = win->getOutputFolder() + "/" + win->getFileName(i->first);
		}
	}

	int master;
	win->setExtractionProcessPID(forkpty(&master, NULL, NULL, NULL));
	if (win->getExtractionProcessPID() == 0) { // child process
		close(master);

		win->getMkvExtractor().extractTracks(toExtract);

	} else { // parent process
		win->setIsExtracting(true);
		std::cout << "Command line used : "<< std::endl;
		std::cout << "-------------------- "<< std::endl;
		std::cout << win->getMkvExtractor().getExtractCommandLine(toExtract) << std::endl;

		std::string str;
		while (getLine(master, str)) {
			int progress = 0;
			int ret = sscanf(str.c_str(), "%*[^:] : %d", &progress);

			if (ret > 0) {
				win->setProgressPercentage(progress);
			} else {
//				std::cout << str;
			}

		}
		win->setIsExtracting(false);
	}
	return 0;
}

std::string MainWindow::getFileName(int id) {
	Gtk::TreeModel::Path path(toString(id-1));
	Gtk::TreeModel::Row row = *(refListStore->get_iter(path));
	std::string name;
	row.get_value(5, name);
	return name;
}

void MainWindow::stopExtraction()
{
    kill(this->extractionProcess_pid, SIGKILL);
    setIsExtracting(false);
}

bool MainWindow::onTimeOut() {
	progressBar.set_fraction((double)progress_percentage / 100.0);
	progressBar.set_text(toString(progress_percentage)+ "%");
	if (!isExtracting()) {
		extractButton.set_label("Extract");
		return false;
	} else {
		return true;
	}
}

void MainWindow::onExtractButton() {
	if (!isExtracting()) {
		if (isATrackSelected()) {
			extractButton.set_label("Cancel");
			sigc::connection conn = Glib::signal_timeout().connect(sigc::mem_fun(*this, &MainWindow::onTimeOut), 500);
			pthread_create(&extraction_thread, 0, &extractionThread_fun, (void*) this);
		}
	} else {
		stopExtraction();
		extractButton.set_label("Extract");
	}
	progress_percentage = 0;
	progressBar.set_fraction((double)progress_percentage / 100.0);
	progressBar.set_text(toString(progress_percentage)+ "%");
}

bool MainWindow::onCloseButton(GdkEventAny * ev) {
	if (isExtracting()) {
		this->stopExtraction();
	}
	return false;
}

void MainWindow::fileSet() {
	fileChoosen = true;
	trackSelected = false;

	trackList.set_sensitive(true);

	mkvExtractor = MkvExtractor(inputFileButton.get_filename());

	tracksToExtract.clear();
	tracks = mkvExtractor.getTracksInfos();
	printTracksInfos(tracks);

	outputFileButton.set_current_folder(dirName(inputFileButton.get_filename().c_str()));
}

