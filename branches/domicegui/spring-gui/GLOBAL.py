#!/usr/bin/env python

#======================================================
 #            GLOBAL.py
 #
 #  Sat July 26 11:20 2006
 #  Copyright  2006  Josh Mattila
 #  		     Declan Ireland
 #  Other authors may add their names above!
 #
 #  jm6.linux@gmail.com
 #  deco_ireland@yahoo.ie
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


#======================================================
 #	Setup Information for Spring-GUI
#======================================================

LINUX_INSTALL_TYPES = ['Basic', 'Advanced']
LINUX_INSTALL_TYPES_DEFAULT = 0

WINDOWS_INSTALL_TYPES = ['Windows XP']
WINDOWS_INSTALL_TYPES_DEFAULT = 0

SPRING_GUI_BACKGROUND = '/home/deco/spring-gui/svn/spring-gui/background.jpg'


# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
# User shouldnt normally dont need to edit below this line EVER!!!!
# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


# Spring Map extension  <<-- Dont change
MAP_EXTENSION = '.smf'

# Size of Map Preview
MIP_LEVEL = 2
PIXEL_SIZE = 256   # Need to change this to correct value when u change MIP_LEVEL. So spring-gui can properbly resize Image

# Upper Limit for Teams (Army)
MAX_TEAMS = 16
# Upper Limit for Allies
MAX_ALLIES = 16


#======================================================
 #	Battle Settings for Spring
#======================================================


# Game Modes
GAME_MODES =         ['Continue', 'Game Over']
GAME_MODES_DEFAULT = 0

# Starting Positions
STARTING_POSITIONS = ['Fixed','Random','Select in map']
STARTING_POSITIONS_DEFAULT = 0

#======================================================
 #	External Debug Settings for Spring
#======================================================


# Gdb Backtrace
GDB_BACKTRACE = ['Off', 'On']
GDB_BACKTRACE_DEFAULT = 0


#======================================================
 #	Option Settings for Spring
#======================================================


#
# Notes:
#	For combobox
#		User Text Fields Are stored in a array i.e
#			FSAA = ['Off', 'On']
#		Default Value starts count @ ZERO i.e
#		'Off' ->	FSAA_Default = 0
# 		'On'  ->	FSAA_Default = 1
#
#	Also for spinner values i.e
#		FSAALevel = [0,16,2,4]
#		0  = Lowest Value
#		16 = Highest Value
#		2  = Increase/Decrease when pressing button
#		4  = Increase/Decrease when using page up/down button


# Sound Volume
Sound_Volume = [0,100,5,10]
Sound_Volume_Default = 60


# Resolution
Resolution   = ['640x480', '800x600', '1024x768', '1280x1024', '1600x1200']
Resolution_X = ['640',     '800',     '1024',     '1280', '1600']
Resolution_Y = ['480',     '600',     '768',      '1024', '1200']
Resolution_Default = 2

# Fullscreen
Fullscreen = ['Off', 'On']
Fullscreen_Default = 1

# FSAA
FSAA = ['Off', 'On']
FSAA_Default = 1

# FSAA Level
FSAALevel = [0,16,2,4]
FSAALevel_Default = 2

# Shadows
Shadows = ['Off','On']
Shadows_Default = 0

# Ground Detail
Ground_Detail = [20,120,1,10]
Ground_Detail_Default = 60

# Ground Decals
Ground_Decals = [0,5,1,10]
Ground_Decals_Default = 1

# Grass Detail
Grass_Detail = [0,10,1,1]
Grass_Detail_Default = 3

# Max Particles
Max_Particles = [0,12000,10,100]
Max_Particles_Default = 4000

# Adv Sky
Adv_Sky = ['Off','On']
Adv_Sky_Default = 1

# Dynamic Sky
Dynamic_Sky = ['Off','On']
Dynamic_Sky_Default = 0

# Adv Unit Shading
Adv_Unit_Shading = ['Off','On']
Adv_Unit_Shading_Default = 0

# Reflective Water
Reflective_Water = ['Off','Pretty','Dynamic']
Reflective_Water_Default = 0

# Max Sounds
Max_Sounds = [0,16,1,2]
Max_Sounds_Default = 16

# Unit Lod Dist
Unit_Lod_Dist = [100,600,10,100]
Unit_Lod_Dist_Default = 200

# Verbose Level
Verbose_Level = [0,10,1,1]
Verbose_Level_Default = 0

# Nano Team Colour
Nano_Team_Colour = ['Off','On']
Nano_Team_Colour_Default = 0

# Catch AI Exceptions
Catch_AI_Exceptions = ['Off','On']
Catch_AI_Exceptions_Default = 1
