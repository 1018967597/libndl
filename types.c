/* types.c -- this file parse resource record type values and
   the machinery to call it.
 
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

#define NO_MEMORY return no_memory

static ndl_status 
parse_inaddr (psi, cbyte, max, datap)
      const parseinfo *psi;
      int cbyte;
      int max; 
      void *datap;                         
{
  struct in_addr *storeto = datap;
  
  if (max - cbyte != 4)
    return invaild_data;
  
  memcpy (storeto, psi->datagram + cbyte, 4);

  return dns_ok;
}

static int
search_sortlist (nds, ia)
       ndl_state nds;
       struct in_addr ia;
{
  const struct sortlist *p;
  int i;
  
  for (i = 0, p = nds->sortlist;
       i < nds->nsortlist 
       && !((ia.s_addr & p->mask.s_addr) == p->base.s_addr);
       i++, p++);

  return i;
}

static int
dip_inaddr (nds, a, b)
    ndl_state nds;
    struct in_addr a;
    struct in_addr b;
{
  int i, j;

  if (!nds->nsortlist)
    return 0; 
  i = search_sortlist (nds, a);
  j = search_sortlist (nds, b);
  
  return i > j;
}
#if 0
static int 
di_addr (nds, parse_data_a, parse_data_b)
   ndl_state nds;
   const void *parse_data_a; 
   const void *parse_data_b; 
{
  const ndl_rr_addr *i = parse_data_a, *j = parse_data_b;  

  assert (i->addr.sa.sa_family == AF_INET);

  return dip_inaddr (nds, i->addr.sai.sin_addr, j->addr.sai.sin_addr);
}
#endif
static int
di_inaddr (nds, parse_data_a, parse_data_b)
   ndl_state nds; 
   const void *parse_data_a; 
   const void *parse_data_b;
{
  const struct in_addr *i = parse_data_a, *j = parse_data_b;
  
  return dip_inaddr (nds, *i, *j);
}

static ndl_status
cs_inaddr (vb, datap)
   vbuf *vb;
   const void *datap;
{
  const struct in_addr *rrp = datap, rr = *rrp;
  const char *ia;

  ia = inet_ntoa (rr);
  assert (ia);
  //CSP_ADDSTR (ia);
 
  return dns_ok;
}

static void 
mf_flat (nqs, data)
   ndl_query nqs;
   void *data;
{

}
#define TYPESZ_M(member)   (sizeof (*((ndl_answer*)0)->rrs.member))

#define DEEP_MEMB(memb) TYPESZ_M(memb), mf_##memb, cs_##memb
#define FLAT_MEMB(memb) TYPESZ_M(memb), mf_flat, cs_##memb

#define DEEP_TYPE(type_values, type_names, format_names, memb,  parser, comparer) \
{type_values, type_names, format_names, TYPESZ_M(memb), mf_##memb, cs_##memb, parser, comparer} 

#define FLAT_TYPE(type_values, type_names, format_names, memb,  parser, comparer) \
{type_values, type_names, format_names, TYPESZ_M(memb), mf_flat, cs_##memb, parser, comparer}

static const type_i 
typeinfos[] = 
{
  FLAT_TYPE(A, 'A', 0, inaddr, parse_inaddr, di_inaddr),
#if 0
  DEEP_TYPE(cname, "CNAME", 0, str, parse_host_raw, 0),
  DEEP_TYPE(soa, "SOA", "raw", soa, parse_soa, 0), 
  DEEP_TYPE(ptr, "PTR", "raw", str, parse_raw, 0), 
#endif
};

const type_i 
*ndl_get_type (type_values type)
{
  const type_i *i, *j, *mid;
  
  i = typeinfos;
  j = typeinfos + (sizeof (typeinfos) / sizeof (typeinfo));
  
  //printf ("typeinfo is %d\n", sizeof (typeinfos) / sizeof (typeinfo));
  //printf ("typeinfos i is %s\n", i);
  
  while (i < j)
    {
      mid = i + ((j - i) >> 1);
      if (mid->type == type)
        //printf ("mid is: %d\n", mid->type);
        return mid;
      if (mid->type < type)
        i = mid + 1;
      else
        j = mid;
    } 
   
  return 0; 
}

