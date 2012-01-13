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
#include <core/MkvInfoParser.h>
#include <core/MkvExtractor.h>

#include <iostream>
#include <sstream>
#include <pthread.h>
#include <gtkmm/filefilter.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <gtkmm/image.h>
#include <glibmm/refptr.h>
#include <gtkmm/messagedialog.h>

#include <sys/time.h> // for gettimeofday() function

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
	this->set_size_request(500,350);
	extractOrPauseButton.set_image(*Gtk::manage (new Gtk::Image (Gtk::Stock::CONVERT, Gtk::ICON_SIZE_BUTTON)));
	inputFrame.add(inputFileButton);
	//inputFileButton.set_border_width(5);
	//outputFileButton.set_border_width(5);
	mainVBox.pack_start(inputFrame, Gtk::PACK_SHRINK);
	Glib::RefPtr<Gtk::FileFilter> mkvFileNameFilter = Gtk::FileFilter::create();
	mkvFileNameFilter->set_name("MKV Files");
	mkvFileNameFilter->add_mime_type("video/x-matroska");
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
	//trackList.set_size_request(450,400);
	trackList.set_border_width(20);
	trackList.set_rules_hint(true);
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

	progressBar.set_show_text(true);

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



void MainWindow::extract() {
	std::map<int, std::string> toExtract;
	std::map<int,bool> tracksToExtract = getUserSelection();
	for (std::map<int, bool>::iterator i = tracksToExtract.begin(); i != tracksToExtract.end(); i++) {
		if (i->second) {
			toExtract[i->first] = getOutputFolder() + "/" + getFileName(i->first);
		}
	}

	int master;
	setExtractionProcessPID(forkpty(&master, NULL, NULL, NULL));
	if (getExtractionProcessPID() == 0) { // child process
		close(master);

		Core::MkvExtractor::extractTracks(getInputFileName(), toExtract);

	} else { // parent process
		setExtractionStatus(MainWindow::extracting_status);
		std::cout << "Command line used : "<< std::endl;
		std::cout << "-------------------- "<< std::endl;
		std::cout << Core::MkvExtractor::getExtractCommandLine(getInputFileName(), toExtract) << std::endl;

		std::string str;
		while (getLine(master, str)) {
			int progress = 0;
			int ret = sscanf(str.c_str(), "%*[^:] : %d", &progress);

			if (ret > 0) {
				setProgressPercentage(progress);
			}
			std::cout << str;

		}
		setExtractionStatus(MainWindow::stop_status);
	}
}

void* MainWindow::extractThread_fun(void* thisWindow) {
	MainWindow *win = (MainWindow*) thisWindow;
	win->extract();
	return 0;
}

std::string MainWindow::getInputFileName(){
	return this->inputFileButton.get_filename();
}

std::string MainWindow::getFileName(int id) {
	Gtk::TreeModel::Path path(Core::toString(id));
	Gtk::TreeModel::Row row = *(refListStore->get_iter(path));
	std::string name;
	row.get_value(5, name);
	return name;
}

bool MainWindow::stopExtraction() {

	Gtk::MessageDialog dialog(*this, "Extraction process is currently running.",
			false /* use_markup */, Gtk::MESSAGE_QUESTION,
			Gtk::BUTTONS_OK_CANCEL);

	dialog.set_secondary_text("Are you sure you want to cancel the extraction ?");
	int result = dialog.run();

	switch (result) {
	case (Gtk::RESPONSE_OK):
		kill(this->extractionProcess_pid, SIGKILL);
		disableTimer();
		setExtractionStatus(stop_status);
		onExtractionEnd(false);
		return true;
		break;
	case (Gtk::RESPONSE_CANCEL):
		break;
	default:
		break;
	}
	return false;
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
    gettimeofday(&lastStartTime, 0);
	enableTimer();
}

void MainWindow::startExtraction() {
	extractOrPauseButton.set_label("Pause");
	cancelButton.set_sensitive(true);
	extractOrPauseButton.set_image(*Gtk::manage (new Gtk::Image (Gtk::Stock::MEDIA_PAUSE, Gtk::ICON_SIZE_BUTTON)));
	pthread_create(&extraction_thread, 0, &extractThread_fun, (void*) this);
	gettimeofday(&lastStartTime, 0);
	time_elapsed = 0;
	enableTimer();
}



void MainWindow::enableTimer() {
	con = Glib::signal_timeout().connect(sigc::mem_fun(*this, &MainWindow::onTimeOut), 500);
}

void MainWindow::disableTimer() {
	con.disconnect();
}

bool MainWindow::onTimeOut() {
	bool ret = true;
	switch(current_state) {
	case paused_status:
		ret = false; // disconnect timer
		break;
	case extracting_status:
		updateProgress(); // return true = continue timer
		break;
	case stop_status:
		onExtractionEnd(true);
		ret = false; // disconnect timer

		Gtk::MessageDialog dialog(*this, "Extraction success !");
		dialog.run();
		break;
	}
	return ret;
}


std::string readableTime(int time_in_sec) {
	std::string ret ="";
	int hour, min, seconds;
	hour = time_in_sec / 3600;
	seconds = time_in_sec % 3600;
	min = seconds / 60;
	seconds = seconds % 60;
	if (hour != 0)
		ret += Core::toString(hour) + "h ";
	if (min != 0)
		ret += Core::toString(min) + "min ";
	ret += Core::toString(seconds) + "sec";
	return ret;
}

void MainWindow::updateProgress() {
	progressBar.set_fraction((double)progress_percentage / 100.0);
	timeval currentTime;
	gettimeofday(&currentTime, 0);
	time_elapsed += (currentTime.tv_sec - lastStartTime.tv_sec);
	gettimeofday( &lastStartTime, 0);
	lastStartTime = currentTime;

	int remainingTime;
	if (progress_percentage !=0) {
		remainingTime = time_elapsed * (100-progress_percentage) / progress_percentage;
	} else {
		remainingTime = 0;
	}

	progressBar.set_text(Core::toString(progress_percentage)+ "%, Time elapsed = " + readableTime(time_elapsed) +", Time remaining = " + readableTime(remainingTime));
}

void MainWindow::onExtractionEnd(bool extractionSuccess) {
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

		if (!stopExtraction()) {
			return true;
		}
	}
	return false;
}

void MainWindow::onFileSet() {

	trackList.set_sensitive(true);

	tracksToExtract.clear();
	printTracksInfos(Core::MkvInfoParser::parseTracksInfos(getInputFileName()));

	outputFileButton.set_current_folder(dirName(getInputFileName()));
}

void MainWindow::initTime() {
	time_elapsed = 0;
}

std::string MainWindow::getRemainingTime() {
	std::string ret;

	return ret;
}

