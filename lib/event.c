/* event.c -- this file is event loop core.  
 
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

#include "inside.h"

static void 
inter_maxto (tv **tv_io, 
             tv *tvbuf,
	     tv maxto) 
{
  tv *rbuf;

  if (!tv_io) 
    return;

  rbuf = *tv_io;

  if (!rbuf) 
    {
      *tvbuf = maxto; 
      *tv_io = tvbuf;
    } 
  else 
    {
      if (timercmp (rbuf, &maxto, >)) 
        *rbuf = maxto;
    }
}

static void 
inter_maxtoabs (tv **tv_io, 
                tv *tvbuf,
	        tv stamp_t, 
                tv maxtime) 
{
  ldiv_t dr;

  if (!tv_io) 
    return;

  maxtime.tv_sec -= (stamp_t.tv_sec + 2);
  maxtime.tv_usec -= (stamp_t.tv_usec - 2000000);

  dr = ldiv (maxtime.tv_usec, 1000000);

  maxtime.tv_sec += dr.quot;
  maxtime.tv_usec -= dr.quot * 1000000;

  if (maxtime.tv_sec < 0) 
    timerclear (&maxtime);

  inter_maxto (tv_io, tvbuf, maxtime);
}

static void 
io_fd_set (int *max_fd, 
           fd_set *fds, 
           int fd) 
{
  if (!max_fd || !fds) 
    return;

  if (fd >= *max_fd) 
    *max_fd = fd + 1;

  FD_SET (fd, fds);
}

static void 
timeout_check (ndl_sd nsd, 
               tv stamp_t,
	       tv **tv_io, 
               tv *tvbuf) 
{
  query_sd qsd, nqsd;
  
  for (qsd = nsd->list.head; qsd; qsd = nqsd) 
    {
      nqsd = qsd->next;
      if (timercmp (&stamp_t, &qsd->timeout, >)) 
        {
          LIST_FREE (nsd->list, qsd);
          if (qsd->state != udp) 
            {
	      query_fail (qsd, TIMEOUT);
            } 
          else 
            {
	      udp_messages (qsd, stamp_t);
            }
        } 
      else 
        {
          inter_maxtoabs (tv_io, tvbuf, stamp_t, qsd->timeout);
        }
    }
}  
 
void 
likefd (ndl_sd nsd, 
        int *max_fd,
        fd_set *read_fds, 
        fd_set *write_fds, 
        fd_set *except_fds,
        tv **tv_io, 
        tv *tvbuf) 
{
  tv stamp_t;
  tv tvto_lr;
  int r;

  r = gettimeofday (&stamp_t, 0);
  if (r) 
    {
      printw (nsd, -1, 0, "gettimeofday failed - will sleep for a bit: %s",
	      strerror (errno));
      timerclear (&tvto_lr); 
      set_timeout (&tvto_lr, LOCAL_RESOURCE_MS);
      inter_maxto (tv_io, tvbuf, tvto_lr);
    } 
  else 
    {
      timeout_check (nsd, stamp_t, tv_io, tvbuf);
    }
  
  io_fd_set (max_fd, read_fds, nsd->protoc.udp);
}

static int 
io_fd_check (int max_fd, 
             const fd_set *fds, 
             int fd) 
{
  return max_fd < 0 || !fds 
         ? 1 
         : fd < max_fd && FD_ISSET (fd, fds);
}

static int 
process_response (ndl_sd nsd, 
                  query_sd qsd,
                  int max_fd,
	          const fd_set *read_fds, 
                  const fd_set *write_fds,
		  const fd_set *except_fds,
	          tv stamp_t) 
{
  int count, udp_addrlen, r, server;
  byte udp_buf[NAMES];
  struct sockaddr_in udp_addr;

  count = 0;
  if (io_fd_check (max_fd, read_fds, nsd->protoc.udp)) 
    {
      count++;
      for (;;) 
        {
          udp_addrlen = sizeof (udp_addr);
          r = recvfrom (nsd->protoc.udp, udp_buf, sizeof (udp_buf), 0, &udp_addr, &udp_addrlen);

          if (r < 0) 
            {
	      if (!(errno == EAGAIN || errno == EWOULDBLOCK ||
	            errno == EINTR || errno == ENOMEM || errno == ENOBUFS))
	        printw (nsd, -1, 0, "datagram receive error: %s", strerror(errno));

	      break;
            }
          
          if (udp_addrlen != sizeof(udp_addr)) 
            {
	      printd (nsd, -1, 0, "datagram received with wrong address length %d"
		   " (expected %d)", udp_addrlen,sizeof(udp_addr));
	      continue;
            }

          if (udp_addr.sin_family != AF_INET) 
            {
	      printd (nsd, -1, 0, "datagram received with wrong protocol family"
		   " %u (expected %u)", udp_addr.sin_family,AF_INET);
	      continue;
            }

          if (ntohs (udp_addr.sin_port) != SERVER_PORT) 
            {
	      printd (nsd, -1, 0, "datagram received from wrong port %u (expected %u)",
		      ntohs (udp_addr.sin_port),SERVER_PORT);
	      continue;
            }

          for (server = 0;
	       server < nsd->nservers 
               && nsd->servers[server].addr.s_addr != udp_addr.sin_addr.s_addr;
	       server++);

          if (server >= nsd->nservers) 
            {
	      printw (nsd, -1, 0, "datagram received from unknown nameserver %s",
		      inet_ntoa (udp_addr.sin_addr));
	      continue;
            }
          
          procmsg (nsd, qsd, udp_buf, r , server, stamp_t);
        }
     
    }
  return count;
}

int 
dnsmsg (ndl_sd nsd, 
        query_sd qsd,
        int max_fd,
        const fd_set *read_fds, 
        const fd_set *write_fds,
        const fd_set *except_fds) 
{
  tv stamp_t;
  int r;

  r = gettimeofday (&stamp_t, 0); 
  if (r) 
    return -1;

  timeout_check (nsd, stamp_t, 0, 0);

  return process_response (nsd, qsd, max_fd, read_fds, write_fds, except_fds, stamp_t);
}

static int 
io_check (ndl_sd nsd,
	  query_sd *query_io,
	  answer_sd **answer) 
{
  query_sd qsd;

  qsd = *query_io;
  if (!qsd) 
    {
      if (!nsd->output_list.head) 
        return EWOULDBLOCK;

      qsd = nsd->output_list.head;
    } 
  else 
    {
      if (qsd->id >= 0) 
        return EWOULDBLOCK;
    }

  LIST_FREE (nsd->output_list, qsd);
  *answer = qsd->asd;

  free (qsd);
  return 0;
}

int 
ioevent (ndl_sd nsd,
         query_sd *query_io,
	 answer_sd **answer_r) 
{
  int r, max_fd, rsel, rmsg;
  fd_set read_fds, write_fds, except_fds;
  tv tvbuf, *tvp;
  query_sd qsd = *query_io; 

  qsd->asd->q_id = qsd->nsd->next_id;
  qsd->asd->q_opcode = qsd->nsd->q_opcode;
  qsd->asd->q_rd = qsd->nsd->q_rd;

  for (;;) 
    {
      r = io_check (nsd, query_io, answer_r);
      if (r != EWOULDBLOCK) 
        return r;

      max_fd = 0; 
      tvp = 0;
      FD_ZERO (&read_fds); 
      FD_ZERO (&write_fds); 
      FD_ZERO (&except_fds);

      likefd (nsd, &max_fd, &read_fds, &write_fds, &except_fds, &tvp, &tvbuf);

      rsel = select (max_fd, &read_fds, &write_fds, &except_fds, tvp);

      if (rsel == -1) 
        {
          if (errno == EINTR) 
            continue;

          return errno;
        }

      rmsg = dnsmsg (nsd, qsd, max_fd, &read_fds, &write_fds, &except_fds);
      assert(rmsg == rsel);
    }
}

