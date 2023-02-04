// $Id: mainwin.cc,v 1.35.2.1 2001/11/28 21:14:47 cactus Exp $
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

#include "mainwin.h"

#include "config.h"
#include <libgnome/libgnome.h>

#include <stdarg.h>
#include <errno.h>
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <list>

#include <gtk--/table.h>
#include <gtk--/main.h>
#include <gtk--/adjustment.h>
#include <gnome--/about.h>
#include <libgnomeui/gnome-uidefs.h>

#define COPYRIGHT "(C) NAGY András, ÉRDI Gergõ"

using namespace Gnomoku;
using std::string;
using std::list;

MainWin::MainWin(int rows_, int cols_):
    App ("main", "Gnomoku"),
    status (true, true, GNOME_PREFERENCES_USER),
    rows (rows_),
    cols (cols_),
    
    opponent (0),
    optype (OP_AI),
    server ("localhost"),
    port ("23776"),
    beep (false),
    options_win (optype, server, port, beep)
{
    // Window
    set_title ("MainWin");
    set_policy (false, false, false);
    destroy.connect (Gtk::Main::instance ()->quit.slot());

    // Status bar
    string greetmsg ((string)"Gnomoku" + " " + VERSION + " -- " + COPYRIGHT);
    status.set_default (greetmsg);
    set_statusbar (status);

    // Menu
    list<Gnome::UI::SubTree> menubar;

    // Game menu
    list<Gnome::UI::Info> m_game;
    m_game.push_back (Gnome::MenuItems::NewGame(
	SigC::slot (this, &MainWin::start_game)));
    m_game.push_back (Gnome::UI::Separator ());
    m_game.push_back (Gnome::MenuItems::Exit (
	SigC::slot (this, &MainWin::exit_cb)));    
    menubar.push_back (Gnome::Menus::Game (m_game));

    // Settings menu
    list<Gnome::UI::Info> m_settings;
    m_settings.push_back (Gnome::MenuItems::Preferences (
	SigC::slot (this, &MainWin::options_cb)));
    menubar.push_back (Gnome::Menus::Settings (m_settings));

    // Help menu
    list<Gnome::UI::Info> m_help;
    m_help.push_back (Gnome::MenuItems::About (
	SigC::slot (this, &MainWin::about_cb)));
    menubar.push_back (Gnome::Menus::Help (m_help));
    
    create_menus (menubar);
    install_menu_hints ();
    
    // Table
    Gtk::Table* table = new Gtk::Table (rows, cols, true);
    Point *p;
    tbl = new (Point**)[rows];
    for (int y=0; y<rows; y++) {
	tbl[y] = new (Point*)[cols];
	for (int x=0; x<cols; x++) {
	    p = new Point(y,x);
	    tbl[y][x] = p;
	    p->clicked.connect(
		SigC::bind(SigC::slot(this, &MainWin::point_pressed), p));
	    table->attach(*p, x, x+1, y, y+1, GTK_EXPAND, GTK_EXPAND);
	}
    }
    table->set_border_width (GNOME_PAD);
    table->show_all ();
    
    set_contents (*manage (table));

    // Options dialog
    options_win.set_transient_for (*this);
    options_win.set_modal (true);
    options_win.changed.connect (SigC::slot (this, &MainWin::reset));

    // tmout
    Gtk::Main::timeout.connect (slot(this, &MainWin::tmout), 100);
}

MainWin::~MainWin()
{
    if (opponent) {
	delete opponent;
	opponent = 0;
    }
    
    for (int y=0; y<rows; y++) {
	for (int x=0; x<cols; x++) {
	    delete tbl[y][x];
	}
	delete tbl[y];
    }
    delete tbl;
}

gint MainWin::status_timeout()
{
    static gfloat low_bound  = status.get_progress ()->get_adjustment ()->get_lower ();
    static gfloat high_bound = status.get_progress ()->get_adjustment ()->get_upper ();
    static gfloat i = low_bound;

    if (i++ == high_bound)
	i = low_bound;

    status.get_progress()->set_value(i);
    
    return true;
}

void MainWin::reset()
{
    if (opponent) {
	delete opponent;
	opponent = 0;
	cleanup();
	message(_("Game reset"));
    }
}

#define MAX_MSG_LEN 256
void MainWin::message(const char *fmt, ...)
{
    va_list ap;
    char text[MAX_MSG_LEN];
    
    va_start(ap, fmt);
    vsnprintf(text, MAX_MSG_LEN, fmt, ap);
    
    status.pop();
    status.push(text);
    
    va_end(ap);
}

void MainWin::cleanup()
{
    for (int y=0; y<rows; y++)
	for (int x=0; x<cols; x++) {
	    tbl[y][x]->set(0);
	    tbl[y][x]->set_blink(false);
	}
    my_turn = true;
}

