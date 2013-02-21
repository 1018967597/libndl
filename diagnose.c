/* diagnose.c -- this file is a some diagnosis functions 
   and cache handling.

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

#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>

#include "inner.h"

/* diagnosis functions.  */
void
ndl_diag (ndl_sd nsd,
          query_sd qsd,
          const char *tell,
          int server,
          const char *format,
          va_list vl)
{
  cache c;
  const char *pi, *pj;
  
  if (!nsd->file)
    return;
  
  fprintf (nsd->file, "ndl %s", tell);
  
  vfprintf (nsd->file, format, vl);
  
  pi = "(";
  pj = "\n";
  
  if (qsd && qsd->query_datagram)
    {
      INIT_CACHE(&c);
      /*
      fprintf (nsd->file, "%sQNAME = %s, QTYPE = %s", pi, 
               diagname (qsd->nsd, -1, 0, &c, qsd->query_datagram,
                         qsd->query_dglen, DNS_HEADER_SIZE),
               qsd->ti ? qsd->ti->format_name : "<unknown>");       
      */
      if (qsd->ti && qsd->ti->format_name)
        fprintf (nsd->file, "(%s)", qsd->ti->format_name);

      pi = ", ";
      pj = ")\n"; 
    }
  
  if (server > 0)
    {
      fprintf (nsd->file, "%sNS = %s", pi, 
               inet_ntoa (nsd->server[server].addr));
      pi = ", ";
      pj = ")\n";
    }
 
  fputs (pj, nsd->file);
}
//ndl_debug
void
ndl_printb (ndl_sd nsd,
            query_sd qsd,
            int server,
            const char *format,
            ...)
{
  va_list vl;
  
  va_start (vl, format);
  ndl_diag (nsd, qsd, "bug: ", server, format, vl);
  va_end (vl);
}
//ndl_warn
void
ndl_printw (ndl_sd nsd,
            query_sd qsd,
            int server,
            const char *format,
            ...)
{
  va_list vl;
  
  va_start (vl, format);
  ndl_diag (nsd, qsd, "warning: ", server, format, vl);
  va_end (vl);
}
//ndl_diag
void
ndl_printd (ndl_sd nsd,
        query_sd qsd,
        int server,
        const char *format,
        ...)
{
  va_list vl;
  
  va_start (vl, format);
  ndl_diag (nsd, qsd, "", server, format, vl);
  va_end (vl);
}

/* cache functions  */
int
ndl_cache_intend (cache *c, 
                  int intend)
{
  void *i;
  
  if (c->avail > intend)
    return 1;
  
  i = realloc (c->buf, intend);
  
  if (!i)
    return 0;
  
  c->buf = i;
  c->avail = intend;
  
  return 1;
}

int
ndl_recache (cache *c,
             const byte *data,
             int len)
{
  int l;
  void *i;
  
  l = c->used + len;
  
  if (c->avail < l)
    {
      if (l < 32)
        l = 32;
      
      l <<= 1;
      i = realloc (c->buf, l);
      
      if (!i)
        {
          l = c->used + len;
          i = realloc (c->buf, l);  
        } 
       
      if (!i)
        return 0;
      
      c->buf = i;
      c->avail = l;
    }
    
    ndl_cache_load (c, data, len);
  
    return 1; 
}

void
ndl_cache_load (cache *c,
                const byte *data,
                int len)
{
  memcpy (c->buf + c->used, data, len);
  c->used += len;
}

int
ndl_cache_load_str (cache *c,
                    const char *data)
{
  int l;
  l = strlen (data);
  
  return ndl_recache (c, data, l);
}

void
ndl_free_cache (cache *c)
{
  free (c->buf);
  INIT_CACHE(c);
}

