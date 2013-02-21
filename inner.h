/* inner.h -- this file is a library programing interface.
 
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



#ifndef INNER_H
#define INNER_H

#include <stdarg.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>

#include <sys/time.h>

#include "ndl.h"

#define SERVERS 7 
#define SLIST 17
#define TTL (7*86400) 


typedef unsigned char byte;
typedef struct timeval tv;

typedef struct
{
  int used, avail;
  byte *buf;
} cache;

typedef struct
{
  ndl_sd nsd;
  query_sd qsd;
  int server;
  const byte *datagram;
  int dglen, nsstart, nscount, arcount;
  tv stamp_t;
} parse_i;

typedef struct 
{
  void *ctx;
  
  void 
  (*callback) (query_sd parent, 
               query_sd child);
  union
  {
    ndl_rr_addr ptr_parent_addr;
    //ndl_rr_host_addr *host_addr;
  } info;
} query_context;

typedef struct nodealloc
{
  struct nodealloc *prev, *next;
} Node;

typedef struct
{
  type_values type;
  const char *type_name;
  const char *format_name;
  int i;
  //int rrsz;
  
  void 
  (*makefinal) (query_sd qsd, 
                void *data);
  
  ndl_si 
  (*convstr) (cache *c, 
              const void *data);

  ndl_si 
  (*parse) (const parse_i *psi,
            int cbyte, 
            int max, 
            void *store_r);
       
  int 
  (*diff_needswap) (ndl_sd nds, 
                    const void *datap_a, 
                    const void *datap_b); 
} type_i;

struct answer_state_desc
{
  /* some a error status  */
  ndl_si nsi; 
  /* domain and cname  */ 
  char *cname;
  char *name;
  /* some query type values  */
  type_values type;
  
  time_t limits;
  /* crrs: count rrs number.
     rrsz: unkown.  */
  int crrs, rrsz;
  /* resource record struct  */
  union
  {
    void *untyped;
    unsigned char *bytes;
    char *(*str);
    ndl_rr_addr *addr;
    struct in_addr *ia;
  } rrs;
};

struct query_state_desc
{
  ndl_sd nsd;

  enum
  {
    query_udp,
    query_tcp_wait,
    query_tcp_sent,
    query_child,
    query_done
  } state;

  struct
  {
    query_sd prev, next, parent;
  } tree;

  struct
  {
    query_sd head, tail;
  } children;

  struct
  {
    query_sd prev, next;
  } siblings;

  struct
  {
    Node *head, *tail;
  } node;

  //int interim_allocd;
  //void *final_allocspace;

  const type_i *ti;
  byte *query_datagram;
  int query_dglen;
  
  cache c;
   
  /*  */ 
  answer_sd *asd;
  
 // byte *cname_datagram;
 // int cname_dglen, cname_begin;
  /* 
  cache search_c;
  int search_orglen, search_pos, search_done_abs;
  */
  
  int id, flags;
 
  struct
  {
    int retries, next;
  } udp_server;
  
  unsigned long udp_sent, tcp_failed;
  tv timeout;
  time_t limits;
  
  query_context qc;
};

struct ndl_state_desc
{
  FILE *file;
  int conferrno;

  struct 
  {
    query_sd head, tail;
  } time_wait, child_wait, output;
  
  struct serv 
  {
    struct in_addr addr;
  } server[SERVERS]; 
  
  int servers, sortlist;
  
  struct 
  {
    int udp, tcp;
  } protoc;

  int next_id;

  cache tcp_write, tcp_read;
 
  enum tcp_connect_status
  {
    disconnected,
    connecting,
    ok
  } tcp_status;

  tv tcp_timeout;
  struct sigaction sigpipe;
  sigset_t sigmask;
};
/* 
  struct sortlist
  {
    struct in_addr base, mask; 
  } sortlist[MAXSORTLIST];
*/

struct label_state_desc
{
  ndl_sd nsd;
  query_sd qsd;
  int server;
  const byte *datagram;
  int dglen, max, ctype, nl;
  int *name_end;
};

union maxalign
{
  byte d[1];
  struct in_addr ia;
  long l;
  void *p;
  void (*fp) (void);
  union maxalign *up;
} data;

/* From types.c  */
const type_i *ndl_get_type (type_values type);

/* From diagnosis.c  */
/* diag is mainly  */
void ndl_diag (ndl_sd nsd, query_sd qsd, const char *tell,
               int server, const char *format, va_list vl);

