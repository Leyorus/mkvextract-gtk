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


#include <gtkmm/main.h>
#include <iostream>
//#include <gtkmm.h>

#include "MainWindow.h"

int main(int argc, char *argv[]) {
    Gtk::Main app(argc, argv);

    MainWindow window;
//    fenetre.set_title("MkvExtract-Gtk");
//
//    Gtk::VBox mainVBox(false, 10);
//
//    Gtk::Frame inputFrame("Input file");
//    Gtk::FileChooserButton inputFileButton(Gtk::FILE_CHOOSER_ACTION_OPEN);
//    inputFrame.add(inputFileButton);
//    mainVBox.pack_start(inputFrame, Gtk::PACK_SHRINK);
//
//    inputFileButton.signal_file_set().connect(sigc::ptr_fun(&fileSet));
//
//    Gtk::Frame outputFrame("Output folder");
//    Gtk::FileChooserButton outputFileButton(Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
//    outputFrame.add(outputFileButton);
//    mainVBox.pack_start(outputFrame, Gtk::PACK_SHRINK);
//
//    Gtk::Frame contentFrame("Content");
//    mainVBox.pack_start(contentFrame, Gtk::PACK_EXPAND_WIDGET);
//
//    Glib::RefPtr<Gtk::ListStore> refListStore = Gtk::ListStore::create(m_Columns);
//
//    Gtk::TreeView trackList(refListStore);
//    trackList.set_size_request(350,150);
//    contentFrame.add(trackList);
//    trackList.append_column_editable("", m_Columns.m_col_selected);
//    trackList.append_column("ID", m_Columns.m_col_id);
//    trackList.append_column("Type", m_Columns.m_col_type);
//    trackList.append_column("Codec", m_Columns.m_col_codec);
//    trackList.append_column("Language", m_Columns.m_col_language);
//    trackList.append_column_editable("Output filename", m_Columns.m_col_outputFileName);
//
//    Gtk::TreeModel::Row row = *(refListStore->append());
//     row[m_Columns.m_col_selected] = true;
//     row[m_Columns.m_col_id] = 1;
//     row[m_Columns.m_col_type] = "video";
//     row[m_Columns.m_col_codec] = "A_DTS";
//     row[m_Columns.m_col_language] = "fre";
//     row[m_Columns.m_col_outputFileName] = "Track1_fre.dts";
////     row[m_Columns.m_col_number] = 10;
//
////    trackList.set_column_title(0, "");
////    trackList.set_column_title(1, "ID");
////    trackList.set_column_title(2, "Type");
////    trackList.set_column_title(3, "Codec");
////    trackList.append("salut les gens");
//
//    Gtk::Button extractButton("Extract");
//    mainVBox.pack_start(extractButton, Gtk::PACK_SHRINK);
//
//    fenetre.add(mainVBox);
//
//    extractButton.set_can_focus(false);
//
//    fenetre.show_all();

    Gtk::Main::run(window);

    return 0;
}
