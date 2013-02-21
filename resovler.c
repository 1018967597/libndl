/* resolver.c -- this file is a example.
  
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

#include <stdio.h>

#include "ndl.h"

int
main ()
{
  ndl_sd test_nsd;
  //type_values test_type;
  //query_sd test_qsd;
  //static const char *test_name = "www.baidu.com";
  
  //test_owner = "www.baidu.com";
  //printf ("test_owner is: %s\n", test_owner);  
  dnsinit(&test_nsd, 0);
  
  //dnstransf(test_nsd, &test_qsd, test_name, 1, 0);
  //dns_dump();  
  //dns_end();
  return 0;
}
