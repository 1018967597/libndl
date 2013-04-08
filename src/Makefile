#  Makefile -- make client source file.
#  
#  Copyright (C) 2012 2013 S.meng.
#    
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>. 

include ../config.mk
include client.mk

CLIENT_C:= client.c 

client: $(CLIENT_C)
	$(CC) $(CFLAGS) $< -L $(LIBDIR) -I $(LIBDIR) -l$(NDL_NAME) -o $@ 