/* print error  */
void ndl_printb (ndl_sd nsd, query_sd qsd, int server, 
                 const char *format, ...);

/* print warning  */
void ndl_printw (ndl_sd nsd, query_sd qsd, int server, 
                 const char *format, ...);

/* print diagnostic  */
void ndl_printd (ndl_sd nsd, query_sd qsd, int server, 
                 const char *format, ...);

int ndl_cache_intend (cache *c, int intend);

int ndl_recache (cache *c, const byte *data, int len);

void ndl_cache_load (cache *c, const byte *data, int len);

int ndl_cache_load_str (cache *c, const char *data);

void ndl_free_cache (cache *c);

const char *ndl_diagname (ndl_sd nsd, query_sd qsd, int server,cache *c, 
                          const byte *datagram, int dglen,int ctype);

/* From messages.c  */

/* From analysis.c */
void ndl_take_labels (label_sd *lsd, ndl_sd *nsd, query_sd *qsd,
                      int server, const byte *datagram, int dglen,
                      int max, int name_begin, int *name_end);

void ndl_parse_labels (label_sd *lsd, int lablen, int label_begin);

ndl_si ndl_take_names (ndl_sd nsd, query_sd qsd, int server,
                       cache *c, const byte *dgram, int dglen,
                       int *cbyte_io, int max);

ndl_si ndl_parse_names (ndl_sd nsd, query_sd qsd, int server,
                        cache *c, const byte *dgram, int dglen,
                        int *cbyte_io, int max);

ndl_si ndl_find_rrs (query_sd qsd, int server, const byte *datagram,
                     int dglen, int *cbyte_io, int *type,
                     int *class, unsigned long *ttl, int *rdlength,
                     int *rdstart, const byte *eo_dgram, int eo_dglen,
                     int eo_cbyte, int *eo_matched_r);

ndl_si ndl_parse_rrs (query_sd qsd, int server, const byte *datagram,
                      int dglen, int *cbyte_io, int *type, int *class,
                      unsigned long *ttl, int *rdlength, int *rdstart,
                      int *name_matched_query);

/* From tcp.c */
void ndl_tcp_fail (ndl_sd nsd, const char *what, const char *why);

void ndl_tcp_tryconnect (ndl_sd nsd, tv stamp_t);

void ndl_tcp_protect (ndl_sd nsd);

void ndl_tcp_unprotect (ndl_sd nsd);

/* static inline functions  */
static inline int deblank (int c) 
{ 
  return c == ' ' || c == '\n' || c == '\t';
}

static inline int dedigital (int c)
{
  return c >= '0' && c <= '9';
}

static inline int dealpha (int c)
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

/* Macros  */
#define INIT_TREE(t) ((t).prev = (t).next = (t).parent = 0)
#define INIT_QUEUE(q) ((q).head = (q).tail = 0)
#define INIT_LINK(l) ((l).prev = (l).next = 0)
#define INIT_CACHE(c) ((c)->used = (c)->avail = (c)->buf = 0) 

#define PROC_MEM(l)\
        (((l) + sizeof (union maxalign) - 1) / sizeof (union maxalign)) * sizeof (union maxalign)

#define LIST_LINK_TAIL(list, node) LIST_LINK_TAIL_PART (list, node,)

#define LIST_LINK_TAIL_PART(list, node, part)\
  do\
    {\
      if ((node)->part prev)\
        (node)->part prev->part prev->part next = (node)->part next;\
      else\
        (list).head = (node)->part next;\
      if ((node)->part next)\
        (node)->part next->part next->part prev = (node)->part prev;\
      else\
         (list).tail = (node)->part prev;\
    } while (0)
  
/* Send function  */
//void _send (void perm1, void perm2, void perm3, void perm4);

/* read functon  */
//void _read (void perm1, void perm2, void perm3);

/* */
//static int nextword (void perm1, void perm2, void perm3);

/* */
//static int rank (void perm1, void perm2, void perm3);

/* */
//int _write (void perm1, void perm2);

/* */
//int squeeze (char s[], int c);

/* */
//int getbits (unsigned x, int p, int n);

/* */
//int setbits (unsigned x, int p, int n, unsigned y);

/* */
//int invert (unsigned x, int p, int n);

/* */
//int rightrot (unsigned x, int n);

/* */
int getfile (FILE *file, const char *filename, 
             int lno, char *buf, int buflen);

//int gettext (void perm1, void perm2);

 
#endif
