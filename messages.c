/* messages.c -- this file is core.
   
   Copyright (C) 2012 2013 meng shi.  

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

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <sys/time.h>

#include "inner.h"


static query_sd
ndl_query_load (ndl_sd nsd,
                const type_i *ti,
                tv stamp_t)
{
  query_sd qsd;
  
  qsd = malloc (sizeof (*qsd));
  if (!qsd)
    return 0; 
 
  qsd->asd = malloc (sizeof (*qsd->asd));
  if (!qsd->asd)
    {
      free (qsd);
      return 0;
    } 
  
  qsd->nsd = nsd;
  qsd->state = udp;
  
  INIT_TREE (qsd->tree);
  INIT_QUEUE (qsd->children);
  INIT_LINK (qsd->siblings);
  INIT_QUEUE (qsd->inode); 
  INIT_CACHE (&qsd->vb);
 // nqs->interim_allocd = 0;
  //nqs->final_allocspace = 0;
  
  qsd->ti = ti;
  qsd->query_datagram = 0;
  qsd->query_dglen = 0;
  
  qsd->cname_datagram = 0;
  qsd->cname_dglen = qsd->cname_begin = 0;
  
 // ndl_init_buf (&nqs->search_vb);
 // nqs->search_orglen = nqs->search_pos = nqs->search_done_abs = 0;
  
  qsd->id = 0;
  qsd->udp_server.retries = 0;
  qsd->udp_server.next = 0;
  qsd->udp_sent = qsd->tcp_failed = 0;
  timerclear (&qsd->timeout);
  qsd->limits = stamp_t.tv_sec + TTL; 
  
  memset (&qsd->qc, 0, sizeof (qsd->qc));
  
  
  /* answer load  */ 
  qsd->asd->nsi = dns_ok;
  qsd->asd->name = qsd->asd->name = 0;
  qsd->asd->type = ti->type;
  qsd->asd->limits = -1;
  qsd->asd->crrs = 0;
  qsd->asd->rrs = 0;
  qsd->asd->rrsz = ti->rrsz;

  return qsd;
}

static void
ndl_send_query (ndl_sd nsd,
                query_sd qsd,
                const type_i *ti,
                cache *c,
                int id,
                tv stamp_t)
{
  qsd->c = *c;
  INIT_CACHE (c);
  qsd->query_datagram = malloc (qsd->c.used);
  
  if (!qsd->query_datagram)
    {
      ndl_query_fail (qsd, no_memory);
      return;
    } 
   
  qsd->id = id;
  qsd->query_dglen = qsd->c.used;
  
  memcpy (qsd->query_datagram, qsd->c.buf, qsd->c.used);

  ndl_udp_messages (qsd, stamp_t);
  
  //ndl_autosys (nsd, now);
}
/*
static void *
lalloc (nqs, l)
     ndl_query nqs; 
     size_t l;
{
  node *n;
  
  if (!l)
    return nqs;
  assert (!nqs->final_allocspace);
  
  n = malloc (PROC_MEM (PROC_MEM (sizeof (*n)) + l));
  if (!n)
    return 0;
  
  LIST_LINK_TAIL (nqs->allocations, n);
  
  return (byte*)n + PROC_MEM (sizeof (*n));
}

void *
ndl_i_alloc (nqs, l)
         ndl_query nqs;
         size_t l;
{
  l = PROC_MEM (l);
  nqs->interim_allocd += l;
  return lalloc (nqs, l);
}

static int
ndl_get_name (nqs, name, l)
     ndl_query nqs;
     const char *name;
     int l;
{
  ndl_answer *nas;
  
  nas = nqs->answer;
  assert (!nas->name);
  
  nas->name = ndl_i_alloc (nqs, l + 1);
  
  memcpy (nas->name, name, l);
  nas->name[l] = 0;
  
  return 1; 
}
*/
int 
dnstransf (ndl_sd nsd,
           query_sd *qsd,
           const char *name,
           type_values type,
           class_values class,
           void *context)
{
  int i, nl;
  ndl_si nsi;
  const type_i *ti;
  tv stamp_t;
  query_sd in;
  //const char *p;
  
  //printf ("type is %s\n", ti);
  //printf ("name is %s\n", name); 
  ti = ndl_get_type (type);
  if (!ti)
    return unknown_rr_type; 
  //else
   // printf ("sucessfull!");
  i = gettimeofday (&now, 0);
  if (i)
    goto j_errno;
  
  in = ndl_query_load (nsd, ti, stamp_t);
  if (!in)
    goto j_errno;
  
  in->qc.ctx = context;
  in->qc.callback = 0;
 
  memset (&in->qc.info, 0 , sizeof (in->qc.info));
  
  *qsd = in;
  
  nl = strlen (name);
  if (!nl)
    {
      nsi = query_domain_invalid;
      goto j_ndl_fail; 
    }
  
  if (nl > NAMES + 1)
    {
      nsi = query_domain_too_long;
      goto j_ndl_fail; 
    } 
  
  if (nl >= 1 && name[nl-1] == '.' 
      && (nl < 2 || name[nl-2] != '\\'))
    {
      nl--;
    }  
  
  if (!ndl_get_name (qsd, name, nl))
    {
      s = no_memory;
      goto j_ndl_fail;
    } 
  
  ndl_make_query (nds, qsd, name, nl, ti, stamp_t);

  return 0;
  
  j_ndl_fail:
    return 0;

  j_errno:
    i = errno;
    assert (i);
    return i;
}

