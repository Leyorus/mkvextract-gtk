

#include "MainWindow.h"
#include <iostream>
#include <sstream>
#include <pthread.h>

using namespace std;


MainWindow::MainWindow() :
			mainVBox(false, 10),
			inputFrame("Input file"),
			inputFileButton(Gtk::FILE_CHOOSER_ACTION_OPEN),
			outputFrame("Output folder"),
			outputFileButton(Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER),
			contentFrame("Content"),
			extractButton("Extract")
{

	this->set_title("MkvExtract-Gtk");

	inputFrame.add(inputFileButton);
	mainVBox.pack_start(inputFrame, Gtk::PACK_SHRINK);

	inputFileButton.signal_file_set().connect(sigc::mem_fun(this, &MainWindow::fileSet));

	outputFrame.add(outputFileButton);
	mainVBox.pack_start(outputFrame, Gtk::PACK_SHRINK);


	mainVBox.pack_start(contentFrame, Gtk::PACK_EXPAND_WIDGET);

	contentFrame.add(trackList);

	 //Create the Tree model:
	refListStore = Gtk::ListStore::create(m_Columns);
	trackList.set_model(refListStore);
	trackList.set_size_request(400,150);
	trackList.append_column_editable("", m_Columns.m_col_selected);

	((Gtk::CellRendererToggle *) trackList.get_column_cell_renderer(0))->signal_toggled().connect(
			sigc::mem_fun(this, &MainWindow::onEditingStarted));

	trackList.append_column("ID", m_Columns.m_col_id);
	trackList.append_column("Type", m_Columns.m_col_type);
	trackList.append_column("Codec", m_Columns.m_col_codec);
	trackList.append_column("Language", m_Columns.m_col_language);
	trackList.append_column_editable("Output filename", m_Columns.m_col_outputFileName);

	extractButton.signal_clicked().connect(sigc::mem_fun(this, &MainWindow::onExtract));
//	extractButton.signal_button_press_event().connect(sigc::mem_fun(this, &MainWindow::onExtract));

	mainVBox.pack_start(extractButton, Gtk::PACK_SHRINK);

	this->add(mainVBox);
	extractButton.set_can_focus(false);

	this->show_all();

	extracting = false;
	pthread_mutex_init(&isExtracting_mutex, 0);
}

void MainWindow::printTracksInfos(std::vector<track_info_t> & tracks) {
	refListStore->clear();
    std::cout << "Number of tracks = " << tracks.size() << std::endl;
    for(std::vector<track_info_t>::iterator i = tracks.begin();i != tracks.end();i++){

    	Gtk::TreeModel::Row row = *(refListStore->append());
    	row[m_Columns.m_col_selected] = false;
    	row[m_Columns.m_col_id] = toInteger(i->num);
    	row[m_Columns.m_col_type] = i->type;
    	row[m_Columns.m_col_codec] = i->codec;
    	row[m_Columns.m_col_language] = i->language;
    	row[m_Columns.m_col_outputFileName] = "Track" + i->num + "_" + i->language;

    	tracksToExtract[toInteger(i->num)] = false;

        std::cout << i->num << " : ";
        std::cout << i->type << " : ";
        std::cout << i->language << " : ";
        std::cout << i->codec << std::endl;
    }
}

std::string dirName(std::string source)
{
    source.erase(std::find(source.rbegin(), source.rend(), '/').base(), source.end());
    return source;
}

void MainWindow::onEditingStarted(Glib::ustring str) {
	std::cout << "checkbox" << str << " ";
	Gtk::TreeModel::Path path(str.c_str());
	Gtk::TreeModel::Row row = *(refListStore->get_iter(path));
	tracksToExtract[row[m_Columns.m_col_id]] = !tracksToExtract[row[m_Columns.m_col_id]];
	std::cout << "ID=" << row[m_Columns.m_col_id] <<std::endl;
}

void* test(void*) {

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
	std::vector<int> toExtract;
	std::map<int,bool> tracksToExtract = win->getUserSelection();

	for (std::map<int,bool>::iterator i= tracksToExtract.begin(); i != tracksToExtract.end(); i++) {
		if (i->second) {
			toExtract.push_back(i->first);
		}
	}
	win->getMkvExtractor().extractTracks(toExtract);
	win->setIsExtracting(false);
}

void MainWindow::onExtract() {
	pthread_mutex_lock(&this->isExtracting_mutex);
	if(! extracting) {
		extracting = true;
		pthread_mutex_unlock(&this->isExtracting_mutex);
		pthread_create(&extraction_thread, 0, &extractionThread_fun, (void*)this);
	}
	else {
		pthread_mutex_unlock(&this->isExtracting_mutex);
	}
}

void MainWindow::fileSet() {
	std::cout << "File choosen is " << inputFileButton.get_filename() << std::endl;

	mkvExtractor = MkvExtractor(inputFileButton.get_filename());

	tracksToExtract.clear();
	tracks = mkvExtractor.getTracksInfos();
	printTracksInfos(tracks);

	outputFileButton.set_current_folder(dirName(inputFileButton.get_filename().c_str()));
}

