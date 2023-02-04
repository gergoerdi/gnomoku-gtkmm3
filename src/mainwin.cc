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

#include <cstdarg>
#include <cerrno>
//#define _GNU_SOURCE
#include <cstdio>
#include <string>
#include <cstring>
#include <vector>

#include <gtkmm/table.h>
#include <gtkmm/main.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/aboutdialog.h>
//#include <libgnomeui/gnome-uidefs.h>

#include "pic_gnomoku.h"

#define VERSION "1.4"
#define COPYRIGHT "(C) NAGY András, ÉRDI Gergõ"

using namespace Gnomoku;
using std::string;
using std::vector;


MainWin::MainWin(const Glib::RefPtr<Gtk::Application>& app, int rows_, int cols_):
    rows (rows_),
    cols (cols_),
    m_box(Gtk::ORIENTATION_VERTICAL),
    opponent (0),
    optype (OP_AI),
    server ("localhost"),
    port ("23776"),
    beep (false),
    options_win (optype, server, port, beep)

{
    // Window
    set_title ("MainWin");

    //set_policy (false, false, false);
    //destroy.connect (Gtk::Main::instance ()->quit.slot());

    add(m_box);

	//Logo
	try
	{
          m_logo = Gdk::Pixbuf::create_from_inline (-1, pic_gnomoku, false);
	}
	catch (const Glib::FileError& ex)
	{
	  std::cerr << "FileError: " << ex.what() << std::endl;
	}
	catch(const Gdk::PixbufError& ex)
	{
	  std::cerr << "PixbufError: " << ex.what() << std::endl;
	}

    // Status bar
    std::string greetmsg = (std::string("Gnomoku ") + VERSION + " -- " + COPYRIGHT);
    status.push(greetmsg);
	status.pack_start(m_pbar);
    //add (status);

    //Define the actions:
    m_refActionGroup = Gio::SimpleActionGroup::create();

    m_refActionGroup->add_action("NewGame",
    sigc::mem_fun(*this, &MainWin::start_game) );

    m_refActionGroup->add_action("quit",
    sigc::mem_fun(*this, &MainWin::exit_cb) );

    m_refActionGroup->add_action("preference",
    sigc::mem_fun(*this, &MainWin::options_cb) );

    m_refActionGroup->add_action("About",
    sigc::mem_fun(*this, &MainWin::about_cb) );

    insert_action_group("main_Game", m_refActionGroup);


	m_refBuilder = Gtk::Builder::create();

       //Layout the actions in a menubar:
    const char* ui_info =
    "<interface>"
    "  <menu id='menubar'>"
    "    <submenu>"
    "      <attribute name='label' translatable='yes'>_Game</attribute>"
    "      <section>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>_NewGame</attribute>"
	"          <attribute name='action'>main_Game.NewGame</attribute>"
    "          <attribute name='accel'>&lt;Primary&gt;n</attribute>"
    "        </item>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>_Quit</attribute>"
    "          <attribute name='action'>main_Game.quit</attribute>"
    "          <attribute name='accel'>&lt;Primary&gt;o</attribute>"
    "        </item>"
    "      </section>"
    "    </submenu>"
    "    <submenu>"
    "      <attribute name='label' translatable='yes'>_Settings</attribute>"
    "      <item>"
    "        <attribute name='label' translatable='yes'>_Preferences</attribute>"
    "        <attribute name='action'>main_Game.preference</attribute>"
    "        <attribute name='accel'>&lt;Primary&gt;x</attribute>"
    "      </item>"
    "    </submenu>"
    "    <submenu>"
    "	   <attribute name='label' translatable='yes'>_Help</attribute>"
    "      <item>"
    "        <attribute name='label' translatable='yes'>_About</attribute>"
    "        <attribute name='action'>main_Game.About</attribute>"
    "        <attribute name='accel'>&lt;Primary&gt;h</attribute>"
    "      </item>"
    "    </submenu>"
    "  </menu>"
    "</interface>";



	try
	{
		m_refBuilder->add_from_string(ui_info);
    }
    catch(const Glib::Error& ex)
    {
		std::cerr << "Building menus failed: " <<  ex.what();
	}


	//Get the menubar:
	auto object = m_refBuilder->get_object("menubar");
	auto gmenu = Glib::RefPtr<Gio::Menu>::cast_dynamic(object);
	if (!gmenu)
		g_warning("GMenu not found");
	else
	{
		auto pMenuBar = Gtk::make_managed<Gtk::MenuBar>(gmenu);

		//Add the MenuBar to the window:
		m_box.pack_start(*pMenuBar, Gtk::PACK_SHRINK);
	}

/*
   // Menu
   install_menu_hints ();
*/

   // Table
	Gtk::Table* table = new Gtk::Table (rows, cols, true);
	Point *p;
	tbl = new Point**[rows];
	for (int y=0; y<rows; y++)
	{
		tbl[y] = new Point*[cols];

		for(int x=0; x<cols; x++)
		{
			p = new Point(y,x);
			tbl[y][x] = p;
			p->signal_clicked().connect(
			sigc::bind<Point*>(sigc::mem_fun(this, &MainWin::point_pressed), p));
			table->attach(*p, x, x+1, y, y+1, Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND | Gtk::FILL);
		}
	}
    table->set_border_width (2);
    table->show_all ();

	Gtk::manage (table);
	m_box.pack_start(*table);
	m_box.pack_start (status);
    //add(table);

    // Options dialog
    options_win.set_transient_for (*this);
    options_win.changed.connect (sigc::mem_fun (this, &MainWin::reset));

    // tmout
    //Gtk::Main::timeout.connect (sigc::slot(this, &MainWin::tmout), 100);
	Glib::signal_timeout().connect(sigc::mem_fun(this, &MainWin::tmout), 100);

	show_all_children();
}

