// $Id: ai.cc,v 1.13 2001/11/23 23:07:29 cactus Exp $
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

#include "ai.h"
#include <cstdlib>
//#include <libgnome/libgnome.h>
/*
#define NIL_SCORE	7
#define X_SCORE		15
#define XX_SCORE	400
#define XXX_SCORE	1800
#define XXXX_SCORE	100000
#define O_SCORE		35
#define OO_SCORE	800
#define OOO_SCORE	15000
#define OOOO_SCORE	800000
*/

const int NIL_SCORE = 7;
const int X_SCORE = 15;
const int XX_SCORE = 400;
const int XXX_SCORE = 1800;
const int XXXX_SCORE = 100000;
const int O_SCORE = 35;
const int OO_SCORE = 800;
const int OOO_SCORE = 15000;
const int OOOO_SCORE = 800000;

const int score_trans_table[] = {
    NIL_SCORE,  X_SCORE,    XX_SCORE,   XXX_SCORE,  XXXX_SCORE, 0,
    O_SCORE,    0,          0,          0,          0,          0,
    OO_SCORE,   0,          0,          0,          0,          0,
    OOO_SCORE,  0,          0,          0,          0,          0,
    OOOO_SCORE, 0,          0,          0,          0,          0,
    0
};

//#define MIN(a,b) ((a) < (b) ? (a) : (b))
//#define MAX(a,b) ((a) > (b) ? (a) : (b))
/*
#define xy_index(x, y) ((y * width) + x + y)
#define index_x(index) (index % (width + 1))
#define index_y(index) (index / (width + 1))
*/
using namespace Gnomoku;

AI::AI(int arows, int acols)
{
    width = acols;
    height = arows;
    length = ((height+2) * (width+1) + 1);
	
    board = new int[length];
    score = new int[length];
    for (int i=0; i<length; i++) {
	board[i] = -1;
	score[i] = -1;
    }

    first = true;    
    reset();
}

AI::~AI()
{
    delete [] board;
    delete [] score;
}

void AI::put_msg(msg_t &msg)
{
   switch (msg.type) 
   {
   case MSG_START:
	  reset();
	  break;
   case MSG_PUT:
	  play_move(msg.x+1, msg.y+1, 1);
	  my_turn = true;
	  break;
   case MSG_GREET:
   case MSG_CLOSE:
	  break;
   }
}

void AI::get_msg(msg_t &msg)
{
   if(first)
   {
		msg.type = MSG_GREET;
		msg.ident = ("the Gnomoku Game Engine");
		first = false;
   } else if (my_turn) {
		int square = strongest_square();
		int x = index_x(square);
		int y = index_y(square);
		play_move(x, y, 6);
		my_turn = false;
	
		msg.type = MSG_PUT;
		msg.x = x-1;
		msg.y = y-1;
   }
}	

void AI::reset()
{
    for (int x=1; x<=width; x++)
   {
		 for (int y=1; y<=height; y++)
		{
			board[xy_index(x,y)] = 0;
			score[xy_index(x,y)] = nb_qtuples(x,y) * NIL_SCORE;
		}
	}
    my_turn = false;
}

void AI::play_move(int x, int y, int val)
{
    int square = xy_index(x,y);
    board[square] = val;
    update_score(square, val);
    score[square] = -1;
}

int AI::strongest_square()
{
    int max=0;
    int square=0;
    int count=0;

//     printf("\n\n\n");
//     for (int y=1; y<=height; y++) {
// 	for (int x=1; x<=width; x++) {
// 	    printf("%6d ", score[xy_index(x,y)]);
// 	}
// 	printf("\n\n");
//     }

    for (int i=0; i<length; i++) 
    {
		if ((board[i]) || (score[i] < max)) continue;

		if (score[i] > max) 
		{
			count = 1;
			max = score[i];
			square = i;
		} else if ((score[i] == max) && ((rand() % (++count))==0)) {
			max = score[i];
			square = i;
		}
    }

    return square;
}


int AI::nb_qtuples(int x, int y)
{
    int left  = min(4, x-1);
    int right = min(4, width-x);
    int up    = min(4, y-1);
    int down  = min(4, height-y);
    return ((-12) +
	min(max(left+right, 3), 8) +
	min(max(up+down, 3), 8) +
	min(max(min(left, up) + min(right, down), 3), 8) +
	min(max(min(right, up) + min(left, down), 3), 8));
}

void AI::update_score(int square, int dval)
{
    int x = index_x(square);
    int y = index_y(square);
    int imin = max(-4, 1-x);
    int jmin = max(-4, 1-y);
    int imax = min(0, width-x-4);
    int jmax = min(0, height-y-4);
    update_score_dir(imin, imax, square, 1, 0, dval);
    update_score_dir(jmin, jmax, square, 0, 1, dval);
    update_score_dir(max(imin, jmin), min(imax, jmax), square, 1, 1, dval);
    update_score_dir(max(1-y, max(-4, x-width)),
		     min(0, min(x-5, height-y-4)),
		     square, -1, 1, dval);
}

void AI::update_score_dir(int left, int right, int square,
			  int dx, int dy, int dval)
{
    if (left > right) return;

    int depl = xy_index(dx, dy);
    int square0 = square + (left * depl);
    int square1 = square + (right * depl);
    int square2 = square0 + (4 * depl);
    int count, delta;

    square = square0;
    count = 0;

   while (square <= square2) 
   {
		count += board[square];
		square += depl;
   }

   while (square0 <= square1) 
   {
		delta = score_trans_table[count] - score_trans_table[count-dval];
		if (delta) 
		{
			square = square0;
			while (square <= square2) 
			{
				if (board[square]==0)
					score[square] += delta;
				square += depl;
			}
		}
	square2 += depl;
	count += -board[square0] + board[square2];
	square0 += depl;
   }
}


int AI::index_x(int index)
{
	return (index % (width + 1));
}

int AI::index_y(int index)
{
	return (index / (width + 1));
}

int AI::xy_index(int x, int y)
{
	return ((y * width) + x + y);
}

int AI::min(int a, int b)
{
  if(a < b)
  {
	return a;
  }else{
	return b;
  }
}


int AI::max(int a, int b)
{
  if(a > b)
  {
	return a;
  }else{
	return b;
  }
}






