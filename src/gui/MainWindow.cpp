

#include "MainWindow.h"
#include <iostream>
#include <sstream>
#include <pthread.h>
#include <gtkmm/filefilter.h>
#include <sys/types.h>
#include <sys/wait.h>

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
//	extractButton.signal_button_press_event().connect(sigc::mem_fun(this, &MainWindow::onExtract));

	mainVBox.pack_start(extractButton, Gtk::PACK_SHRINK);

	this->add(mainVBox);
	extractButton.set_can_focus(false);
	extractButton.set_sensitive(false);

	this->show_all();

	extracting = false;
	pthread_mutex_init(&isExtracting_mutex, 0);

	bool fileChoosen = false;
	bool trackSelected = false;
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

void* extractionThread_fun(void* args) {
	MainWindow *win = (MainWindow*) args;
//	std::vector<int> toExtract;
	std::map<int, std::string> toExtract;
	std::map<int,bool> tracksToExtract = win->getUserSelection();

	win->setExtractionProcessPID(fork());
	if (win->getExtractionProcessPID() == 0) { // child process

		for (std::map<int, bool>::iterator i = tracksToExtract.begin(); i != tracksToExtract.end(); i++) {
			if (i->second) {
				toExtract[i->first] = win->getOutputFolder() + "/" + win->getFileName(i->first);
			}
		}

		win->getMkvExtractor().extractTracks(toExtract);
		std::cout<< "end child process"<< std::endl;
	}
	else { // parent process
		win->setIsExtracting(true);
		pid_t tpid;
		int status;
		do {
			tpid = wait(&status);
		} while (tpid != win->getExtractionProcessPID());
		win->setIsExtracting(false);
	}
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

			// Creation of a new object prevents long lines and shows us a little
			  // how slots work.  We have 0 parameters and bool as a return value
			  // after calling sigc::bind.
//			  sigc::slot<bool> my_slot = sigc::bind(sigc::mem_fun(*this, &MainWindow::onTimeOut), 0);

			  // This is where we connect the slot to the Glib::signal_timeout()
			  sigc::connection conn = Glib::signal_timeout().connect(sigc::mem_fun(*this, &MainWindow::onTimeOut), 500);

			pthread_create(&extraction_thread, 0, &extractionThread_fun, (void*) this);
		}
	} else {
		stopExtraction();
		extractButton.set_label("Extract");
	}

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

