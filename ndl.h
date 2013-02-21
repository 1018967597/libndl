/* ndl.h -- this file is user application programing interface.
  
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



#ifndef NDL_H 
#define NDL_H 

#include <stdio.h>


#include <sys/socket.h>
#include <netinet/in.h>

/* Ndl state description, global status.  */

typedef struct ndl_state_desc *ndl_sd;

/* Query state description, query status. */

typedef struct query_state_desc *query_sd;

/* Answer state description, answer status 
   inited in query struct.  */

typedef struct answer_state_desc answer_sd;

/*  */

typedef struct lable_state_desc *label_sd;

/* 
struct 
{
  unsigned int code_1:1;
  unsigned int code_2:1;
  unsigned int code_3:1;
  unsigned int code_4:1;
  unsigned int code_5:1;
  unsigned int code_6:1;
  unsigned int code_7:1;
} flags;
*/
/* base definitions  

#define MAX_NAME 1024
#define MAX_SERV 7
#define SEARCH_LIST 5  */

#define DNS_HEADER_SIZE 12

/* Transport - The DNS assumes that messages will be transmitted as
   datagrams or in a byte stream carried by virtual cirecuit.  
   
   The Internet supports name server access using TCP[RFC-793]/UDP[RFC-768]
   on server port 53 (decimal).  */

#define SERVER_PORT 53
   
/* UDP usage - Messages carried by UDP are restricted to 512 bytes 
   (not counting the IP or UDP headers).  */
 
#define UDP_MESSAGES 512

/* */
 
#define EDNS0_UDP_SIZE 4096

/* Various objects and parameters in the DNS hava size limits (octets).  */

#define LABELS 63
#define NAMES 255

/* RR DEFINITIONS  */

typedef enum 
{
  /* TYPE fields are used in resource records.
     Note that these types are a subset of QTYPEs.  */
  A =     1,   /* a host address, according to RFC1035.  */
  MD =    3,   /* an authoritative name server  */
  MF =    4,   /* a mail destination (Obsolete - use MX)  */
  CNAME = 5,   /* a mail forwarder (Obsolete - use MX)  */
  SOA =   6,   /**/
  MB =    7,   /**/
  MG =    8,   /**/
  MR =    9,   /**/
  NUL =  10,   /**/
  WKS =   11,  /**/
  PTR =   12,  /**/
  HINFO = 13,  /**/
  MINFO = 14,  /**/
  MX =    15,  /**/
  TXT =   16,  /**/
} type_values;

/* QTYPE fields appear in the question part of a query.
   QTYPEs are a superset of TYPEs, hence all TYPEs are valid QTYPEs.  */

typedef enum 
{
  AXFR =  252,
  MAILB = 253,
  MAILA = 254,
  QTALL = 255
} qtype_values;
 
typedef enum 
{
  /* CLASS fields appear in resource records.  */ 
  IN = 1,  /* The Internet  */
  CS = 2,  /* The CSNET class  */
  CH = 3,  /* The CHAOS class  */
  HS = 4,  /* Hesiod [Dyer 87]  */
} class_values;

/* QCLASS fields appear in the question section of query.  */

typedef enum
{
  QCALL = 255  /* Any class  */
} qclass_values;

/* Header section format. according to RFC1035.
   Response code - this 4 bit field is set as part of responses.  
   
   0: No error condition.
   1: The name server was unable to interpret the query.
   2: The name server was unable to process this query due to 
      a problem with the name server.
   3: This code signifies that the domain name referenced in the query does not exist.
   4: The name server does not support the requested kind of query.
   5: The name server refuses to perform the specified operation for policy reasons.  */

typedef enum
{
  NO_ERROR =          0,
  FORMAT_ERROR =      1,
  SERV_FAILURE =      2, 
  NAME_ERROR =        3, 
  NOT_IMPLEMENTED =   4,  
  REFUSED =           5  
} rcode;

/*
typedef enum
{
  no_check_env =      0x0001,
  no_print_error =    0x0002,
  print_debug =       0x0004
} dns_initsigns;
*/

/* ndl_status_info  */
typedef enum 
{
  dns_ok,
  
  /* locally induced errors */
  no_memory,
  unknown_rr_type, 
  
  /* */
  timeout,
  all_server_fail,
  no_recurse,
  invalid_response,
  unknown_format,

  /**/
  max_temp_fail,
  
  /**/
  inconsistent,
  prohibited_name,
  answer_name_invaild,
  answer_name_too_long,
  invaild_data,
  max_config_fail,
  
  /**/
  query_name_wrong,
  query_name_invalid,
  query_name_too_long,
  max_query_fail,
  
  /**/ 
  no_domain,
  no_data,
} ndl_si;


/*  */
typedef struct
{
  int len;
  union
  {
    struct sockaddr sa;
    struct sockaddr_in sai;
  } addr;
} ndl_rr_addr;

/* Initialization function  */
int dnsinit (ndl_sd *nsd, FILE *configfile);


/* DNS transform function.  */
int dnstransf (ndl_sd nsd, query_sd *qsd,  
               const char *qname, type_values qtype, 
               void *context);
/*
int dnstransf (ndl_sd nsd,
               query_sd *qsd,  
               const char *qname, 
               type_values qtype, 
               class_values qclass, 
               void *context,
               flags *opt_flg);
*/

/* I/O multiplexing  
int dnscheck (ndl_sd nsd, query_sd qsd, 
              answer_ad **asd, void **context);

int dnswait (ndl_sd nsd, query_sd *qsd, 
             answer_sd **asd, void **context);
*/

/* DNS output infomation.  
void dnsdump (type_values type, 
              const char **name
              const char **fmt_name, 
              int len
              const void *data_p, 
              const void *data_r);
*/

/* DNS cancel  */
//void dnsremove (query_sd qsd);
//void dnspause (void perm1, void perm2);
//void dnsexit (void perm1, void perm2);

/* DNS finish  */
//void dnsend (void perm);
//void dnsquit (ndl_sd nsd);
/*
int callback (ndl_sd nsd, int max_fd, const fd_set *read_fds, 
              const fd_set *write_fds, const fd_set *except_fds);
*/
/*
int interest (ndl_sd nsd, int max_fd_io, const fd_set *read_fds_io, 
              const fd_set *write_fds_io, const fd_set *except_fds_io
              tv **tv_mod, tv *tv_buf); 
*/
/**/
//const char *dnserrno (ndl_status se);
#endif
