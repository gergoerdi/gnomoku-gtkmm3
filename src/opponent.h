// $Id: opponent.h,v 1.6 2001/11/23 23:07:29 cactus Exp $ -*- c++ -*-
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

#ifndef GNOMOKU_OPPONENT_H
#define GNOMOKU_OPPONENT_H

#include <string>

#define GNOMOKU_VERSION 001;
enum op_t { OP_AI, OP_CLIENT, OP_SERVER };
//enum msgtype { MSG_GREET, MSG_START, MSG_PUT, MSG_CLOSE };
#define msgtype int
#define MSG_GREET 1
#define MSG_START 2
#define MSG_PUT 3
#define MSG_CLOSE 4

namespace Gnomoku
{    
    struct msg_t {
	msgtype type;
	int version;
	std::string ident;
	int y, x;
    };
    
    class Opponent {
    public:
	virtual ~Opponent() {};
	virtual bool ok() = 0;
	virtual int err() = 0;
	virtual bool ready() = 0;
	virtual void put_msg(msg_t &msg) = 0;
	virtual void get_msg(msg_t &msg) = 0;
	virtual void won() = 0;
    };
}

#endif