/*
const char *
ndl_diagname (ndl_sd nsd,
              query_sd qsd,
              int server,
              cache *c,
              const byte *datagram,
              int dglen,
              int ctype)
{
  ndl_si nsi;
  
  ns = no_memory;//ndl_take_names (nsd, qsd, server, c, datagram, dglen, &ctype, dglen);
  if (ns == no_memory)
    {
      return "<cannot report domain... out of memory>";
    } 
  
  if (ns)
    {
      c->used = 0;
      if (!(ndl_cache_load_str(c, "<bad format..") && 
            ndl_cache_load_str(c, strerr (ns)) &&
            ndl_cache_load_str(c, ">") && ndl_recache(c, "", 1)))
        {
          return "<cannot report bad format... out of memory>";
        }
    }
  
  if (!c->used)
    {
      ndl_cache_load_str (c, "<truncated ...>");
      ndl_recache (c, "", 1);  
    }
  
  return c->buf;
}

ndl_si
dnsdump (type_values type,
         const char **type_name,
         const char **format_name,
         int *len,
         const void *datap,
         char **datar)
{
  ndl_si nsi;
  const typeinfo *ti;
  cache c;
 
  ti = gettype (type);
  if (!ti)
    return unknown_rr_type;
  
  if (type_name)
    *type_name = ti->type_name;
  
  if (format_name)
    *format_name = ti->format_name;
  
  if (len)
    *len = ti->i;
  
  if (!datap)
    return dns_ok;
  
  INIT_CACHE(&c);
  
  ns = ti->convstr (&c, datap);
  
  if (ns)
    goto j_freec;
  
  if (!ndl_recache (&c, "", 1))
    {
      ns = no_memory;
      goto j_freec;
    }

  assert (strlen (c.buf) == c.used - 1);
  
  *datar = realloc (c.buf, c.used);
  
  if (!*datar)
    *datar = c.buf;
  
  return dns_ok;

  j_freec:
    return ns;
}
*/
/* warning infomation, i:index, o:output  */
#define WINFO(i, o) {i, o}

static const struct winfo
{
  ndl_si nsi;
  const char *string;
} winfos[] =
  {
    WINFO (dns_ok, "successful"),
    
    WINFO (no_memory, "Have not available memory."),
    WINFO (unknown_rr_type, "The query type not implemented in NDL."),

    WINFO (timeout, "DNS query timeout."),
    WINFO (all_server_fail, "All name servers failed."),
    WINFO (no_recurse, "Recursion method denied by name server."),
    WINFO (invalid_response, "The name server do a bad response."),
    WINFO (unknown_format, "The name server used unknown format."),
    
    WINFO (inconsistent, "Inconsistent resource record in DNS."),
    WINFO (prohibited_name, "DNS data refers to an alias cname."),
    WINFO (answer_name_invaild, "Answer domain name is invalid."),
    WINFO (answer_name_too_long, "Answer domain name is too long."),
    WINFO (invaild_data, "Invaild DNS data."),

    WINFO (query_name_wrong, "Have not such domain name for particular DNS query type."),
    WINFO (query_name_invalid, "Query domain name is invalid."),
    WINFO (query_name_too_long, "Query domain name is too long."),
    
    WINFO (no_domain, "No such domain."),
    WINFO (no_data, "No such data."),
    
    WINFO (FORMAT_ERROR, "The name server was unable to interpret the query."),
    WINFO (SERV_FAILURE, "The name server was unable to process this query ."),
    WINFO (NAME_ERROR, "This code signifies that the domain name referenced in the query does not exist."),
    WINFO (NOT_IMPLEMENTED, "The name server does not support the requested kind of query."),
    WINFO (REFUSED, "The name server refuses to perform the specified operation for policy reasons."),
  };

static int
winfocmp (const void *key,
          const void *val)
{
  const ndl_si *nsi = key; 
  const struct winfo *w = val; 
  
  return *nsi < w->nsi ? -1 : *nsi > w->nsi ? 1 : 0; 
}

const char *
dnsprinte (ndl_si nsi)
{
  const struct winfo *w;
  static char buf[128];
  
  w = bsearch (&nsi, winfos, sizeof (winfos) / sizeof (*w), sizeof (*w), winfocmp);
  
  if (w)
    return w->string;
  
  snprintf (buf, sizeof (buf), "code %d", nsi);
   
  return buf; 
}

/*
void 
ndl_isort (p1, p2)
      void p1;
      void p2;
{


}
*/

