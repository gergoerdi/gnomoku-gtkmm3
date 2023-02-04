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

#include "fdstream.h"

#include <sys/types.h>
#include <unistd.h>

namespace {

enum {
    BUFFER_SIZE     = 1024,
    PUTBACK_SIZE    = 8,
};

inline
int char_to_int (const char &c)
{
    // Avoid sign bit expansion in case char is signed.
    return static_cast<int>(static_cast<unsigned char>(c));
}

inline
char int_to_char (const int &i)
{
    return static_cast<char>(i);
}

} // anonymous namespace


namespace Gnomoku {

fdstreambuf::fdstreambuf (int fd_):
    buf_size (PUTBACK_SIZE + BUFFER_SIZE),
    buf      (new char[buf_size]),
    fd       (fd_)
{
    char *const pbuf = buf + PUTBACK_SIZE;
    pback_end_ = pbuf;
    
    setg (pbuf, pbuf, pbuf);
    setp (buf, buf + buf_size - 1);
}

fdstreambuf::~fdstreambuf()
{
    close ();
    delete buf;
}

bool fdstreambuf::close()
{
    flush();

    if (fd >= 0)
    {
        int rc;
        while ((rc = ::close (fd)) < 0 && errno == EINTR);
        if (rc < 0)
	    return false;
	
        fd = -1;
    }

    return true;
}


int fdstreambuf::flush()
{
    char *const pbuf = buf + PUTBACK_SIZE;
    setg (pbuf, pbuf, pbuf);

    const std::size_t count       = pptr() - pbase();
    const std::size_t num_written = sys_write_ (pbase(), count, count);

    // If the write didn't succeed, you're in bad luck ;)
    pbump (-count);

    return (num_written == count) ? 0 : -1;
}

fdstreambuf::int_type fdstreambuf::underflow()
{
    // Is the buffer already empty yet?
    char *tmp_gptr = gptr();
    if (tmp_gptr >= egptr()) {

        char *tmp_eback = eback();
        const std::size_t num_putback =
            std::min <std::size_t> (tmp_gptr - tmp_eback, pback_end_ - buf);

        // Fill putback area.
        tmp_eback = pback_end_ - num_putback;
        std::memcpy (tmp_eback, tmp_gptr - num_putback, num_putback);

        // Restore standard buffer position (directly after putback area).
        tmp_gptr = pback_end_;

        // Try to read as much data as available right now.
        const std::size_t num_read = sys_read_ (tmp_gptr, (buf + buf_size) - pback_end_, 1);

        // FIXME: The error reporting could be more accurate.
        if (num_read == 0) return EOF;

        setg (tmp_eback, tmp_gptr, pback_end_ + num_read);
    }
    return char_to_int (*tmp_gptr);
}

std::streamsize fdstreambuf::xsgetn (char *dest, std::streamsize count)
{
    char *tmp_eback = eback();
    char *tmp_gptr  = gptr();
    char *tmp_egptr = egptr();

    // First, empty the current buffer.
    const std::size_t buf_avail = tmp_egptr - tmp_gptr;
    std::streamsize   idx       = std::min <std::size_t> (buf_avail, count);

    std::memcpy (dest, tmp_gptr, idx);

    // Reset buffer positions if the end was reached.
    if ((tmp_gptr += idx) == tmp_egptr) {
        tmp_gptr  = pback_end_;
        tmp_egptr = tmp_gptr;
    }

    // Get remaining data directly from fd.
    idx += sys_read_ (dest+idx, count-idx, count-idx);

    // Refill putback area if we're not currently inside it.
    if (tmp_gptr >= pback_end_) {

        const std::size_t num_putback =
	    std::min <std::size_t> (idx, pback_end_ - buf);

        tmp_eback = pback_end_ - num_putback;
        std::memcpy (tmp_eback, dest + idx - num_putback, num_putback);
    }

    // Finally, update buffer positions.
    setg (tmp_eback, tmp_gptr, tmp_egptr);
    return idx;
}

std::size_t fdstreambuf::sys_read_ (char        *dest,
				    std::size_t  count,
				    std::size_t  min_count)
{
    std::size_t idx = 0;

    while (idx < min_count)
    {
        const long num_read = read (fd, dest + idx, count - idx);
        if (num_read <= 0) {
            if (num_read == 0) // EOF
		break;
        }
        idx += num_read;
    }
    return idx;
}



int fdstreambuf::sync()
{
    return (fd >= 0) ? flush() : -1;
}

fdstreambuf::int_type fdstreambuf::overflow (fdstreambuf::int_type c)
{
    if (c != EOF) {
        *pptr() = int_to_char (c);
        pbump (1);
    }
    return (flush() >= 0) ? c : EOF;
}

std::streamsize fdstreambuf::xsputn (const char *src, std::streamsize count)
{
    // If it fits into the buffer, everything is just fine.
    if (count <= epptr()-pptr()) {
        std::memcpy (pptr(), src, count);
        pbump (count);
    }
    else {
        // Otherwise, flush buffer contents.
        if (flush() < 0) return 0;

        const std::streamsize buf_space = epptr() - pptr();

        // Here we use some guesswork. If it fits
        // into a quarter of the buffer, put it there.
        if (count < buf_space/4) {
            std::memcpy (pptr(), src, count);
            pbump (count);
        }
        else {
            // Let's go on to the real deal, then.
            const std::size_t
                num_written = sys_write_ (src, count, count - buf_space);

            // Does the rest fit into the buffer?
            const std::streamsize num_left = count - num_written;
            if (num_left > buf_space) return num_written;

            // Ok, fill the buffer with the remaining stuff.
            if (num_left > 0) {
                std::memcpy (pptr(), src+num_written, num_left);
                pbump (num_left);
            }
        }
    }
    return count;
}

std::size_t fdstreambuf::sys_write_ (const char* src, std::size_t count,
				     std::size_t min_count)
{
    std::size_t idx = 0;
    while (idx < min_count) {

        const long num_written = write (fd, src + idx, count - idx);
        if (num_written <= 0) {
            if (num_written == 0) break; // EOF
        }
        idx += num_written;
    }
    return idx;
}


// **** template class Util::generic_fdstream:

fdstream::fdstream (int fd):
    std::iostream (0),
    buf           (fd)
{
    std::iostream::init (&buf);
}

fdstream::~fdstream()
{
}

} // namespace Gnomoku


