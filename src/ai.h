// $Id: ai.h,v 1.6 2001/11/23 23:07:29 cactus Exp $ -*- c++ -*-
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

#ifndef GNOMOKU_AI_H
#define GNOMOKU_AI_H

#include "opponent.h"

namespace Gnomoku
{
    class AI : public Opponent {
	int width, height, length;
	int *board, *score;
	bool my_turn, first;
    public:
	AI(int arows, int acols);
	~AI();
	bool ok() { return true; };
	int err() { return 0; };
	bool ready() { return (my_turn||first); };
	void put_msg(msg_t &msg);
	void get_msg(msg_t &msg);
	void won() { my_turn = false; };
    private:
	void reset();
	void play_move(int x, int y, int val);
	int strongest_square();
	int nb_qtuples(int x, int y);
	void update_score(int square, int dval);
	void update_score_dir(int left, int right, int square,
			      int dx, int dy, int dval);
    };
}

#endif
