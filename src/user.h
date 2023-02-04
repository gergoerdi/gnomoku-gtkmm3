// $Id: user.h,v 1.6 2001/11/23 23:07:29 cactus Exp $ -*- c++ -*-
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

#ifndef GNOMOKU_USER_H
#define GNOMOKU_USER_H

#include "opponent.h"
#include <pthread.h>
#include <string>
#include <iostream>

namespace Gnomoku
{
    class User : public Opponent {
	op_t           optype;
	std::string    server;
	int            port;
	
	int            sock;
	std::iostream *str;
	bool           okay;
	int            error;
	pthread_t      thread;
    public:
	User (op_t               aoptype,
	      const std::string &aserver,
	      const std::string &aport);
	~User();
	
	bool ok  () { return okay; };
	int  err () { return error; };
	
	bool ready ();
	void put_msg (msg_t &msg);
	void get_msg (msg_t &msg);
	void won () {};
    private:
	int init ();
	static void* sinit (User *ins);
    };
}

#endif