static void
ndl_make_query (ndl_sd nsd,
                query_sd, qsd,
                const char *name,
                int nl,
                const type_i *ti,
                tv stamp_t)
{
  cache c;
  int id;
  ndl_si nsi;
  
  INIT_CACHE (&c);
  
  nsi = ndl_create_query (nsd, &c, &id, name, nl, ti);
  
  if (nsi)
    {
      ndl_query_fail (qsd, nsi);
      return;
    }
  
  ndl_send_query (nsd, qsd, ti, &c, id, stamp_t);
}

ndl_si
ndl_create_query (ndl_sd nsd,
                  cache *c,
                  int *id,
                  const char *name,
                  int nl,
                  const type_i *ti)
{
  byte name[NAMES];
  ndl_si nsi;
  const char *p, *q;
  /* l is inner name length.
     ex:www.facebook.com, www/facebook/com.  */
  int l, c, clabels; 
  
  nsi = ndl_make_header ();
  if (nsi)
    return nsi;
  
  INIT_QUERY ();
  
  p = name;
  q = name + nl;
  clabel = 0;
  while (p != q)
    {
      l = 0;
      while (p != q && (c = *p++) != '.')
        {
          if (c == '\\')
            {
             // if ()
             //   return;
              if (dedigital (p[0]))
                {
                  if (dedigital (p[1]) && dedigital (p[2]))
                    {
                      c = (*p++ - '0') * 100 + (*p++ - '0') * 10 + (*p++ -'0');
                      if (c > NAMES)
                        return query_name_too_long; 
                    }
                  else
                    {
                      return query_name_invaild;
                    }
                } 
              else if (! (c = *p++))
                {
                  return query_name_invaild;
                }
            }
          /*
          if ()
            {
              if ()
                {
                  if ()
                    return; 
                }
              else if ()
                {
          
                }
            }
          */
          if (nl == sizeof (name))
            return query_name_invalid;
          name[nl++] = c;
        } 
      if (!nl)
        return query_name_invalid;
      if (clabel++ > LABELS)
        return query_label_too_long;
    } 
  
  nsi = ndl_make_footer ();
  return dns_ok; // '0'
}

void 
ndl_tcp_messages (query_sd qsd,
                  tv stamp_t)
{
  struct iovec k[2];
  byte l;
  int s, i;
  ndl_sd nsd;
  
  if (qsd->nsd->tcp_status != ok)
    {
      return;
    }
  
  l[0] = (qsd->query_dglen & 0x0ff00U) >> 8;
  l[1] = (qsd->query_dglen & 0x0ff);
  
  nsd = qsd->nsd;
  
  if (!ndl_cache_intend (&ndl->tcp_write, ndl->tcp_write.used + qsd->query_dglen + 2))
    return;
  
  addtimeval (&now, TCPMS);
  qsd->timeout = stamp_t;
  qsd->state = tcp_sent;
  
  if (ndl->tcp_write.used)
    {
      s = 0;
    }
  else
    {
      k[0].iov_base = l;
      k[0].iov_len = 2;
      k[1].iov_base = qsd->query_dgram;
      k[1].iov_len = qsd->query_dglen;
      
      ndl_tcp_protect (qsd->nsd);
      s = writev (qsd->nsd->protoc.tcp, k, 2);
      ndl_tcp_unprotect (qsd->nsd);
      
      if (s < 0)
        {
          if (!(errno == EAGAIN || errno == EINTR || errno == ENOSPC
              || errno == ENOBUFS || errno == ENOMEM))
            {
              //ndl_tcp_broken();
              return;
            }
          
           s = 0;
        }
    }
   
    if (s < 2)
      {
        i = ndl_recache (&nsd->tcp_write, l, 2 - s);
        assert (i);
        s = 0;
      }
    else
      {
        s -= 2;
      }
  
    if (s < qsd->query_dglen)
      {
        i = ndl_cache_append (&ndl->tcp_send, qsd->query_datagram + s, qsd->query_dglen - s);
        assert (i);
      }
}

static void
ndl_tcp_usage (query_sd qsd,
               tv stamp_t)
{
  add_timeval (&stamp_t, TCPMS);
  qsd->timeout = stamp_t;
  qsd->state = query_tcp_wait;
  LIST_LINK_TAIL (qsd->nsd->time_wait, qsd);
  ndl_tcp_messages (qsd, stamp_t);
  ndl_tcp_tryconnect (qsd->nsd, stamp_t);
}

void
ndl_udp_messages (query_sd qsd,
                  tv stamp_t)
{
  struct sockaddr_in sai;
  int server, i;
  ndl_sd nsd;
  
  assert (qsd->state == udp)
  
  if (qsd->query_dglen > UDP_MESSAGES)
    {
      ndl_tcp_usage (qsd, stamp_t);
      return;  
    } 

  if (qsd->udp_server.retries >= MAXUDPRETRIES)
    {
      ndl_query_fail (qsd, timeout);
      return;  
    } 
  
  server = qsd->udp_server.next;
  memset (&sai, 0 , sizeof (sai));
 
  nsd = qsd->nsd;
  
  sai.sin_family = AF_INET;
  sai.sin_addr = nsd->servers[server].addr;
  sai.sin_port = htons (SERVER_PORT);
  
  i = sendto (nsd->protoc.udp, qsd->query_datagram, qsd->query_dglen, 0, &sai, sizeof (sai));
  
  if (i < 0 && errno == EMSGSIZE)
    {
      ndl_tcp_usage (qsd, stamp_t);
      return;
    } 
  
  if (i < 0)
    ndl_printw (nsd, server, 0);
}

void
ndl_inner_submit (nsd, qsd, ti, c, id, now, ctx)
      ndl_sd nsd;
      query_sd *qsd;
      const typeinfo *ti;
      cache *c;
      int id;
      struct timeval now;
      const query_context *ctx;
{



}

static void
ndl_process_responses (perm1, perm2)
      void perm1;
      void perm2;
{



}
