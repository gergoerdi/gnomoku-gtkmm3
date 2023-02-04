// $Id: options.h,v 1.16 2001/11/23 23:07:29 cactus Exp $ -*- c++ -*-
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

#ifndef GNOMOKU_OPTIONS_H
#define GNOMOKU_OPTIONS_H

#include "opponent.h"

#include <gnome--/dialog.h>
#include <gtk--/label.h>
#include <gtk--/optionmenu.h>
#include <gtk--/entry.h>
#include <gtk--/checkbutton.h>

namespace Gnomoku
{
    class Options : public Gnome::Dialog
    {
	Gtk::Label       op_label;
	Gtk::OptionMenu  op_combo;
	
	Gtk::Label       host_label, port_label;
	Gtk::Entry       host_entry, port_entry;
	
	Gtk::CheckButton beep_check;
	
	op_t        &optype, optype_cache;
	std::string &host, &port;
	bool        &beep;
	
    public:
	Options(op_t        &optype,
		std::string &host, std::string &port,
		bool        &beep);
	
	SigC::Signal0<void> changed;
	
    private:
	void update();
	void apply_op (op_t op);
	
	void op_change (op_t op);
	void button_clicked (int button);
	int  delete_event_impl(GdkEventAny *);
    };
}

#endif
