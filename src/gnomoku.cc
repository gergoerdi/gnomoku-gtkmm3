// $Id: gnomoku.cc,v 1.22 2001/11/23 23:07:29 cactus Exp $
/*
  Gnomoku Copyright (C) 1998-1999 NAGY András <nagya@telnet.hu>
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2
  as published by the Free Software Foundation.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "config.h"

#include <gtkmm.h>
#include <cstdlib>
#include <ctime>
#include "mainwin.h"

// Main program
int main(int argc, char **argv)
{
   // bindtextdomain(PACKAGE, GNOMELOCALEDIR);
   // textdomain(PACKAGE);

   // Gtk::Main m(PACKAGE, VERSION, argc, argv);

  auto app =  Gtk::Application::create(argc, argv,
      "org.gtkmm.examples.base");
    Gnomoku::MainWin w(app);
    std::srand(time(NULL));

    w.show();
    return app->run(w);

    //return 0;
}
