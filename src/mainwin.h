// $Id: mainwin.h,v 1.13 2001/11/23 23:07:29 cactus Exp $ -*- c++ -*-
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

#ifndef GNOMOKU_MAINWIN_H
#define GNOMOKU_MAINWIN_H

#include "point.h"
#include "ai.h"
#include "user.h"
#include "options.h"
#include <gtkmm.h>
#include <gtkmm/window.h>
#include <gtkmm/statusbar.h>

namespace Gnomoku
{
   class MainWin : public Gtk::Window 
   {
		int	rows;
		int	cols;
		Gtk::Box m_box;
		Gtk::Statusbar status;
		Gtk::ProgressBar m_pbar;
		Glib::RefPtr<Gtk::Builder> m_refBuilder;
		Glib::RefPtr<Gio::SimpleActionGroup> m_refActionGroup;
		
	
		
		
		Point     ***tbl;
	
		Opponent    *opponent;
		bool         my_turn;
	
		op_t         optype;
		std::string  server, port;
		bool         beep;
		Options      options_win;
		
		Glib::RefPtr<Gdk::Pixbuf> m_logo;
   
   public:
   MainWin (const Glib::RefPtr<Gtk::Application>& app, int rows = 15, int cols = 15);
	~MainWin ();
	
   private:
	void reset();
	void message(const char *fmt, ...);
	void cleanup();
	void start_game();
	int  tmout();
	int  status_timeout();
	void get_msg();
	void point_pressed(Point *p);
	bool won(int y, int x);

	void exit_cb ();
	void options_cb ();
	void about_cb ();
	sigc::connection status_conn;
   };
}

#endif

