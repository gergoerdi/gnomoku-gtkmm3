// $Id: point.cc,v 1.21 2001/11/23 23:07:29 cactus Exp $
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

//#include "config.h"

#include "point.h"
#include "pic_px0.h"
#include "pic_px1.h"
#include "pic_px2.h"

#include <gtkmm/main.h>
#include <iostream>

using namespace Gnomoku;


bool Point::initialized = false;
sigc::signal0<void> Point::blink_sig;

void Point::init()
{
    Glib::signal_timeout().connect(sigc::ptr_fun (&Point::blink_method), 500);
    initialized = true;
}

bool Point::blink_method()
{
    blink_sig();
    return true;
}

Point::Point(int y_, int x_):
    y(y_),
    x(x_)
{
    blink=false;
    set(0);
    //add(px);
	set_image (px);


}

void Point::set(int ap)
{
    p = ap;
    if (!blink)
    vset(p);
}

void Point::set_blink(bool b)
{
    if (blink==b)
    return;
    blink = b;

    if (blink) {
	conn = blink_sig.connect(sigc::mem_fun(*this, &Point::toggle));
    } else {
	conn.disconnect();
	vset(p);
    }
}

void Point::vset(int vp_)
{
  if (!initialized)
  init();
  vp = vp_;

  switch(vp) {
	case 0:
          px.set(Gdk::Pixbuf::create_from_inline (-1, pic_px0, false));
	  break;
	case 1:
          px.set(Gdk::Pixbuf::create_from_inline (-1, pic_px1, false));
	  break;
	default:
          px.set(Gdk::Pixbuf::create_from_inline (-1, pic_px2, false));
  }



}

void Point::toggle()
{
    vset(vp ? 0 : p);
}
