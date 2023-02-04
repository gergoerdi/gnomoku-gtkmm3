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

#include "config.h"

#include "point.h"

#include "px0.xpm"
#include "px1.xpm"
#include "px2.xpm"

#include <gtk--/main.h>

char **pixdata[3] = { px0_xpm, px1_xpm, px2_xpm };

using namespace Gnomoku;

bool Point::initialized = false;
SigC::Signal0<void> Point::blink_sig;

void Point::init()
{
    Gtk::Main::timeout.connect (SigC::slot (&blink_method), 500);
    initialized = true;
}

gint Point::blink_method()
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
    add(px);
}

void Point::set(int ap)
{
    p = ap;
    if (!blink) vset(p);
}

void Point::set_blink(bool b)
{
    if (blink==b) return;
    blink = b;
    
    if (blink) {
	conn = blink_sig.connect(slot(this, &Point::toggle));
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
    px.load(pixdata[vp]);
}
    
void Point::toggle()
{
    vset(vp ? 0 : p);
}