MainWin::~MainWin()
{
   if (opponent)
   {
	delete opponent;
	opponent = nullptr;
   }

   for (int y=0; y<rows; y++)
   {
		for (int x=0; x<cols; x++) {
			delete tbl[y][x];
		}
	delete tbl[y];
   }
   delete tbl;
}

int MainWin::status_timeout()
{
   //static gfloat low_bound  = status.get_progress ()->get_adjustment ()->get_lower ();
  static double low_bound  = m_pbar.get_fraction ();
  // static gfloat high_bound = status.get_progress ()->get_adjustment ()->get_upper ();
  static double high_bound = m_pbar.get_fraction ();
   static double i = low_bound;

   if (i++ == high_bound)
	i = low_bound;

   m_pbar.set_fraction (i);

   return true;
}

void MainWin::reset()
{
    if (opponent) {
	delete opponent;
	opponent = nullptr;
	cleanup();
	message(("Game reset"));
    }
}

const unsigned int max_msg_len =  256;
void MainWin::message(const char *fmt, ...)
{
    va_list ap;
    char text[max_msg_len];

    va_start(ap, fmt);
    vsnprintf(text, max_msg_len, fmt, ap);

    status.pop();
    status.push(text);

    va_end(ap);
}

void MainWin::cleanup()
{
	for (int y=0; y<rows; y++)
	{
		for(int x=0; x<cols; x++)
		{
			tbl[y][x]->set(0);
			tbl[y][x]->set_blink(false);
		}
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
	    message(("Game restarted"));
	}
    } else {
	if (optype==OP_AI) {
	    opponent = new AI(rows, cols);
	} else {
	    message(("Estabilishing connection..."));
	    //status.get_progress()->set_activity_mode(true);
		m_pbar.set_visible (true);
	    if (!status_conn.connected ())
		status_conn = Glib::signal_timeout().connect (sigc::mem_fun (this, &MainWin::status_timeout), 50);
	    opponent = new User(optype, server, port);
	}
   }
}

gint MainWin::tmout()
{
   if (opponent) {
	if (int err=opponent->err()) {
	    message(("ERROR: %s"), strerror(err));
	    status_conn.disconnect();
	    //status.get_progress()->set_activity_mode(false);
		m_pbar.set_visible (true);
	    //status.get_progress()->set_value(0);
		m_pbar.set_fraction (0);
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
		message(("Your opponent restarted the game"));
		cleanup();
		break;
   case MSG_PUT:
		if ((msg.y>=0) && (msg.y<rows) &&
	   (msg.x>=0) && (msg.x<cols) &&
	   !tbl[msg.y][msg.x]->get())
		{
			tbl[msg.y][msg.x]->set(2);
			tbl[msg.y][msg.x]->grab_focus();
			if (!won(msg.y, msg.x))
			{
			my_turn = true;
			message(("It's your turn"));
			//if (beep) gdk_beep();
		   if (beep) gdk_display_beep(	gdk_display_get_default ());
			}
		}
		break;
   case MSG_CLOSE:
		message(("Your party closed the connection"));
		delete opponent;
		opponent = 0;
		break;
   case MSG_GREET:
		message(("Your opponent is %s"), msg.ident.c_str());
		status_conn.disconnect();
		//status.get_progress()->set_activity_mode(false);
		m_pbar.set_visible (false);
		//status.get_progress()->set_value(0);
		m_pbar.set_fraction (0);
		break;
   }
}

void MainWin::point_pressed(Point *p)
{
   msg_t msg;

   if (opponent && opponent->ok() && my_turn && !p->get())
   {
		msg.type = MSG_PUT;
		msg.y = p->gety();
		msg.x = p->getx();
		opponent->put_msg(msg);
		p->set(1);
		if (!won(msg.y, msg.x))
		{
			my_turn = false;
			message(("Waiting for your party to respond"));
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
	   if (ay || ax)
	   {
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
			message(("You won the game"));
	   else
			message(("Your opponent won the game"));
			my_turn=false;
	}

   return wins;
}

void MainWin::exit_cb ()
{
   Gtk::MessageDialog dialog(("Do you really want to quit the game?"), 0, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO, 0);
   if (dialog.run())
	   destroy_();
}

void MainWin::options_cb ()
{
    options_win.show ();
}

void MainWin::about_cb ()
{
    vector<Glib::ustring> authors;
    authors.push_back ("András Nagy <nagya@telnet.hu>");
    authors.push_back ("Gergõ Érdi <cactus@cactus.rulez.org>");
    authors.push_back ("Daniel Elstner <daniel.elstner@gmx.net>");

   Gtk::AboutDialog about;
   about.set_logo(m_logo);
   about.set_program_name("Gnomoku");
   about.set_version ("1.4");
   about.set_copyright (COPYRIGHT);
   about.set_authors(authors);
   about.set_comments ("Gomoku game for GNOME");
   about.run();
}