void MainWin::start_game()
{
    msg_t msg;

    cleanup();
    
    if (opponent) {
	if (opponent->ok()) {
	    msg.type = MSG_START;
	    opponent->put_msg(msg);
	    message(_("Game restarted"));
	}
    } else {
	if (optype==OP_AI) {
	    opponent = new AI(rows, cols);
	} else {
	    message(_("Estabilishing connection..."));
	    status.get_progress()->set_activity_mode(true);
	    if (!status_conn.connected ())
		status_conn = Gtk::Main::timeout.connect (slot (this, &MainWin::status_timeout), 50);
	    opponent = new User(optype, server, port);
	}
    }
}

gint MainWin::tmout()
{
    if (opponent) {
	if (int err=opponent->err()) {
	    message(_("ERROR: %s"), strerror(err));
	    status_conn.disconnect();
	    status.get_progress()->set_activity_mode(false);
	    status.get_progress()->set_value(0);
	    delete opponent;
	    opponent = 0;
	} else {
	    if ((opponent->ok()) &&
		(opponent->ready())) get_msg();
	}
    }
    return true;
}

void MainWin::get_msg()
{
    msg_t msg;

    opponent->get_msg(msg);
    switch (msg.type) {
    case MSG_START:
	message(_("Your opponent restarted the game"));
	cleanup();
	break;
    case MSG_PUT:
	if ((msg.y>=0) && (msg.y<rows) &&
	    (msg.x>=0) && (msg.x<cols) &&
	    !tbl[msg.y][msg.x]->get())
	{
	    tbl[msg.y][msg.x]->set(2);
	    tbl[msg.y][msg.x]->grab_focus();
	    if (!won(msg.y, msg.x)) {
		my_turn = true;
		message(_("It's your turn"));
		if (beep) gdk_beep();
	    }
	}
	break;
    case MSG_CLOSE:
	message(_("Your party closed the connection"));
	delete opponent;
	opponent = 0;
	break;
    case MSG_GREET:
	message(_("Your opponent is %s"), msg.ident.c_str());
	status_conn.disconnect();
	status.get_progress()->set_activity_mode(false);
	status.get_progress()->set_value(0);
	break;
    }
}

void MainWin::point_pressed(Point *p)
{
    msg_t msg;
    
    if (opponent && opponent->ok() && my_turn && !p->get()) {
	msg.type = MSG_PUT;
	msg.y = p->gety();
	msg.x = p->getx();
	opponent->put_msg(msg);
	p->set(1);
	if (!won(msg.y, msg.x)) {
	    my_turn = false;
	    message(_("Waiting for your party to respond"));
	}
    }
}

bool MainWin::won(int y, int x)
{
    int who=tbl[y][x]->get();
    bool wins=false;
    int pieces[4];
    Point *points[4][5];
    int seq=0;

    for (int i=0; i<4; i++) pieces[i] = 0;

    for (int ay=-1; ay<=1; ay++)
	for (int ax=-1; ax<=1; ax++)
	    if (ay || ax) {
		int ny=y, nx=x;
		while (((ny+=ay)>=0) && ((nx+=ax)>=0) &&
		       (ny<rows) && (nx<cols) &&
		       tbl[ny][nx]->get()==who)
		{
		    int s = (seq<4 ? seq : 7-seq);
		    points[s][++pieces[s]] = tbl[ny][nx];
		}
		seq++;
	    }

    for (int i=0; i<4; i++)
	if (pieces[i]>=4) {
	    wins=true;
	    points[i][0] = tbl[y][x];
	    for (int p=0; p<5; p++)
		points[i][p]->set_blink(true);
	    opponent->won();
	    
	    if (who==1)
		message(_("You won the game"));
	    else
		message(_("Your opponent won the game"));
	    my_turn=false;
	}

    return wins;
}

void MainWin::exit_cb ()
{
    Gnome::Dialog* dialog = Gnome::Dialogs::question_modal
	(_("Do you really want to quit the game?"), 0);
    if (!dialog->run ())
	destroy ();
}

void MainWin::options_cb ()
{
    options_win.show ();
}

void MainWin::about_cb ()
{
    list<string> authors;
    authors.push_back ("András Nagy <nagya@telnet.hu>");
    authors.push_back ("Gergõ Érdi <cactus@cactus.rulez.org>");
    authors.push_back ("Daniel Elstner <daniel.elstner@gmx.net>");

    Gnome::About* about = new Gnome::About ("Gnomoku", VERSION,
					    COPYRIGHT,
					    authors,
					    _("Gomoku game for GNOME"));
    about->run();
}
