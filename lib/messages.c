/* messages.c -- this file is used to assembly and 
   distribute datagram.
  
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

#include <sys/uio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>

#include "inside.h"

static ndl_si 
*classof (query_sd qsd, 
          class_values cv)
{
  if (cv == IN)
    {
      return "IN";
    }
  else if (cv == CS)
    {
      return "CS";
    }
  else if (cv == CH)
    {
      return "CH";
    }
  else if (cv == HS)
    {
      return "HS"; 
    }
  else
    {
      query_fail (qsd, CLASS_ERROR);
      return;
    }
}

static ndl_si 
make_header (ndl_sd nsd, 
             cache *cc, 
             int *id_r, 
             int qdlen,
             query_flag qf) 
{
  int id;
  byte *set;
  
  if (!cache_intend (cc, DNS_HEADER_SIZE + qdlen + 4)) 
    return WITHOUT_MEMORY;

  cc->used = 0;
  INIT_SET (cc);
  
  *id_r = id = (nsd->next_id++) & 0x0ffff;
  WORD_SET (id);
  BYTE_SET (qf >> 8); 
  BYTE_SET (qf & 0x00ff); 
  WORD_SET ((query_flag)QDCOUNT);
  WORD_SET ((query_flag)ANCOUNT); 
  WORD_SET ((query_flag)NSCOUNT); 
  WORD_SET ((query_flag)ARCOUNT); 
  
  FINISH_SET (cc);
  
  if (!((qf & 0xff) >> 4))
    nsd->q_opcode = "QUERY"; 
  
  if ((qf >> 4) & 0xff)
    nsd->q_rd = "rd";
     
  return DNS_OK;
}

ndl_si
make_owner (cache *cc,
            const char *owner,
            int ol)
{
  int ll, c, cls;
  byte label[NAMES], *set;
  const char *p, *q;

  
  INIT_SET (cc);

  p = owner; 
  q = owner+ol;
  cls= 0;

  while (p != q) 
    {
      ll = 0;
      while (p != q && (c= *p++)!='.') 
        {
          if (ll == sizeof(label)) 
            return QUERY_NAME_INVALID;

          label[ll++]= c;
        }

      if (!ll) 
        return QUERY_NAME_INVALID;

      if (cls++ > LABELS) 
        return QUERY_NAME_TOO_LONG;

      BYTE_SET (ll);
      memcpy(set, label, ll); 
      set += ll;
    }

  BYTE_SET (0);

  FINISH_SET (cc);
  
  return DNS_OK;
}

static ndl_si
make_question (cache *cc,
               const char *owner,
               int ol,
               class_values class,
               type_values type)
{
  ndl_si nsi;
  byte *set;
 
  nsi = make_owner (cc, owner, ol);
  if (nsi)
    return nsi;
  
  INIT_SET (cc);

  WORD_SET (type & 0x0ffff); 
  WORD_SET (class); 

  FINISH_SET (cc);
  assert(cc->used <= cc->vacant);
  
  return DNS_OK;
}

ndl_si 
make_query (ndl_sd nsd,
            cache *cc,
            int *id_r,
            const char *owner,
            int ol,
            query_sd qsd,
            const type_sd *tsd,
            query_flag qf)
{
  ndl_si nsi;
 
  nsi = make_header (nsd, cc, id_r, strlen (owner) + 2, qf);
  if (nsi)
    return nsi;
   
  nsi = make_question (cc, owner, ol, qsd->class, tsd->type);
  if (nsi)
    return nsi;
  
  return DNS_OK;
}

void 
udp_messages (query_sd qsd, 
              tv stamp_t) 
{
  struct sockaddr_in servaddr;
  int server, r;
  ndl_sd nsd;

  assert(qsd->state == udp);
  if (qsd->udp_server.retries >= UDP_MAX_RETRIES) 
    {
      query_fail (qsd, TIMEOUT);
      return;
    }

  server = qsd->udp_server.next;
  memset(&servaddr, 0, sizeof(servaddr));

  nsd = qsd->nsd;
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr = nsd->servers[server].addr;
  servaddr.sin_port = htons (SERVER_PORT);
  
  r = sendto (nsd->protoc.udp, qsd->query_dgram, 
              qsd->query_dglen, 0, &servaddr, sizeof(servaddr));

  qsd->asd->query_dglen = qsd->query_dglen; 
  qsd->asd->query_dgram = qsd->query_dgram;

  if (r < 0) 
    printw (nsd, server, 0, "sendto failed: %s", strerror (errno));
  
  set_timeout (&stamp_t, UDP_PERTRY_MS);
  qsd->timeout = stamp_t;
  qsd->sent_udp |= (1 << server);
  qsd->udp_server.next = (server + 1) % nsd->nservers;
  qsd->udp_server.retries++;
  LIST_SET_UP (nsd->list, qsd);
}

static query_sd 
query_state_load (ndl_sd nsd, 
                  const type_sd *tsd,
                  class_values cv, 
                  tv stamp_t) 
{
  query_sd qsd;
  
  qsd = malloc(sizeof(*qsd));  
  if (!qsd) 
    return 0;

  qsd->asd = malloc(sizeof(*qsd->asd));  
  if (!qsd->asd) 
    { 
      free(qsd); 
      return 0; 
    }
  
  qsd->nsd = nsd;
  qsd->nsd->class = classof (qsd, cv);
  qsd->state = udp;
  qsd->prev = qsd->next = qsd->parent= 0;
  INIT_LIST (qsd->allocations);
  qsd->class = cv;

  qsd->tsd = tsd;
  INIT_CACHE (&qsd->cc); 
  qsd->query_dgram = qsd->asd->query_dgram =  0;
  qsd->answer_dgram = qsd->asd->answer_dgram = 0;
  qsd->query_dglen = qsd->asd->query_dglen = 0;
  qsd->answer_dglen = 0;

  qsd->id = 0;
  qsd->udp_server.retries = 0;
  qsd->udp_server.next = 0;
  qsd->sent_udp = 0;
  timerclear (&qsd->timeout);
  qsd->limits = stamp_t.tv_sec + MAX_TTL;

  qsd->asd->nsi = DNS_OK;
  qsd->asd->cname = qsd->asd->owner = 0;
  qsd->asd->a_opcode = qsd->asd->a_rd = 0;
  qsd->asd->a_ra = qsd->asd->a_rcode = 0;
  qsd->asd->type_name = qsd->asd->class_name = 0;
  qsd->asd->type = tsd->type;
  qsd->asd->limits = -1;
  qsd->asd->recv_dglen = qsd->asd->q_id = 0;
  qsd->asd->a_id = qsd->asd->qdc = 0;
  qsd->asd->anc = qsd->asd->nsc = 0;
  qsd->asd->arc = 0;

  return qsd;
}

static void 
send_query (ndl_sd nsd, 
            query_sd qsd,
            const type_sd *tsd, 
            cache *qsdmsg_cc, 
            int id,
	    tv stamp_t) 
{
  qsd->cc= *qsdmsg_cc;
  INIT_CACHE (qsdmsg_cc);

  qsd->query_dgram = malloc (qsd->cc.used);

  if (!qsd->query_dgram) 
    { 
      query_fail (qsd, WITHOUT_MEMORY); 
      return; 
    }
  
  qsd->id = id;
  qsd->query_dglen = qsd->cc.used;
  memcpy (qsd->query_dgram, qsd->cc.buf, qsd->cc.used);
  
  udp_messages (qsd, stamp_t);
  dnsmsg (nsd, qsd, 0, 0, 0, 0); 
}

static void 
create_query (ndl_sd nsd, 
              query_sd qsd,
	      const char *owner, 
              int ol,
	      const type_sd *tsd, 
              class_values class,
	      tv stamp_t,
              query_flag qf) 
{
  cache cc;
  int id;
  ndl_si nsi;

  INIT_CACHE (&cc);
  
  nsi = make_query (nsd, &cc, &id, owner, ol, qsd, tsd, qf);
  if (nsi) 
    { 
      query_fail (qsd, nsi); 
      return; 
    }

  send_query (nsd, qsd, tsd, &cc, id, stamp_t);
}

int 
vtransf (ndl_sd nsd,
         query_sd *query_r,
         answer_sd *asd_r,
         const char *owner,
	 type_values *type_r,
         class_values class,
         query_flag qf) 
{
  int r, ol;
  ndl_si nsi;
  const type_sd *tsd;
  tv stamp_t;
  query_sd qsd;
  query_flag id = ID;
  type_values *type = type_r; 
   
  r = dnsinit (&nsd, 0, id);
  if (r) 
    return 0; 

  tsd = get_type (type);
  if (!tsd) 
    {
      nsi = UNKNOWN_RR_TYPE;
      goto j_failure;
    }

  r = gettimeofday (&stamp_t, 0); 
  if (r) 
    {
      nsi = GET_TIME_ERROR;
      goto j_failure;
    }

  qsd = query_state_load (nsd, tsd, class, stamp_t); 
  if (!qsd) 
    {
      nsi = QUERY_STATE_LOAD_FAILURE;
      goto j_failure;
    } 

  *query_r = qsd;

  ol = strlen (owner);
  if (!ol)
    { 
      nsi = QUERY_NAME_INVALID; 
      goto j_failure; 
    } 

  if (ol > NAMES + 1) 
    { 
      nsi = QUERY_NAME_TOO_LONG; 
      goto j_failure; 
    } 

  create_query (nsd, qsd, owner,ol, tsd, qsd->class, stamp_t, qf);
  
  qsd->asd->type_name =  tsd->type_name; 
  qsd->asd->class_name =  nsd->class;
 
  io_event (nsd, &qsd, asd_r);

  dnsend (nsd);

  return 0;

 j_failure:
  query_fail (qsd, nsi);
  return 0;
}

static void 
free_query_sd (query_sd qsd) 
{
  allocnode *an, *ann;

  for (an = qsd->allocations.head; an; an = ann) 
    { 
      ann = an->next;
      free(an); 
    } 

  free_cache (&qsd->cc);
}

void 
dnsquit (query_sd qsd) 
{
  switch (qsd->state) 
    {
      case udp: 
        LIST_FREE (qsd->nsd->list,qsd);
        break;
      case done:
        LIST_FREE (qsd->nsd->output_list,qsd);
        break;
      default:
        abort();
    }

  free_query_sd(qsd);
  free(qsd->asd);
  free(qsd);
}

void 
update_limits (query_sd qsd, 
               unsigned long ttl, 
               tv stamp_t) 
{
  time_t max;

  assert (ttl <= MAX_TTL);
  max = stamp_t.tv_sec + ttl;
  if (qsd->limits < max) 
    return;

  qsd->limits = max;
}

void 
query_done (query_sd qsd) 
{
  answer_sd *asd_i;

  qsd->id = -1;
  asd_i = qsd->asd;
  asd_i->limits = qsd->limits;
}

void 
query_fail (query_sd qsd, 
            ndl_si nsi) 
{
  qsd->asd->nsi = nsi;
  query_done (qsd);
}

void 
procmsg (ndl_sd nsd, 
         query_sd qsd_io,
         const byte *dgram, 
         int dglen,
         int server, tv stamp_t) 
{
  int count_byte;
  int id, qoatrr, zr, qdcount, ancount, nscount, arcount;
  int qr, opcode, tc, rd, ra;
  query_sd qsd, nqsd;
  dns_rcode rcode;
 
  if (dglen < DNS_HEADER_SIZE) 
    {
      printd (nsd, server, 0, "received datagram too short for message header (%d)", dglen);
      return;
    }

  
  count_byte = 0;
  
  WORD_GET (count_byte, id);
  BYTE_GET (count_byte, qoatrr);
  BYTE_GET (count_byte, zr);

  WORD_GET (count_byte, qdcount);
  WORD_GET (count_byte, ancount);
  WORD_GET (count_byte, nscount);
  WORD_GET (count_byte, arcount);

  assert (count_byte == DNS_HEADER_SIZE);

  qr = qoatrr & 0x80;
  opcode = (qoatrr & 0x78) >> 3;
  tc = qoatrr & 0x02;
  rd = qoatrr & 0x01;
  ra = zr & 0x80;
  rcode = (zr & 0x0f);

  qsd_io->answer_dgram = malloc (dglen);

  memcpy (qsd_io->answer_dgram, dgram, dglen);

  qsd_io->asd->answer_dgram = qsd_io->answer_dgram;

  qsd_io->asd->recv_dglen = dglen;
  
  qsd_io->asd->a_id = id;
  
  if (!qr) 
    {
      printd (nsd, server, 0, "server sent us a query, not a response");
      return;
    }
  else
    {
      qsd_io->asd->a_qr = "qr"; 
    } 
  
  if (rd)
    qsd_io->asd->a_rd = "rd";
 
  if (ra)
    qsd_io->asd->a_ra = "ra";

  if (opcode) 
    {
      printd (nsd, server, 0, "server sent us unkstamp_tn opcode %d (wanted 0=QUERY)", opcode);
      return;
    }
  else
    {
       qsd_io->asd->a_opcode = "QUERY";
    }

  if (!qdcount) 
    {
      printd (nsd, server, 0, "server sent reply without quoting our question");
      return;
    } 
  else if (qdcount > 1) 
    {
      printd (nsd, server, 0, "server claimed to asd %d qsdestions with one message", qdcount);
      return;
    }
  else
    {
      qsd_io->asd->qdc = qdcount;
    }
  
  if (ancount)
    qsd_io->asd->anc = ancount;
   
  if (nscount)
    qsd_io->asd->nsc = nscount;
    
  if (arcount)
    qsd_io->asd->arc = arcount;
   

  for (qsd = nsd->list.head; qsd; qsd = nqsd) 
    {
      nqsd = qsd->next;
      if (qsd->id != id) 
        continue;

      if (dglen < qsd->query_dglen) 
        continue;

      if (memcmp (qsd->query_dgram+DNS_HEADER_SIZE,
	          dgram+DNS_HEADER_SIZE,
	          qsd->query_dglen-DNS_HEADER_SIZE))
        continue;

      break;
    }

  LIST_FREE (nsd->list, qsd);
   
  switch (rcode) 
    {
      case NO_ERROR:
        qsd_io->asd->a_rcode = "NO ERROR";
        break;
      case NAME_ERROR:
        qsd_io->asd->a_rcode = "NAME ERROR";
        break;
      case FORMAT_ERROR:
        qsd_io->asd->a_rcode = "FORMAT ERROR";
        printw (nsd, server, qsd, "server cannot understand our query (Format Error)");
        query_fail (qsd, R_FORMAT_ERROR);
        return;
      case SERV_FAILURE:
        qsd_io->asd->a_rcode = "SERVER FAILURE";
        query_fail (qsd, R_SERV_FAILURE);
        return;
      case NOT_IMPLEMENTED:
        qsd_io->asd->a_rcode = "NOT IMPLEMENTED";
        printw (nsd, server, qsd, "server claims not to implement our query");
        query_fail (qsd, R_NOT_IMPLEMENTED);
        return;
      case REFUSED:
        qsd_io->asd->a_rcode = "REFUSED";
        printw (nsd, server, qsd, "server refused our query");
        query_fail (qsd, R_REFUSED);
        return;
      default:
        qsd_io->asd->a_rcode = "UNKNOWN RCODE";
        printw (nsd, server, qsd, "server gave unkstamp_tn response code %d", rcode);
        query_fail (qsd, RCODE_UNKNOWN);
        return;
    }

  if (tc) 
    goto j_truncated;

  query_done (qsd);
  return;

 j_truncated:
  
  if (!tc) 
    {
      printd (nsd, server, qsd, "server sent datagram which points outside itself");
      query_fail (qsd, INVALID_RESPONSE);
      return;
    }
}

static const type_sd type_tables[] = {
  TYPE_LOAD (A,                    "A"),
  TYPE_LOAD (NS,                  "NS"),
  TYPE_LOAD (MD,                  "MD"),
  TYPE_LOAD (MF,                  "MF"),
  TYPE_LOAD (CNAME,            "CNAME"),
  TYPE_LOAD (SOA,                "SOA"),
  TYPE_LOAD (MB,                  "MB"),
  TYPE_LOAD (MG,                  "MG"),
  TYPE_LOAD (MR,                  "MR"),
  TYPE_LOAD (NUL,                "NUL"),
  TYPE_LOAD (WKS,                "WKS"),
  TYPE_LOAD (PTR,                "PTR"),
  TYPE_LOAD (HINFO,            "HINFO"),
  TYPE_LOAD (MINFO,            "MINFO"),
  TYPE_LOAD (MX,                  "MX"),
  TYPE_LOAD (TXT,                "TXT"),
  TYPE_LOAD (RP,                  "RP"),
  TYPE_LOAD (AFSDB,            "AFSDB"),
  TYPE_LOAD (X25,                "X25"),
  TYPE_LOAD (ISDN,              "ISDN"),
  TYPE_LOAD (NSAP,              "NSAP"),
  TYPE_LOAD (NSAP_PTR,      "NSAP_PTR"),
  TYPE_LOAD (SIG,                "SIG"),
  TYPE_LOAD (KEY,                "KEY"),
  TYPE_LOAD (PX,                  "PX"),
  TYPE_LOAD (GPOS,              "GPOS"),
  TYPE_LOAD (AAAA,              "AAAA"),
  TYPE_LOAD (LOC,                "LOC"),
  TYPE_LOAD (NXT,                "NXT"),
  TYPE_LOAD (EID,                "EID"),
  TYPE_LOAD (NIMLOC,          "NIMLOC"),
  TYPE_LOAD (SRV,                "SRV"),
  TYPE_LOAD (ATMA,              "ATMA"),
  TYPE_LOAD (NAPTR,            "NAPTR"),
  TYPE_LOAD (KX,                  "KX"),
  TYPE_LOAD (CERT,              "CERT"),
  TYPE_LOAD (A6,                  "A6"),
  TYPE_LOAD (DNAME,            "DNAME"),
  TYPE_LOAD (SINK,              "SINK"),
  TYPE_LOAD (OPT,                "OPT"),
  TYPE_LOAD (DS,                  "DS"),
  TYPE_LOAD (SSHFP,            "SSHFP"),
  TYPE_LOAD (IPSECKEY,      "IPSECKEY"),
  TYPE_LOAD (RRSIG,            "RRSIG"),
  TYPE_LOAD (NSEC,              "NSEC"),
  TYPE_LOAD (DNSKEY,          "DNSKEY"),
  TYPE_LOAD (DHCID,            "DHCID"),
  TYPE_LOAD (NSEC3,            "NSEC3"),
  TYPE_LOAD (NSEC3PARAMS,"NSEC3PARAMS"),
  TYPE_LOAD (TALINK,          "TALINK"),
  TYPE_LOAD (SPF,                "SPF"),
  TYPE_LOAD (UINFO,            "UINFO"),
  TYPE_LOAD (UID,                "UID"),
  TYPE_LOAD (GID,                "GID"),
  TYPE_LOAD (UNSPEC,          "UNSPEC"),
  TYPE_LOAD (TSIG,              "TSIG"),
  TYPE_LOAD (IXFR,              "IXFR"), 
  TYPE_LOAD (AXFR,              "AXFR"),
  TYPE_LOAD (MAILB,            "MAILB"),
  TYPE_LOAD (MAILA,            "MAILA"),
  TYPE_LOAD (ANY,                "ANY"),
  TYPE_LOAD (ZXFR,              "ZXFR"),
  TYPE_LOAD (DLV,                "DLV")
};

const type_sd 
*get_type (type_values *type) 
{
  const type_sd *begin, *end, *mid;

  begin = type_tables;  
  end = type_tables + (sizeof (type_tables) / sizeof (type_sd));

  while (begin < end) 
    {
      mid = begin + ((end - begin) >> 1);
      if (mid->type == type) 
        return mid;

      if (type > mid->type) 
        begin = mid + 1;
      else 
        end = mid;
    }

  return 0;
}
