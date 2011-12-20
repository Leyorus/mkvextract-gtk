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

#include <iostream>
#include <gtkmm/main.h>
#include <gtkmm/settings.h>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    Gtk::Main app(argc, argv);

    MainWindow window;
	Glib::RefPtr<Gtk::Settings> settings = Gtk::Settings::get_default();
	/* force using icons on stock buttons: */
	settings->property_gtk_button_images() = true; ;

    Gtk::Main::run(window);

    return 0;
}
