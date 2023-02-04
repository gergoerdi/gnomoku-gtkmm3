//-*-Mode: C++;-*-
// Copyright (C) 2001 Daniel Elstner <daniel.elstner@gmx.net>
// Horribly mutilated by Gergõ Érdi <cactus@cactus.rulez.org>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License VERSION 2 as
// published by the Free Software Foundation.  You are not allowed to
// use any other version of the license; unless you have the explicit
// permission from the author to do so.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef GNOMOKU_fdstream_h
#define GNOMOKU_fdstream_h

#include <cstddef>
#include <iostream>
#include <cstring>
#include <streambuf> /* libstdc++-v2 doesn't have <streambuf> */

namespace Gnomoku
{
    class fdstreambuf : public std::streambuf
    {
	int   buf_size;
	char *buf;
	int   fd;
	
    public:
	fdstreambuf (int fd);
	virtual ~fdstreambuf();	
	
	int  flush ();
	int  sync  ();
	bool close ();

	int  get_fd() const   { return fd; }
	void detach()         { flush(); fd = -1;  }
	void attach (int fd_) { flush(); fd = fd_; }
	
    protected:
	virtual int underflow();
	virtual int overflow (int c);

	virtual std::streamsize xsgetn (char* dest,      std::streamsize count);
	virtual std::streamsize xsputn (const char* src, std::streamsize count);
	
    private:
	char_type* pback_end_;
	
	std::size_t sys_read_ (char* dest,       std::size_t count, std::size_t min_count);
	std::size_t sys_write_ (const char* src, std::size_t count, std::size_t min_count);
    };    

    class fdstream : public std::iostream
    {
    private:
	fdstreambuf buf;
	
	// noncopyable
	fdstream (const fdstream&);
	fdstream& operator= (const fdstream&);
	
    public:
	explicit  fdstream (int fd = -1);
	virtual  ~fdstream();
	
	fdstreambuf* rdbuf() const { // dunno why this has to be const
	    return const_cast<fdstreambuf*>(&buf);
	}
	
	// The fd will be closed by the dtor.
	// If you don't want that, detach it before destruction.
	void close() {
	    if (!buf.close())
		this->setstate (std::ios::failbit);
	}
	
	bool is_open() const {
	    return (buf.get_fd () >= 0);
	}

	int get_fd() const {
	    return buf.get_fd ();
	}
	
	// Note: neither attach nor detach tries to close the fd.
	void attach (int fd) { buf.attach (fd); clear(); }
	void detach ()       { buf.detach (); clear(); }
    };
}

#endif

