// $Id: options.cc,v 1.32 2001/11/23 23:07:29 cactus Exp $
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

#include "options.h"

#include "config.h"
#include <libgnome/libgnome.h>

#include <vector>

#include <gtk--/menu.h>
#include <gtk--/table.h>
#include <gtk--/box.h>
#include <gnome--/stock.h>

using namespace Gnomoku;
using std::string;

Options::Options(op_t &optype_,
		 string &host_, string &port_,
		 bool &beep_):
    
    op_label (_("Opponent type:"), 0, 0.5),
    host_label (_("Host name:"),   0, 0.5),
    port_label (_("Port number:"), 0, 0.5),

    beep_check (_("Beep on my turn")),
    
    optype(optype_),
    host (host_),
    port (port_),
    beep(beep_)
{
    set_title(_("Options"));
    set_border_width(10);
    set_policy(false, false, false);
    set_wmclass("options", "Gnomoku");
    set_position(GTK_WIN_POS_MOUSE);
    show.connect(slot(this, &Options::update));
    
    // Text entries
    host_entry.set_max_length(40);
    port_entry.set_max_length(5);

    // Dropdown list
    Gtk::Menu     *menu = new Gtk::Menu;
    Gtk::MenuItem *item;
    item = new Gtk::MenuItem (_("Computer AI"));
    item->activate.connect (SigC::bind (SigC::slot (this, &Options::op_change), OP_AI));
    menu->append (*manage (item));
    
    item = new Gtk::MenuItem(_("Net, I'm the client"));
    item->activate.connect (SigC::bind (SigC::slot (this, &Options::op_change), OP_CLIENT));
    menu->append (*manage (item));
    
    item = new Gtk::MenuItem(_("Net, I'm the server"));
    item->activate.connect (SigC::bind (SigC::slot (this, &Options::op_change), OP_SERVER));
    menu->append (*manage (item));

    menu->show_all();
    op_combo.set_menu(*manage (menu));
    
    // Beep check button
    Gtk::Label *label = static_cast<Gtk::Label*> (beep_check.get_child ());
    label->set_alignment (0, 0.5);
    
    Gtk::Table* table = new Gtk::Table (4, 3, false);
    table->set_row_spacings (10);
    table->set_col_spacings (10);

    table->attach (op_label, 0, 1, 0, 1);
    table->attach (op_combo, 1, 2, 0, 1);

    table->attach (host_label, 0, 1, 1, 2);
    table->attach (host_entry, 1, 2, 1, 2);
    table->attach (port_label, 0, 1, 2, 3);
    table->attach (port_entry, 1, 2, 2, 3);

    table->attach (beep_check, 0, 2, 3, 4);

    // Buttons
    append_button (_("OK"),     GNOME_STOCK_BUTTON_OK);
    append_button (_("Cancel"), GNOME_STOCK_BUTTON_CANCEL);
    set_default (0);
    set_close (true);
    clicked.connect (SigC::slot (this, &Options::button_clicked));    

    table->show_all ();
    get_vbox ()->add (*table);
}

void Options::update()
{
    switch (optype)
    {
    case OP_AI:
	op_combo.set_history(0);
	break;
    case OP_CLIENT:
	op_combo.set_history(1);
	break;
    case OP_SERVER:
	op_combo.set_history(2);
	break;
    }
    
    host_entry.set_text (host);
    port_entry.set_text (port);
    beep_check.set_active (beep);
    apply_op (optype);
}

void Options::button_clicked (int button)
{
    if (button == 0)
    {
	optype = optype_cache;
	host = host_entry.get_text ();
	port = port_entry.get_text ();
	beep = beep_check.get_active ();
	changed();
    }
}

void Options::apply_op (op_t op)
{
    switch (op)
    {
    case OP_AI:
	host_label.set_sensitive (false);
	host_entry.set_sensitive (false);

	port_label.set_sensitive (false);
	port_entry.set_sensitive (false);
	break;
    case OP_SERVER:
	host_label.set_sensitive (false);
	host_entry.set_sensitive (false);

	port_label.set_sensitive (true);
	port_entry.set_sensitive (true);
	break;
    case OP_CLIENT:
	host_label.set_sensitive (true);
	host_entry.set_sensitive (true);

	port_label.set_sensitive (true);
	port_entry.set_sensitive (true);
	break;
    }
}

void Options::op_change (op_t op)
{
    apply_op (op);
    optype_cache = op;
}

int Options::delete_event_impl (GdkEventAny *)
{
    hide();
    return false;
}
