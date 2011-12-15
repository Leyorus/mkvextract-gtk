

#ifndef DEF_MAIN_WINDOW
#define DEF_MAIN_WINDOW

#include <gtkmm/main.h>
#include <gtkmm/window.h>
#include <gtkmm/button.h>
#include <gtkmm/stock.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/box.h>
#include <gtkmm/frame.h>
#include <gtkmm/filechooserbutton.h>
#include <gtkmm/listviewtext.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>

#include <MkvExtractor.h>
#include <string>
#include <vector>
#include <map>
#include <pthread.h>


class ModelColumns: public Gtk::TreeModelColumnRecord {
public:

	ModelColumns() {
		add(m_col_selected);
		add(m_col_id);
		add(m_col_type);
		add(m_col_codec);
		add(m_col_language);
		add(m_col_outputFileName);
	}

	Gtk::TreeModelColumn<bool> m_col_selected;
	Gtk::TreeModelColumn<int> m_col_id;
	Gtk::TreeModelColumn<Glib::ustring> m_col_type;
	Gtk::TreeModelColumn<Glib::ustring> m_col_codec;
	Gtk::TreeModelColumn<Glib::ustring> m_col_language;
	Gtk::TreeModelColumn<Glib::ustring> m_col_outputFileName;
};

class MainWindow: public Gtk::Window {
public:
	MainWindow();
	MkvExtractor getMkvExtractor(){ return this->mkvExtractor;}
	std::map<int,bool> getUserSelection() {return this->tracksToExtract;}
	bool isExtracting();
	void setIsExtracting(bool isExtracting);
	void setExtractionProcessPID(int PID) { extractionProcess_pid = PID;};
	int getExtractionProcessPID() { return extractionProcess_pid;};
	std::vector<track_info_t> tracks;
	std::string getFileName(int id);
	std::string getOutputFolder() { return outputFileButton.get_current_folder();};
	bool onTimeOut();
private:
	Gtk::Window window;
	Gtk::VBox mainVBox;
	Gtk::Frame inputFrame;
	Gtk::FileChooserButton inputFileButton;
	Gtk::Frame outputFrame;
	Gtk::FileChooserButton outputFileButton;
	Gtk::Frame contentFrame;
	ModelColumns m_Columns;
	Glib::RefPtr<Gtk::ListStore> refListStore;
	Gtk::TreeView trackList;
	Gtk::Button extractButton;

	MkvExtractor mkvExtractor;
	std::map<int,bool> tracksToExtract;

	bool extracting;
	pthread_mutex_t isExtracting_mutex;
	pthread_t extraction_thread;
	pid_t extractionProcess_pid;

	bool fileChoosen;
	bool trackSelected;

	void stopExtraction();
    void fileSet();
	void printTracksInfos(std::vector<track_info_t> & tracks);
	void onCheckboxClicked(Glib::ustring path);
	void onExtractButton();
	bool onCloseButton(GdkEventAny * ev);
	bool isATrackSelected();
};

#endif
