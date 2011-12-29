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
#include "MkvInfoParser.h"
#include "MkvExtractor.h"

#include <iostream>
#include <sstream>
#include <pthread.h>
#include <gtkmm/filefilter.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <gtkmm/image.h>

#include <pty.h>

using namespace std;


MainWindow::MainWindow() :
			mainVBox(false, 10),
			inputFrame("Input file"),
			inputFileButton(Gtk::FILE_CHOOSER_ACTION_OPEN),
			outputFrame("Output folder"),
			outputFileButton(Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER),
			contentFrame("Content"),
			extractOrPauseButton("Extract"),
			cancelButton(Gtk::Stock::CANCEL)
{
	this->set_title("MkvExtract-Gtk");
	this->signal_delete_event().connect(sigc::mem_fun(this, &MainWindow::onCloseButton));
	this->set_border_width(10);
	extractOrPauseButton.set_image(*Gtk::manage (new Gtk::Image (Gtk::Stock::CONVERT, Gtk::ICON_SIZE_BUTTON)));
	inputFrame.add(inputFileButton);
	inputFileButton.set_border_width(5);
	outputFileButton.set_border_width(5);
	mainVBox.pack_start(inputFrame, Gtk::PACK_SHRINK);
	Gtk::FileFilter mkvFileNameFilter;
	mkvFileNameFilter.set_name("MKV Files");
	mkvFileNameFilter.add_mime_type("video/x-matroska");
	inputFileButton.add_filter(mkvFileNameFilter);
	inputFileButton.signal_file_set().connect(sigc::mem_fun(this, &MainWindow::onFileSet));

	outputFrame.add(outputFileButton);
	mainVBox.pack_start(outputFrame, Gtk::PACK_SHRINK);


	mainVBox.pack_start(contentFrame, Gtk::PACK_EXPAND_WIDGET);
	trackList.set_sensitive(false);

//	contentFrame.add(trackList);
	scrolledContentWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	contentFrame.add(scrolledContentWindow);
	scrolledContentWindow.add(trackList);

	 //Create the Tree model:
	refListStore = Gtk::ListStore::create(m_Columns);
	trackList.set_model(refListStore);
	trackList.set_size_request(450,150);
	trackList.set_border_width(20);
	trackList.append_column_editable("", m_Columns.m_col_selected);

	((Gtk::CellRendererToggle *) trackList.get_column_cell_renderer(0))->signal_toggled().connect(
			sigc::mem_fun(this, &MainWindow::onCheckboxClicked));

	trackList.append_column("ID", m_Columns.m_col_id);
	trackList.append_column("Type", m_Columns.m_col_type);
	trackList.append_column("Codec", m_Columns.m_col_codec);
	trackList.append_column("Language", m_Columns.m_col_language);
	trackList.append_column_editable("Output filename", m_Columns.m_col_outputFileName);

	extractOrPauseButton.signal_clicked().connect(sigc::mem_fun(this, &MainWindow::onExtractOrPauseButton));
	cancelButton.signal_clicked().connect(sigc::mem_fun(this, &MainWindow::onCancelButton));

	mainVBox.pack_start(progressBar, Gtk::PACK_SHRINK);
	mainVBox.pack_start(hButtonBox, Gtk::PACK_SHRINK);

	hButtonBox.set_layout(Gtk::BUTTONBOX_SPREAD);
	hButtonBox.pack_start(extractOrPauseButton);
	hButtonBox.pack_start(cancelButton);

	this->add(mainVBox);
	extractOrPauseButton.set_can_focus(false);
	extractOrPauseButton.set_sensitive(false);

	cancelButton.set_can_focus(false);
	cancelButton.set_sensitive(false);

	this->show_all();

	current_state = stop_status;
	pthread_mutex_init(&extraction_status_mutex, 0);
}

