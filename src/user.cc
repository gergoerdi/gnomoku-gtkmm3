// $Id: user.cc,v 1.8.2.2 2001/12/06 17:30:17 cactus Exp $
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

#include "user.h"
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>

#include "fdstream.h"

using namespace Gnomoku;
using std::string;

User::User(op_t aoptype, const string &aserver, const string &aport) :
    optype(aoptype),
    server(aserver),
    port(atoi(aport.c_str())),
    sock(0),
    str(0),
    okay(false),
    error(0)
{
    pthread_create(&thread, NULL, (void*(*)(void*))&sinit, this);
}

User::~User()
{
    if (okay) {
	msg_t msg;
	msg.type = MSG_CLOSE;
	put_msg(msg);
    }
    pthread_cancel(thread);
    pthread_join(thread, NULL);
    if (str) delete str;
    if (sock) close(sock);
}

void* User::sinit(User *ins)
{
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    ins->error = ins->init();
    return NULL;
}

int User::init()
{
    struct sockaddr_in  addr;
    //size_t              size;
	unsigned int		size;
    struct hostent     *host;
    int                 sockn;

    okay = false;

    if ((sock = socket (PF_INET, SOCK_STREAM, 0)) < 0) return errno;
    addr.sin_family = AF_INET;
    addr.sin_port = htons (port);
    
    if (optype == OP_CLIENT) {
	if (!(host = gethostbyname (server.c_str ()))) return errno;
	addr.sin_addr = *(struct in_addr *) host->h_addr;
	if (connect (sock, (struct sockaddr *)&addr, sizeof(addr)))
	    return errno;
    } else {
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind (sock, (struct sockaddr *)&addr, sizeof(addr))) return errno;
	if (listen (sock, 1)) return errno;
	if ((sockn = accept (sock, (struct sockaddr *)&addr, &size)) < 0)
	    return errno;
	close (sock);
	sock = sockn;
    }

    str = new fdstream (sock);
    
    msg_t msg;
    struct passwd *id;
    
    id = getpwuid (getuid());
    string gecos (id->pw_gecos);
    string name (id->pw_name);
    msg.type = MSG_GREET;
    msg.version = GNOMOKU_VERSION;
    msg.ident = gecos.substr (0,gecos.find_first_of(',')) + " (" + name + ")";
    put_msg (msg);

    okay = true;
    return 0;
}

bool User::ready ()
{
    fd_set fds;
    struct timeval tv;
    int retval;

    FD_ZERO(&fds);
    FD_SET(sock, &fds);
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    retval = select(sock + 1, &fds, NULL, NULL, &tv);

    return retval;
}

void User::put_msg (msg_t &msg)
{
    (*str) << msg.type << ' ';

    switch (msg.type)
    {
    case MSG_GREET:
	(*str) << msg.version << ' ' << msg.ident << std::ends;
	break;
    case MSG_START:
    case MSG_CLOSE:
	break;
    case MSG_PUT:
	(*str) << msg.x << ' ' << msg.y;
	break;
    }

    (*str) << std::endl;
}

void User::get_msg (msg_t &msg)
{
    (*str) >> msg.type;
    
    switch (msg.type)
    {
    case MSG_GREET:
	(*str) >> msg.version;
	str->get();
	getline(*str, msg.ident, '\0');
	break;
    case MSG_START:
	break;
    case MSG_CLOSE:
	okay = false;
	break;
    case MSG_PUT:
	(*str) >> msg.x >> msg.y;
	break;
    }

    str->clear();
    while (str->get () != '\n' && !str->eof ());
}
