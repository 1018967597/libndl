/* ndl.h -- this file is user application programing interface.
  
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

#ifndef NDL_H
#define NDL_H

#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>


#define LABELS 63
#define NAMES 255
#define SERVERS 7 
#define SORTLIST 17

#define UDP_MAX_RETRIES 17

#define UDP_PERTRY_MS 2000 
#define LOCAL_RESOURCE_MS 20

#define MAX_TTL (7*86400) 

#define INADDR_ARPA "in-addr", "arpa"

#define SERVER_PORT 53
#define UDP_MESSAGES 512
#define DNS_HEADER_SIZE 12

typedef struct ndl_state_desc *ndl_sd;
typedef struct query_state_desc *query_sd;
typedef struct answer_state_desc answer_sd;

typedef unsigned char byte;
typedef struct timeval tv;

typedef enum 
{
  A		= 1,     NS	        = 2,     MD           = 3,	
  MF		= 4,     CNAME	        = 5,     SOA	      = 6,	
  MB		= 7,     MG	        = 8,     MR	      = 9,	
  NUL		= 10,    WKS	        = 11,    PTR	      = 12,	
  HINFO		= 13,    MINFO	        = 14,    MX	      = 15,	
  TXT		= 16,    RP	        = 17,    AFSDB	      = 18,	
  X25		= 19,    ISDN		= 20,    RT	      = 21,	
  NSAP		= 22,    NSAP_PTR	= 23,    SIG	      = 24,	
  KEY		= 25,    PX		= 26,    GPOS	      = 27,	
  AAAA		= 28,    LOC		= 29,    NXT	      = 30,	
  EID		= 31,    NIMLOC	        = 32,    SRV	      = 33,	
  ATMA		= 34,	 NAPTR		= 35,    KX	      = 36,    
  CERT		= 37,	 A6		= 38,    DNAME	      = 39,    
  SINK		= 40,	 OPT		= 41,    DS	      = 43,	
  SSHFP		= 44,    IPSECKEY	= 45,    RRSIG	      = 46,	
  NSEC		= 47,    DNSKEY	        = 48,    DHCID	      = 49,
  NSEC3		= 50,    NSEC3PARAMS	= 51,    TALINK	      = 58, 
  SPF		= 99,    UINFO		= 100,   UID	      = 101,
  GID		= 102,   UNSPEC	        = 103,   TSIG	      = 250,	
  IXFR		= 251,   AXFR		= 252,	 MAILB	      = 253,	
  MAILA		= 254,	 ANY		= 255,	 ZXFR	      = 256,	
  DLV		= 32769 
} type_values;

typedef enum ndl_state_info
{
  DNS_OK,

  TIMEOUT,
  ALL_SERVER_FAIL,
  NO_RECURSE,
  INVALID_RESPONSE,
  UNKNOWN_FORMAT,

  WITHOUT_MEMORY,
  UNKNOWN_RR_TYPE,  
  GET_TIME_ERROR,
  QUERY_STATE_LOAD_FAILURE,

  INIT_CONF_ERROR,
  NO_PROTOC_OPT,
  SOCKET_ERROR,
  NONBLOCK_ERROR,
  
  EVENT_ERROR,

  R_SERV_FAILURE,
  R_FORMAT_ERROR,      
  R_NOT_IMPLEMENTED,     
  R_REFUSED,             
  RCODE_UNKNOWN,

  CLASS_ERROR,

  TEMP_FAIL = 107,

  INCONSISTENT, 
  INVAILD_DATA,

  CONFIG_ERROR = 207,

  QUERY_NAME_WRONG,
  QUERY_NAME_INVALID,
  QUERY_NAME_TOO_LONG,

  QUERY_FAIL = 307, 

  R_NAME_ERROR, 
  WITHOUT_DATA
} ndl_si;

typedef enum query_flags 
{
  ANCOUNT,
  QDCOUNT,
  NSCOUNT         = ANCOUNT,
  ARCOUNT         = ANCOUNT,
  ID              = 0x7677,
  SQUERY          = 0X0100,
  SQUERY_TC       = 0x0300, 
  SQUERY_NO_RD    = 0x0000,
  IQUERY          = 0x0900,
  STATUS_QUERY    = 0x1100
} query_flag;

typedef enum 
{
  IN = 1, 
  CS = 2, 
  CH = 3,  
  HS = 4  
} class_values;

typedef enum
{
  NO_ERROR =          0,
  FORMAT_ERROR =      1,
  SERV_FAILURE =      2, 
  NAME_ERROR =        3, 
  NOT_IMPLEMENTED =   4,  
  REFUSED =           5  
} dns_rcode;

struct answer_state_desc 
{
  ndl_si nsi;
  type_values type; 
  time_t limits; 
  char *cname, *owner; 
  char *a_opcode, *a_qr, *a_rd, *a_ra, *a_rcode; 
  char *type_name, *class_name; 
  char *q_opcode, *q_rd;
  const byte *query_dgram, *answer_dgram; 
  int a_id, recv_dglen; 
  int qdc, anc, nsc, arc;
  int q_id, query_dglen;
};


int vtransf (ndl_sd nsd,
             query_sd *qsd_r,
             answer_sd *asd_r,
             const char *owner,
             type_values *type,
             class_values class,
             query_flag qf);

const char *ostatinfo (ndl_si nsi);
#endif
