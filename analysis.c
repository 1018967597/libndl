/* analysis.c -- this file provide analysis function, mainly for DNS 
   inside datagrams.

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

#include "inner"

int
cache_append_quoted1035 (c, buf, len)
      cache *c;
      const byte *buf;
      int len; 
{

}

void
ndl_take_labels (label_sd *lsd,
                 ndl_sd *nsd,
                 query_sd *qsd,
                 int server,
                 const byte *datagram,
                 int dglen,
                 int max,
                 int name_begin,
                 int *name_end)
{
  lsd->nsd = nsd;
  lsd->qsd = qsd;
  lsd->server = server;
  lsd->datagram = datagram;
  lsd->dglen = dglen;
  lsd->max = max;
  lsd->cbyte = name_begin;
  lsd->nl = 0;
  lsd->name_end = name_end;
}

void
ndl_parse_labels (label_sd *lsd,
                  int lablen,
                  int label_begin)
{

}

ndl_status
ndl_take_names (ndl_sd nsd,
                query_sd qsd,
                int server,
                cache *c,
                const byte *dgram,
                int dglen,
                int *cbyte_io,
                int max)
{


}

ndl_status
ndl_parse_names (ndl_sd nsd,
                 query_sd qsd,
                 int server,
                 cache *c,
                 const byte *dgram,
                 int dglen,
                 int *cbyte_io,
                 int max)
{


}

ndl_status
ndl_parse_rrs (query_sd qsd,
               int server,
               const byte *datagram,
               int dglen,
               int *cbyte_io,
               int *type,
               int *class,
               unsigned long *ttl,
               int *rdlength,
               int *rdstart,
               int *name_matched_query)
{


}

ndl_status
ndl_find_rrs (query_sd qsd,
              int server,
              const byte *datagram,
              int dglen,
              int *cbyte_io,
              int *type,
              int *class,
              unsigned long *ttl,
              int *rdlength,
              int *rdstart,
              const byte *eo_dgram,
              int eo_dglen,
              int eo_cbyte,
              int *eo_matched_r)
{


}
