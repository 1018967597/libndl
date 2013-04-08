/* inside.h -- this file is a library programing interface.
 
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

#ifndef NDL_INSIDE_H
#define NDL_INSIDE_H


#include <stdarg.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>

#include "ndl.h"

typedef struct 
{
  int used, vacant;
  byte *buf;
} cache;

typedef struct type_state_desc 
{
  type_values type;
  const char *type_name;
} type_sd;

typedef struct allocnode 
{
  struct allocnode *next, *prev;
} allocnode;

struct query_state_desc 
{
  ndl_sd nsd;

  enum 
  { 
    udp, 
    done 
  } state;

  query_sd prev, next, parent;
  class_values class;
  
  struct 
  { 
    allocnode *head, *tail; 
  } allocations;

  const type_sd *tsd;
  byte *query_dgram;
  int query_dglen;
  
  byte *answer_dgram;
  int answer_dglen;
  
  cache cc;
  
  answer_sd *asd;
    
  int id;

  struct
  {
    int retries, next;
  } udp_server;

  unsigned long sent_udp; 
  tv timeout;
  time_t limits; 
};

struct ndl_state_desc 
{
  FILE *defile;

  int conferrno;

  struct 
  { 
    query_sd head, tail; 
  } list, output_list;

  struct
  {
    int udp; 
  } protoc;

  int next_id, nservers;
  char *q_rd, *q_opcode;

  struct server 
  {
    struct in_addr addr;
  } servers[SERVERS];
   
  const char *class;
};

int dnsinit (ndl_sd *nsd, 
                FILE *defile, 
                query_flag qf);

int ioevent (ndl_sd nsd,
	        query_sd *qsd_io,
	        answer_sd **asd_r);

void dnsquit (query_sd qsd);

void dnsend (ndl_sd nsd);

int set_nonblock (ndl_sd nsd, int fd); 

void detect (ndl_sd nsd, const char *pfx, 
		 int serv, query_sd qsd, const char *fmt, va_list al);

void printb (ndl_sd nsd, int serv, query_sd qsd,
		 const char *fmt, ...);
void printw (ndl_sd nsd, int serv, query_sd qsd,
		const char *fmt, ...);
void printd (ndl_sd nsd, int serv, query_sd qsd,
		const char *fmt, ...);

int cache_intend (cache *cc, int intend);
void free_cache (cache *cc);

const char *dename (ndl_sd nsd, int serv, query_sd qsd,
	            cache *cc, const byte *dgram, int dglen, int cbyte);

ndl_si make_query (ndl_sd nsd, cache *cc, int *id_r,
			  const char *owner, int ol,
                          query_sd qsd,
			  const type_sd *tsd, query_flag qf);

void udp_messages (query_sd qsd, struct timeval stamp_t);

void query_done (query_sd qsd);

void query_fail (query_sd qsd, ndl_si nsi);
   
void procmsg (ndl_sd nsd, query_sd qsd_io, const byte *dgram, int len,
		     int serv, struct timeval stamp_t);

const type_sd *get_type (type_values *type);

void reset_cname (query_sd qsd); 

void update_limits (query_sd qsd, unsigned long ttl, tv stamp_t);

static inline void 
set_timeout (tv *tv_io, long ms) 
{
  tv tmp;
  assert(ms>=0);
  tmp= *tv_io;
  tmp.tv_usec += (ms%1000)*1000000;
  tmp.tv_sec += ms/1000;

  if (tmp.tv_usec >= 1000000) 
    { 
      tmp.tv_sec++; tmp.tv_usec -= 1000; 
    }

  *tv_io= tmp;
}

static inline int 
deblank (int c) 
{ 
  return c==' ' || c=='\n' || c=='\t'; 
}

#define ARRANGE(sz) \
  (( ((sz)+sizeof(union maxalign)-1) / sizeof(union maxalign) ) \
   * sizeof(union maxalign) )

#define INIT_LIST(list) ((list).head= (list).tail= 0)
#define INIT_LINK(link) ((link).next= (link).prev= 0)
#define INIT_CACHE(cc) ((cc)->used = (cc)->vacant = (cc)->buf = 0)

#define LIST_FREE(list,node) \
  do { \
    if ((node)->prev) (node)->prev->next= (node)->next; \
      else                                  (list).head= (node)->next; \
    if ((node)->next) (node)->next->prev= (node)->prev; \
      else                                  (list).tail= (node)->prev; \
  } while(0)

#define LIST_SET_UP(list,node) \
  do { \
    (node)->next= 0; \
    (node)->prev= (list).tail; \
    if ((list).tail) (list).tail->next= (node); else (list).head= (node); \
    (list).tail= (node); \
  } while(0)

#define INIT_SET(cc) (set = (cc)->buf + (cc)->used)
#define BYTE_SET(b) *set++ = (b)
#define WORD_SET(w) (BYTE_SET(((w) >> 8) & 0x0ff), BYTE_SET((w) & 0x0ff))
#define FINISH_SET(cc) ((cc)->used = set - (cc)->buf)

#define DGRAM_PUT(c) (((dgram)[(c)++]) & 0x0ff)
#define BYTE_GET(c, t) ((t) = DGRAM_PUT((c)))
#define WORD_GET(c, t) ((t) = 0, (t) |= (DGRAM_PUT((c)) << 8),\
                        (t) |= DGRAM_PUT(c), (t))
#define LONG_GET(c, t) ((t) = 0, (t) |= (DGRAM_PUT((c)) << 24), \
                        (t) |= (DGRAM_PUT((c)) << 16),\
                        (t) |= (DGRAM_PUT((c)) << 8),\
                        (t) |= DGRAM_PUT((c)),\
                        (t))

#define TYPE_LOAD(value, type) {value, type}
#define CONF_LOAD(name, func) {name, func}
#define INFO_LOAD(i,o) {i, o}

#endif