void MainWindow::printTracksInfos(std::vector<Core::track_info_t> tracks) {
	refListStore->clear();
    for(std::vector<Core::track_info_t>::iterator i = tracks.begin();i != tracks.end();i++){
    	Gtk::TreeModel::Row row = *(refListStore->append());
    	row[m_Columns.m_col_selected] = false;
    	row[m_Columns.m_col_id] = Core::toInteger(i->num);
    	row[m_Columns.m_col_type] = i->type;
    	row[m_Columns.m_col_codec] = i->codec;
    	row[m_Columns.m_col_language] = i->language;
    	row[m_Columns.m_col_outputFileName] = Core::MkvExtractor::getDefaultFileName(*i);
    	tracksToExtract[Core::toInteger(i->num)] = false;
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
		extractOrPauseButton.set_sensitive(true);
	} else {
		if(!this->isExtracting()) {
			extractOrPauseButton.set_sensitive(false);
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
	pthread_mutex_lock(&this->extraction_status_mutex);
	bool ret = (current_state == extracting_status);
	pthread_mutex_unlock(&this->extraction_status_mutex);
	return ret;
}

void MainWindow::setExtractionStatus(extraction_status_t newState) {
	pthread_mutex_lock(&this->extraction_status_mutex);
	this->current_state = newState;
	pthread_mutex_unlock(&this->extraction_status_mutex);
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

		Core::MkvExtractor::extractTracks(win->getInputFileName(), toExtract);

	} else { // parent process
		win->setExtractionStatus(extracting_status);
		std::cout << "Command line used : "<< std::endl;
		std::cout << "-------------------- "<< std::endl;
		std::cout << Core::MkvExtractor::getExtractCommandLine(win->getInputFileName(), toExtract) << std::endl;

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
		win->setExtractionStatus(stop_status);
	}
	return 0;
}

std::string MainWindow::getInputFileName(){
	return this->inputFileButton.get_filename();
}

std::string MainWindow::getFileName(int id) {
	Gtk::TreeModel::Path path(Core::toString(id-1));
	Gtk::TreeModel::Row row = *(refListStore->get_iter(path));
	std::string name;
	row.get_value(5, name);
	return name;
}

void MainWindow::stopExtraction()
{
    kill(this->extractionProcess_pid, SIGKILL);
    setExtractionStatus(stop_status);
    onExtractionEnd();
}

void MainWindow::pauseExtraction()
{
    kill(this->extractionProcess_pid, SIGSTOP);
    setExtractionStatus(paused_status);
    extractOrPauseButton.set_image(*Gtk::manage (new Gtk::Image (Gtk::Stock::MEDIA_PLAY, Gtk::ICON_SIZE_BUTTON)));
	extractOrPauseButton.set_label("Continue");
}

void MainWindow::continueExtraction() {
	extractOrPauseButton.set_label("Pause");
	extractOrPauseButton.set_image(*Gtk::manage (new Gtk::Image (Gtk::Stock::MEDIA_PAUSE, Gtk::ICON_SIZE_BUTTON)));
    kill(this->extractionProcess_pid, SIGCONT);
    setExtractionStatus(extracting_status);
	enableTimer();
}

void MainWindow::startExtraction() {
	extractOrPauseButton.set_label("Pause");
	cancelButton.set_sensitive(true);
	extractOrPauseButton.set_image(*Gtk::manage (new Gtk::Image (Gtk::Stock::MEDIA_PAUSE, Gtk::ICON_SIZE_BUTTON)));
	pthread_create(&extraction_thread, 0, &extractionThread_fun, (void*) this);
	enableTimer();
}

void MainWindow::enableTimer() {
	sigc::connection con = Glib::signal_timeout().connect(sigc::mem_fun(*this, &MainWindow::onTimeOut), 500);
}

bool MainWindow::onTimeOut() {
	bool ret = true;
	switch(current_state) {
	case paused_status:
		ret = false; // disconnect timer
		break;
	case extracting_status:
		updateProgressBar(); // return true = continue timer
		break;
	case stop_status:
		onExtractionEnd();
		ret = false; // disconnect timer
		break;
	}
	return ret;
}

void MainWindow::updateProgressBar() {
	progressBar.set_fraction((double)progress_percentage / 100.0);
	progressBar.set_text(Core::toString(progress_percentage)+ "%");
}

void MainWindow::onExtractionEnd() {
	extractOrPauseButton.set_label("Extract");
	cancelButton.set_sensitive(false);
	extractOrPauseButton.set_image(*Gtk::manage (new Gtk::Image (Gtk::Stock::CONVERT, Gtk::ICON_SIZE_BUTTON)));
	progress_percentage = 0;
	progressBar.set_fraction((double)progress_percentage / 100.0);
	progressBar.set_text(Core::toString(progress_percentage)+ "%");
}

void MainWindow::onExtractOrPauseButton() {
	switch(current_state) {
	case paused_status:
		continueExtraction();
		break;
	case stop_status:
		startExtraction();
		break;
	case extracting_status:
		pauseExtraction();
		break;
	}
}

void MainWindow::onCancelButton() {
	stopExtraction();
}

bool MainWindow::onCloseButton(GdkEventAny * ev) {
	if (isExtracting()) {
		this->stopExtraction();
	}
	return false;
}

void MainWindow::onFileSet() {

	trackList.set_sensitive(true);

	tracksToExtract.clear();
	printTracksInfos(Core::MkvInfoParser::parseTracksInfos(getInputFileName()));

	outputFileButton.set_current_folder(dirName(getInputFileName()));
}

