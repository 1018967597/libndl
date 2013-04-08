/* detect.c -- this file is a some diagnose functions. 

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

#include "inside.h"

void 
detect (ndl_sd nsd, 
        const char *pfx, 
        int server, 
        query_sd qsd, 
        const char *fmt, 
        va_list al) 
{
  const char *bef, *aft;
  cache cc;
  
  if (!nsd->defile)
    return;

  fprintf (nsd->defile,"DNS%s: ",pfx);

  vfprintf (nsd->defile,fmt,al);

  bef = " (";
  aft = "\n";

  if (qsd && qsd->query_dgram) 
    {
      INIT_CACHE (&cc);
      fprintf(nsd->defile,"%sQNAME=, QTYPE=%s, CLASS=%s, TTL=%ld", bef,
	      qsd->tsd ? qsd->tsd->type_name : "<unknown>", qsd->nsd ? qsd->nsd->class : "<unknown>");

      bef = ", "; 
      aft = ")\n";
    }
  
  if (server >= 0) 
    {
      fprintf (nsd->defile, "%sNS=%s", bef, inet_ntoa (nsd->servers[server].addr));
      bef = ", "; 
      aft = ")\n";
    }

  fputs (aft, nsd->defile);
}

void printb (ndl_sd nsd, 
             int server, 
             query_sd qsd, 
             const char *fmt, 
             ...) 
{
  va_list vl;

  va_start (vl,fmt);
  detect (nsd," debug", server, qsd, fmt, vl);
  va_end (vl);
}

void printw (ndl_sd nsd, 
             int server, 
             query_sd qsd, 
             const char *fmt, 
             ...) 
{
  va_list vl;

  va_start (vl,fmt);
  detect (nsd, " warning", server, qsd, fmt, vl);
  va_end(vl);
}

void printd (ndl_sd nsd, 
             int server, 
             query_sd qsd, 
             const char *fmt, 
             ...) 
{
  va_list vl;

  va_start (vl,fmt);
  detect (nsd, "", server, qsd, fmt, vl);
  va_end (vl);
}

int 
cache_intend (cache *cc, 
              int intend) 
{
  void *p;
  
  if (cc->vacant >= intend) 
    return 1;

  p = realloc (cc->buf, intend); 
  if (!p) 
    return 0;

  cc->buf = p;
  cc->vacant = intend;
  return 1;
}

void 
free_cache (cache *cc) 
{
  free (cc->buf);
  INIT_CACHE (cc);
}

static const struct info {
  ndl_si nsi;
  const char *string;
} info_table[] = {
  INFO_LOAD (DNS_OK,               "Dns query is ok"                               ),
  
  INFO_LOAD (TIMEOUT,              "Dns query timed out"                           ),
  INFO_LOAD (ALL_SERVER_FAIL,      "All nameservers failed"                        ),
  INFO_LOAD (NO_RECURSE,           "Recursion denied by nameserver"                ),
  INFO_LOAD (INVALID_RESPONSE,     "Nameserver sent bad response"                  ),
  INFO_LOAD (UNKNOWN_FORMAT,       "Nameserver used unknown format"                ),

  INFO_LOAD (WITHOUT_MEMORY,       "Out of memory"                                 ),
  INFO_LOAD (UNKNOWN_RR_TYPE,      "Query not implemented in DNS library"          ),
  INFO_LOAD (GET_TIME_ERROR,       "Get time failure"                              ),
  INFO_LOAD (QUERY_STATE_LOAD_FAILURE,      "Query state load failure"             ),

  INFO_LOAD (INIT_CONF_ERROR,      "Initialization config error"                   ),
  INFO_LOAD (NO_PROTOC_OPT,        "There is no protocol can choose"               ),
  INFO_LOAD (SOCKET_ERROR,         "Socket return error"                           ),
  INFO_LOAD (NONBLOCK_ERROR,       "Non-blocking set the return error"             ),
  INFO_LOAD (EVENT_ERROR,          "I/O multiplexing select is failure"            ),
  INFO_LOAD (R_SERV_FAILURE,       "Nameserver reports failure"                    ),
  INFO_LOAD (R_FORMAT_ERROR,       "Query not understood by nameserver"            ),
  INFO_LOAD (R_NOT_IMPLEMENTED,    "Query not implemented by nameserver"           ),
  INFO_LOAD (R_REFUSED,            "Query refused by nameserver"                   ),
  INFO_LOAD (RCODE_UNKNOWN,        "Nameserver sent unknown response code"         ),

  INFO_LOAD (CLASS_ERROR,          "find class error"                              ),

  INFO_LOAD (INCONSISTENT,         "Inconsistent resource records in DNS"          ),
  INFO_LOAD (INVAILD_DATA,         "Found invalid DNS data"                        ),

  INFO_LOAD (QUERY_NAME_WRONG,     "Domain invalid for particular DNS query type"  ),
  INFO_LOAD (QUERY_NAME_INVALID,   "Domain name is syntactically invalid"          ),
  INFO_LOAD (QUERY_NAME_TOO_LONG,  "Domain name is too long"                       ),

  INFO_LOAD (R_NAME_ERROR,         "No such domain name"                           ),
  INFO_LOAD (WITHOUT_DATA,         "No such data"                                  )
};

static int 
icmp (const void *key, const void *elem) 
{
  const ndl_si *nsi = key;
  const struct info *i = elem;

  return *nsi < i->nsi ? -1 : *nsi > i->nsi ? 1 : 0;
}

const char  
*ostatinfo (ndl_si nsi) 
{
  static char buf[127];

  const struct info *inf;

  inf = bsearch (&nsi, info_table, sizeof(info_table)/sizeof(*inf), 
                 sizeof(*inf), icmp);
  if (inf) 
    return inf->string;
  
  snprintf (buf, sizeof(buf), "code %d", nsi);
  return buf;
}

