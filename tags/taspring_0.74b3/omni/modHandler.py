#!/usr/bin/env python
# -*- coding: UTF-8 -*-

#======================================================
 #            modHandler.py
 #
 #  Thu Sept 01 18:07 2005
 #  Copyright  2005  Josh Mattila
 #
 #  Other authors may add their names above!
 #
 #  jm6.linux@gmail.com
#======================================================
 
#
 #  This program is free software; you can redistribute it and/or modify
 #  it under the terms of the GNU General Public License as published by
 #  the Free Software Foundation; either version 2 of the License, or
 #  (at your option) any later version.
 #
 #  This program is distributed in the hope that it will be useful,
 #  but WITHOUT ANY WARRANTY; without even the implied warranty of
 #  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 #  GNU Library General Public License for more details.
 #
 #  You should have received a copy of the GNU General Public License
 #  along with this program; if not, write to the Free Software
 #  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
 
import os,wx,misc,socket

from mod import mod


class modHandler():

    modlist = []
    current_mod = mod()
    
    def __init__(self):
        # imports the available mods and sets the current mod to default
        modlist = self.importModList()
        
        # this will probably change
        current_mod = modlist[0]
        
    def importModList(self):
        #not done yet
        pass
    

    
    
    
