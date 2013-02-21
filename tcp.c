/* tcp.c -- this file is event loop core and TCP connection management
   and you can use user-visible check/wait and event loop related
   functions.
 
   Copyright (C) 2012 2013 S.meng.
 
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <netdb.h>
#include <arpa/inet.h>

#include "inner"

void
tcp_fail (ndl_sd ndl,
          const char *what,
          const char *why)
{


}

static void
tcp_connected (ndl_sd ndl,
               tv timestamp)
{

}

void
tcp_tryconnect (ndl_sd ndl,
                tv timestamp)
{


}

static void
inter_maxto (tv_io, tv_buf, maxto)
      tv **tv_io;
      tv *tv_buf;
      tv maxto;
{

}

static void
inter_maxtoabs (tv_io, tv_buf, timestamp, maxtime)
      tv **ti_io;
      tv *tv_buf;
      tv timestamp;
      tv maxtime;
{

}

static void
inter_addfd (maxfd, fds, fd)
      int *maxfd;
      fd_set *fds;
      int fd;
{


}

static void
check_timeout (nsd, current_t, tv_io, tv_buf)
      ndl_sd ndl;
      tv current_t;
      tv **tv_io;
      tv *tv_buf;
{

}

void
interest (nsd, max_fd, read_fds, write_fds, 
          except_fds, tv_io, tv_buf)
      ndl_sd ndl;
      int *max_fd;
      fd_set *read_fds;
      fd_set *write_fds;
      fd_set *except_fds;
      tv **tv_io;
      tv *tv_buf;
{

} 

static int
check_fd (max_fd, fds, fd)
      int max_fd;
      const fd_set;
      int fd;
{


}

static int
inner_callback (nsd, max_fd, read_fds, write_fds, 
                except_fds, current_t)
      ndl_sd nsd;
      fd_set *read_fds;
      fd_set *write_fds;
      fd_set *except_fds;
      tv current_t;
{


}

int
callback (nsd, max_fd, read_fds, write_fds, execep_fds)
      ndl_sd nsd;
      int max_fd;
      fd_set *read_fds;
      fd_set *write_fds;
      fd_set *except_fds;
{

}

void
autosys (nsd, current_t)
      ndl_sd nsd;
      tv current_t;
{

}

static int
inner_check (nsd, qsd, asd, ctx)
      ndl_sd nsd;
      query_sd *qsd;
      answer_sd **asd;
      void **ctx;
{

}

int
dnswait (nsd, qsd, asd ctx)
      ndl_sd nsd;
      query_sd *qsd;
      answer_sd **asd;
      void **ctx;
{

}

int
dnscheck (nsd, qsd, asd ctx)
      ndl_sd nsd;
      query_sd *qsd;
      answer_sd **asd;
      void **ctx;
{

}

/* SIGPIPE signal protection.  */
void
tcp_protect (ndl_sd nsd)
{
  sigset_t toblock;
  struct sigaction sa;
  int i;
  
  sigfillset (&toblock);
  sigdelset (&toblock, SIGPIPE);
  
  sa.sa_handler = SIG_IGN;
  sigfillset (&sa.sa_mask);
  sa.sa_flags = 0;
  
  i = sigprocmask (SIG_SETMASK, &toblock, &nsd->sigmask);
  assert (!i);
  
  i = sigaction (SIGPIPE, &sa, &nsd->sigpipe);
  assert (!i);
}

void
tcp_unprotect (ndl_sd nsd)
{
  int i;
  
  i = sigaction (SIGPIPE, &nsd->sigpipe, 0);
  assert (!i);

  r = sigprocmask (SIG_SETMASK, &nsd->sigmask, 0);
  assert (!i);
}
