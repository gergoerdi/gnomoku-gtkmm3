// $Id: point.h,v 1.10 2001/11/23 23:07:29 cactus Exp $ -*- c++ -*-
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

#ifndef GNOMOKU_POINT_H
#define GNOMOKU_POINT_H

#include <gtk--/button.h>
#include <gnome--/pixmap.h>

namespace Gnomoku
{
    class Point : public Gtk::Button {
	static SigC::Signal0<void> blink_sig;
	static bool initialized;
	
	Gnome::Pixmap px;
	int y, x, p, vp, blink;
	SigC::Connection conn;
	
    public:
	void init();
	
	Point(int y_, int x_);
	void set(int p_);
	void set_blink(bool b);
	int get() { return p; };
	int gety() { return y; };
	int getx() { return x; };
    private:
	void vset(int vp_);
	void toggle();
	static gint blink_method();
    };
}

#endif
