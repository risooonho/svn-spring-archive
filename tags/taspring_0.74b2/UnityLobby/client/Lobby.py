#!/usr/bin/env python

#======================================================
 #            Lobby.py
 #
 #  Thurs September 7 11:20 2006
 #  Copyright  2006  Josh Mattila
 #  		          Declan Ireland
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

# Generic Modules
import socket
import thread
import Queue
import time

class Timer:

    
    def __init__(self, interval, function, *args, **kwargs):
    ## ==================================================================================================        
    ## Create Timer Object
    ## ==================================================================================================

        self.__lock = thread.allocate_lock()
        self.__interval = interval
        self.__function = function
        self.__args = args
        self.__kwargs = kwargs
        self.__loop = False
        self.__alive = False

    def start(self):
    ## ==================================================================================================        
    ## Start Timer Object
    ## ==================================================================================================

        self.__lock.acquire()
        if not self.__alive:
            self.__loop = True
            self.__alive = True
            thread.start_new_thread(self.__run, ())
        self.__lock.release()


    def stop(self):
    ## ==================================================================================================        
    ## Stop Timer Object
    ## ==================================================================================================

        self.__lock.acquire()
        self.__loop = False
        self.__lock.release()


    def __run(self):
    ## ==================================================================================================
    ## Private Thread Function
    ## ==================================================================================================    

        while self.__loop:
            self.__function(*self.__args, **self.__kwargs)
            time.sleep(self.__interval)
        self.__alive = False


class user:
# This class is the abstraction of the user "object".
# It stores data about users and keeps track of status,
# cpu, and what they are doing.

    name = ""
    country = ""
    cpu = 0
    ip = ""
    status = 0                  #default user status

    channelList = []            #the channels that the user belongs to
    pmList = []

    battle_id = None            #default battle id
    battle_host = False
    battle_status = 16778240    #default battle status

    status_image = 2

    rank_image = 0

    parent_window = None        #gui_lobby instance
    server = None               #server instance that owns this user


    def __init__(self, server, name, country, cpu, ip):
    ## ==================================================================================================        
    ## The function that creates a new user
    ## and initializes some variables
    ## ==================================================================================================

        self.server = server
        self.cpu = cpu
        self.name = name
        self.country = country
        self.ip = ip
        
        self.channelList = []

#---Channel / PM  Functions---#000000#FFFFFF------------------------------------
    def addChannel(self, channel):
    ## ==================================================================================================
    ## This function adds a channel to the channel list
    ## ==================================================================================================        

        if channel != "":
            self.channelList.append(channel)
            

    def leaveChannel(self, channel):
    ## ==================================================================================================
    ## This function removes a channel from channel list
    ## ==================================================================================================        

        if channel != "":
            if (channel in self.channelList) == True:
                index = self.channelList.index(channel)
                del self.channelList[index]
            

    def listChannels(self):
    ## ==================================================================================================        
    ## Returns list of known channels user is in
    ## ==================================================================================================

        return self.channelList	


    def addPM(self, pm):
    ## ==================================================================================================
    ## This function adds a pm to thepm list
    ## ==================================================================================================

        if pm != "":
            self.pmList.append(pm)
            

    def leavePM(self, pm):
    ## ==================================================================================================
    ## This function removes a pm from the pm list
    ## ==================================================================================================        

        if pm != "":
            if (pm in self.pmList) == True:
                index = self.pmList.index(pm)
                del self.pmList[index]
            

    def listPMs(self):
    ## ==================================================================================================
    ## Returns list of known PM's user is in
    ## ==================================================================================================        

        return self.pmList	

#---User Functions---#000000#FFFFFF---------------------------------------------
    def updateStatus(self, status):
    ## ==================================================================================================
    ## This function updates status
    ## ==================================================================================================

        self.status = int(status)


    def getClientStatus(self):
    ## ==================================================================================================
    ## Returns the client_status variable
    ## ==================================================================================================

        return self.status
            

    def isAway(self):
    ## ==================================================================================================
    ## Returns true if the user is "away"
    ##  and false if they are not
    ## ==================================================================================================

        status = self.status
        bitmask = 2
        
        status = bitmask & status
                
        #Bitwise AND
        #===================
        #2      = 000010
        #status = 010110
        
        #returns= 000010
                
        if status != 0:
            return True
        else:
            return False
            

    def isBot(self):
    ## ==================================================================================================
    ## Returns True if user is a "Bot"
    ##  and False if they are not.
    ## ==================================================================================================        

        status = self.status
        if (status & (1 << 6)) != 0:
            return True
        else:
            return False
            

    def inGame(self):
    ## ==================================================================================================
    ## Returns True if the user is "ingame"
    ##  and false if they are not
    ## ==================================================================================================

        status = self.status
        bitmask = 1
        
        status = bitmask & status
        if status != 0:
            return True
        else:
            return False
            

    def getRank(self):
    ## ==================================================================================================
    ## Returns the users rank and updates the self.rank_image variable
    ## ==================================================================================================
            
        status = self.status
        bitmask = 0x1C
        
        status = bitmask & status
        status = status >> 2
        
        if self.admin() == False:
            self.rank_image = status + 5
        else:
            self.rank_image = 10

        return str(status)
            

    def admin(self):
    ## ==================================================================================================
    ## Returns True if the user is an "admin"
    ##  and False if they are not
    ## ==================================================================================================
    
        status = self.status
        bitmask = 32
        
        status = bitmask & status
        if status != 0:
            return True
        else:
            return False

#---Battle Functions---#000000#FFFFFF-------------------------------------------
    def updateBattleID(self, battle_id):

        self.battle_id = battle_id


    def getBattleID(self):

        return self.battle_id


    def getBattleHost(self):

        return self.battle_host


    def updateBattleHost(self, host):

        self.battle_host = host


    def updateBattleStatus(self, status):
    ## ==================================================================================================
    ## This function updates the battle_status
    ## ==================================================================================================        

        self.battle_status = status
         

    def getBattleStatus(self):
    ## ==================================================================================================
    ## Returns the battle_status variable
    ## ==================================================================================================

        return self.battle_status
            

    def getReady(self):
    ## ==================================================================================================
    ## Returns True if the user is "ready"
    ## ==================================================================================================        

        status = self.battle_status
        bitmask = 2
        
        status = bitmask & status
        
        if status != 0:
            return True
        else:
            return False


    def getTeamNumber(self):
    ## ==================================================================================================
    ## Returns the team number of the user
    ## ==================================================================================================        

        status = self.battle_status
        bitmask = 60
        
        status = bitmask & status
        status = status >> 2
        
        return status


    def getAllyTeam(self):
    ## ==================================================================================================
    ## Returns the ally team number of the user
    ## ==================================================================================================

        status = self.battle_status
        bitmask = 960
        
        status = bitmask & status
        status = status >> 6
        
        return status
            

    def getSpectator(self):
    ## ==================================================================================================
    ## Returns True if the user is a "Spectator"
    ##  of False if they are not
    ## ==================================================================================================        

        status = self.battle_status
        bitmask = 1024
        
        status = bitmask & status
        
        
        if status != 0:
            return True
        else:
            return False
            

    def getHandicap(self):
    ## ==================================================================================================
    ## Returns handicap amount
    ## ==================================================================================================        

        status = self.battle_status
        bitmask = 130048
        
        status = bitmask & status
        status = status >> 10

        return status
            

    def getTeamColor(self):
    ## ==================================================================================================
    ## Returns the team color index
    ## b18..b21
    ## A number 0-9 that is defined in TAPallete.cpp
    ## ==================================================================================================

        # TODO: Is this code obselete ???

        status = self.battle_status
        bitmask = 1966080
        
        status = bitmask & status
        status = status >> 17
        
        
        return status
            

    def getSyncStatus(self):
    ## ==================================================================================================
    ## Returns the sync status of user
    ##  0 = unknown
    ##  1 = synced
    ##  2 = unsynced
    ## ==================================================================================================

        status = self.battle_status
        bitmask = 6291456
        
        status = bitmask & status
        status = status >> 21
                
        return status
            

    def getFaction(self):
    ## ==================================================================================================
    ## Returns the user side index
    ## ==================================================================================================

        status = self.battle_status
        bitmask = 125829120
        
        status = bitmask & status
        status = status >> 23
        
        return status



class client:

    client_connected = False
    client_ip = ""
    client_status = 0           # see MYSTATUS command for actual values of status
    client_battleStatus = 0     # see MYBATTLESTATUS command for actual values of battleStatus
    client_battleID = 0         # battle ID in which client is participating. Must be -1 if not participating in any battle.
    client_inGameTime = 0
    client_country = ""
    client_cpu = 0              # in MHz if possible, or in MHz*1.4 if AMD. 0 means the client cant figure out its CPU speed.

    server_timeout = 30         # In seconds
    server_pingrate = 20        # The rate at which pings are sent in seconds
    server_pings_sent = 0       # The number of pings sent since a PONG message has been received

    client_cpu = 0
    client_localIP = ""

    receivingFile = False       # true if we are sending some file to this client
    cancelTransfer = False      # we use it to signal the SendFileThread to stop sending file
    recvThread_running = False

    client_socket = None
    client_messageQueue = None
    client_sendQueue = None

    client_recvTimer = None
    client_pingTimer = None

    last_command = ""

    count_send = 0
    count = 0


    def __init__(self, parent):
    ## ==================================================================================================
    ## Initialize Variables
    ## ==================================================================================================
        
        self.client_messageQueue = Queue.Queue()
        self.client_sendQueue = Queue.Queue()

        self.client_pingTimer = Timer(20, self.ping)
        self.client_recvTimer = Timer(1, self.receive)
        
        self.client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.client_localIP =  socket.gethostbyaddr(socket.gethostname())[2][0]  # TODO Add Option to select different interfaces

        self.parent_window = parent


    def connect(self, server, port, username, password):
    ## ==================================================================================================
    ##  The function that connects the socket with the spring
    ##   master server.  Placing this here ensures that only
    ##   initialization goes in the __init__() function.
    ##   Also takes arguments for server, address, port, username, password.
    ## ==================================================================================================    
    
        self.server_address = server
        self.server_port = port
        self.client_username = username
        self.client_password = password
        
        self.client_recvTimer.start()
        try:
            self.client_socket.connect((self.server_address, self.server_port))
            self.client_pingTimer.start()
     
        except socket.error, msg:
            self.client_socket.close()
            print str(msg[1])
            self.parent_window.error(msg)
            return False
        
        # Try to recieve the first chunk of data from the server
        try:
            if self.isConnectionAlive():
                print("Server: " + self.client_socket.recv(15))
        except socket.error, msg:
            self.client_socket.close()
            print str(msg[1])
            return False
             
        self.client_connected = True
        # Client Connected
        self.receive_increment = 0
        return self.client_connected


    def reLogin(self):
    ## ==================================================================================================
    ##  The function that logs into the server
    ##  after some operation of being connected
    ##  has occured.    
    ## ==================================================================================================
        
        self.send("LOGIN", self.client_username + " " + self.client_password + " " + str(self.client_cpu) + " " + str(self.client_localIP) + " " + "UnityLobby\t0.009")

#---Net Recieve---#000000#FFFFFF------------------------------------------------
    def receiveData(self):
    ## ==================================================================================================        
    ## The message loop that recieves network messages from the server.
    ## ==================================================================================================    

        if self.isConnectionAlive():
            data = "d"
            new_data = ""
            while data != "":
                data = ""
                try:
                    self.client_socket.setblocking(0)
                    data = self.client_socket.recv(512)
                    new_data = new_data + data
                    self.client_socket.setblocking(1)
                except socket.error, msg:
                    if msg[0] != 11 and msg[0] != 10035:
                        print("Error occured when receiving data from the socket. " + str(msg))

            if new_data != "":
                self.parseCommand(new_data)
                    
            while 1:
                try: 
                    cmdToBeProcessed = self.client_messageQueue.get(False)
                    self.processCommand(cmdToBeProcessed)  
                except:
                    break   
            if self.receive_increment > 4:
                cache = None
                while 1:
                    try: 
                        cmdToSend = self.client_sendQueue.get(False)
                        
                        # TODO:
                        if cmdToSend[:19] == 'UPDATEBATTLEDETAILS':
                            cache = cmdToSend
                        # TODO:
                        
                        else:
                            self.sendData(cmdToSend)  
                    except:
                        break  
                if cache != None:
                    self.sendData(cache)
                    print cache
            self.receive_increment += 1

#---Ping Related---#000000#FFFFFF-----------------------------------------------
    def sendPing(self):
    ## ==================================================================================================
    ## This keeps connection alive, by sending ping commands to the lobby server
    ## ==================================================================================================

        if self.server_pings_sent >= (self.server_timeout / self.server_pingrate):
            print("Server has timed out")
            self.disconnect("force")
            
        if self.isConnectionAlive():
            self.send("PING")
            self.server_pings_sent += 1


    def onPongReceived(self):
    ## ==================================================================================================
    ## When a PONG command is received from the server, this function is called
    ## ==================================================================================================    
        self.server_pings_sent = 0

#---Control Functions---#000000#FFFFFF------------------------------------------
    def isConnectionAlive(self):
    ## ==================================================================================================
    ## Starts the networking loop in a seperate thread
    ## ==================================================================================================
    
        if self.client_connected == True:
            try:
                tmp = self.client_socket.getpeername()
            except socket.error, msg:
                if msg[0] == 107:
                    self.disconnect("force")
                    return False
                print str(msg)
                return False
                
            return True
        else:
            return False


    def disconnect(self, disconnect):
    ## ==================================================================================================
    ## Stops the thread from running forcefully
    ## ==================================================================================================
    
        self.client_connected = False
        self.client_pingTimer.stop()
        self.client_recvTimer.stop()
        self.client_socket.close()
        if disconnect != 'User':
            self.parent_window.UpdateConnectedButton(False)


    def send(self, command, args=""):
    ## ==================================================================================================
    ## Puts a command in the send queue where it is emptied at regular intervals
    ## ==================================================================================================
    
        if command != "" and args != "":
            self.client_sendQueue.put(command + " " + args + "\n")
        elif command != "" and args == "":
            self.client_sendQueue.put(command + "\n")


    def sendData(self, command):
    ## ==================================================================================================
    ## Sends a command to the server
    ## ==================================================================================================
    
        self.client_socket.setblocking(0)
        
        if not command == "":
        
            try:
                bytes_sent = self.client_socket.send(command)
                while bytes_sent < len(command):
                    bytes_sent = self.client_socket.send(command)
            except socket.error, msg:
                if msg[0] == 107:
                    self.disconnect("force")
                    return False
                print("Error sending data! " + str(msg))

        self.client_socket.setblocking(1)

#---Command Functions---#000000#FFFFFF------------------------------------------
    def parseCommand(self, command, retry=False):
    ## ==================================================================================================
    ## Parses commands recieved from the server
    ## ==================================================================================================
        
        increment = 0

        if retry == False:
            commands_parsed = command.split("\n")
        else:
            if len(command) == 1:
                commands_parsed = command
                tmp_str = self.last_command + commands_parsed
                cmd_parsed = tmp_str.split(" ")
                
                self.lookupCommand(cmd_parsed, True)
                return True
            else:
                print ("ERROR ERRROR: " + command)
                return False
        
        while len(commands_parsed) > increment:
        
            if commands_parsed[increment] == "":
                increment += 1
                #break out of the loop because this command is empty
                if not len(commands_parsed) > increment:
                    break
        
            cmd_parsed = commands_parsed[increment].split(" ")
            
            #if for some reason the split function returns an empty string
            if cmd_parsed == "":
                cmd_parsed = commands_parsed[increment]

            if commands_parsed[len(commands_parsed) - 1] != "" and increment == len(commands_parsed) - 1:
               
                if len(cmd_parsed) > 0:
                    sentence = cmd_parsed[0]
                    
                    for x in cmd_parsed[1:]:
                        sentence =  sentence + " " + x

                    self.last_command = sentence
                else:
                    sentence = ""
                self.last_command = sentence
                self.lookupCommand(cmd_parsed, True)
                return False

            if cmd_parsed[0] != "":
                self.lookupCommand(cmd_parsed)

            increment += 1


    def lookupCommand(self, command, retry=False):
    ## ==================================================================================================
    ## Passes data to the queue, and performs special
    ## analysis of certain commands that cannot fail
    ## to be processed.  (Such as ADDUSER)    
    ## ==================================================================================================
        
        if len(command) > 0:
            cmd_sentence = command[0]
            
            for x in command[1:]:
                cmd_sentence =  cmd_sentence + " " + x
        else:
            cmd_sentence = ""

        if command[0] == "TASServer":
            self.client_messageQueue.put(command)
        elif command[0] == "PONG":
            self.client_messageQueue.put(command)
        elif command[0] == "AGREEMENT":
            self.client_messageQueue.put(command)
        elif command[0] == "AGREEMENTEND":
            self.client_messageQueue.put(command)
        elif command[0] == "REGISTRATIONDENIED":
            self.client_messageQueue.put(command)
        elif command[0] == "REGISTRATIONACCEPTED":
            self.client_messageQueue.put(command)
        elif command[0] == "ACCEPTED":
            self.client_messageQueue.put(command)
        elif command[0] == "DENIED":
            self.client_messageQueue.put(command)
        elif command[0] == "MOTD":
            self.client_messageQueue.put(command)
        elif command[0] == "SERVERMSG":
            self.client_messageQueue.put(command)
        elif command[0] == "SERVERMSGBOX":
            self.client_messageQueue.put(command)
        elif command[0] == "CHANNELMESSAGE":
            self.client_messageQueue.put(command)
        elif command[0] == "ADDUSER":
            self.client_messageQueue.put(command)
        elif command[0] == "REMOVEUSER":
            self.client_messageQueue.put(command)
        elif command[0] == "JOIN":
            self.client_messageQueue.put(command)
        elif command[0] == "CHANNELS":
            #Support for old CHANNELS command
            self.client_messageQueue.put(command)
        elif command[0] == "CHANNEL":
            self.client_messageQueue.put(command)
        elif command[0] == "ENDOFCHANNELS":
            self.client_messageQueue.put(command)
        elif command[0] == "JOINFAILED":
            self.client_messageQueue.put(command)
        elif command[0] == "CHANNELTOPIC":
            self.client_messageQueue.put(command)
        elif command[0] == "CLIENTS":
            self.client_messageQueue.put(command)
        elif command[0] == "JOINED":
            self.client_messageQueue.put(command)
        elif command[0] == "LEFT":
            self.client_messageQueue.put(command)
        elif command[0] == "SAID":
            self.client_messageQueue.put(command)
        elif command[0] == "SAIDEX":
            self.client_messageQueue.put(command)
        elif command[0] == "SAYPRIVATE":
            self.client_messageQueue.put(command)
        elif command[0] == "SAIDPRIVATE":
            self.client_messageQueue.put(command)
        elif command[0] == "OPENBATTLE":
            self.client_messageQueue.put(command)
        elif command[0] == "BATTLEOPENED":
            self.client_messageQueue.put(command)
        elif command[0] == "BATTLECLOSED":
            self.client_messageQueue.put(command)
        elif command[0] == "JOINBATTLE":
            self.client_messageQueue.put(command)
        elif command[0] == "JOINEDBATTLE":
            self.client_messageQueue.put(command)
        elif command[0] == "LEFTBATTLE":
            self.client_messageQueue.put(command)
        elif command[0] == "JOINBATTLEFAILED":
            self.client_messageQueue.put(command)
        elif command[0] == "OPENBATTLEFAILED":
            self.client_messageQueue.put(command)
        elif command[0] == "UPDATEBATTLEINFO":
            self.client_messageQueue.put(command)
        elif command[0] == "UPDATEBATTLEDETAILS":
            self.client_messageQueue.put(command)
        elif command[0] == "SAIDBATTLE":
            self.client_messageQueue.put(command)
        elif command[0] == "SAIDBATTLEEX":
            self.client_messageQueue.put(command)
        elif command[0] == "CLIENTSTATUS":
            self.client_messageQueue.put(command)
        elif command[0] == "CLIENTBATTLESTATUS":
            self.client_messageQueue.put(command)
        elif command[0] == "REQUESTBATTLESTATUS":
            self.client_messageQueue.put(command)
        elif command[0] == "FORCEQUITBATTLE":
            self.client_messageQueue.put(command)
        elif command[0] == "DISABLEUNITS":
            self.client_messageQueue.put(command)
        elif command[0] == "ENABLEUNITS":
            self.client_messageQueue.put(command)
        elif command[0] == "ENABLEALLUNITS":
            self.client_messageQueue.put(command)
        elif command[0] == "RING":
            self.client_messageQueue.put(command)
        elif command[0] == "REDIRECT":
            self.client_messageQueue.put(command)
        elif command[0] == "BROADCAST":
            self.client_messageQueue.put(command)
        elif command[0] == "ADDBOT":
            self.client_messageQueue.put(command)
        elif command[0] == "REMOVEBOT":
            self.client_messageQueue.put(command)
        elif command[0] == "UPDATEBOT":
            self.client_messageQueue.put(command)
        elif command[0] == "ADDSTARTRECT":
            self.client_messageQueue.put(command)
        elif command[0] == "REMOVESTARTRECT":
            self.client_messageQueue.put(command)
        elif command[0] == "TASServer":
            pass
        elif command[0] == "LOGININFOEND":
            pass
        else:
        
            #Command doesn't exist or can't be parsed
            #(also if the command is being passed from process command
            #just to see if it works)
            
            if retry == True:
                print("Failed to parse command " + command[0] + " from the server.")
                return False
                
            print("part 2: " + cmd_sentence)
            
            if self.last_command != "" and self.last_command != cmd_sentence.rstrip():
                tmp_cmd = self.last_command + cmd_sentence + "\n"
                self.parseCommand(tmp_cmd, False)
                self.last_command = ""
                return True
            print("last_command: \"" + self.last_command + "\"")
            print("cmd_sentence.rstrip(): \"" + cmd_sentence.rstrip() + "\"")

#		if cmd_sentence != "":
#			print("[->] \"" + cmd_sentence + "\"")


    def processCommand(self, command):
    ## ==================================================================================================
    ## Sends each command to its own appropriate
    ## callback function
    ## ==================================================================================================
    
        
        if len(command) > 1:
            cmd_sentence = command[1]
            
            for x in command[2:]:
                cmd_sentence =  cmd_sentence + " " + x
        else:
            cmd_sentence = ""
            
        #==================================================
        # Login Stuff
        #==================================================
        if command[0] == "TASServer":
            if (command[1] != '0.31') and (command[1] != '0.32'):
                self.parent_window.tasserver(command[1:])
                self.disconnect(None)
        elif command[0] == "PONG":
            self.onPongReceived()
        elif command[0] == "CHANNELTOPIC":
            self.parent_window.channeltopic(command[1:])
        elif command[0] == "REGISTRATIONDENIED":
            text = ''
            if len(command) > 1:
                for i in range(1,len(command)):
                    text = text + str(command[i]) + ' '
                text = text[:-1]
            self.parent_window.warning('REGISTARTION', text)
            self.disconnect("server")
        elif command[0] == "REGISTRATIONACCEPTED":
            self.parent_window.info('REGISTRATION', 'Registration Accepted')
            self.parent_window.UpdateConnectedButton(False)
            self.disconnect("user")
        elif command[0] == "AGREEMENT":
            self.parent_window.agreement(command[1:])
        elif command[0] == "AGREEMENTEND":
            self.parent_window.agreementend()
        elif command[0] == "ACCEPTED":
            self.parent_window.UpdateConnectedButton(True)
        elif command[0] == "DENIED":
            self.parent_window.denied(command[1:])
            self.disconnect("server")
        elif command[0] == "MOTD":
            self.parent_window.motd(command[1:])
        elif command[0] == "SERVERMSG":
            self.parent_window.servermsg(command[1:])
        elif command[0] == "SERVERMSGBOX":
            self.parent_window.servermsgbox(command[1:])
        elif command[0] == "REDIRECT":
            pass
        elif command[0] == "BROADCAST":
            pass

		#==================================================
		# Lobby Stuff
        #==================================================
        elif command[0] == "CHANNELS":
            pass
        elif command[0] == "CHANNEL":
            pass
        elif command[0] == "ENDOFCHANNELS":
            pass
        elif command[0] == "JOIN":
            self.parent_window.join(command[1:])
        elif command[0] == "CLIENTS":
            self.parent_window.clients(command[1], command[2:])
        elif command[0] == "JOINED":
            self.parent_window.joined(command[1], command[2])
        elif command[0] == "JOINFAILED":
            self.parent_window.joinedfailed(command[1:])
        elif command[0] == "ADDUSER": 
            if len(command[1:]) >= 3:
                self.parent_window.adduser(command[1], command[2], command[3])
            else:
                print("Invalid ADDUSER (too few args," + len(command[1:]) + ") command, \"" + cmd_sentence + "\"")
        elif command[0] == "REMOVEUSER":
            self.parent_window.removeuser(command[1])
        elif command[0] == "LEFT":
            self.parent_window.left(command[1:])
        elif command[0] == "SAID":
            self.parent_window.said(command[1:])
        elif command[0] == "SAIDEX":
            self.parent_window.saidex(command[1:])
        elif command[0] == "CLIENTSTATUS":
            if len(command[1:]) == 2:
                self.parent_window.clientstatus(command[1], command[2])
        elif command[0] == "SAYPRIVATE":
            self.parent_window.sayprivate(command[1:])
        elif command[0] == "SAIDPRIVATE":
            self.parent_window.saidprivate(command[1:])
        elif command[0] == "RING":
            pass
        elif command[0] == "FORCELEAVECHANNEL":
            self.parent_window.forceleavechannel(command[1:])
        elif command[0] == "CHANNELMESSAGE":
            self.parent_window.channelmessage(command[1:])

        #==================================================
        #Battle Stuff
        #==================================================
        elif command[0] == "BATTLEOPENED":
            if len(command[1:]) >= 12:
                self.parent_window.battleopened(command[1:])
        elif command[0] == "JOINEDBATTLE":
            self.parent_window.joinedbattle(command[1:])
        elif command[0] == "LEFTBATTLE":
            self.parent_window.leftbattle(command[1:])
        elif command[0] == "BATTLECLOSED":
            self.parent_window.battleclosed(command[1])
        elif command[0] == "JOINBATTLE":
            self.parent_window.joinbattle(command[1:])
        elif command[0] == "JOINBATTLEFAILED":
            pass
        elif command[0] == "SAIDBATTLE":
            self.parent_window.saidbattle(command[1:])
        elif command[0] == "SAIDBATTLEEX":
            self.parent_window.saidbattleex(command[1:])
        elif command[0] == "OPENBATTLE":
            self.parent_window.openbattle(command[1], False)
        elif command[0] == "OPENBATTLEFAILED":
            self.parent_window.openbattlefailed(command[1:])
        elif command[0] == "UPDATEBATTLEINFO":
            self.parent_window.updatebattleinfo(command[1:])
        elif command[0] == "UPDATEBATTLEDETAILS":
            self.parent_window.updatebattledetails(command[1:], False)
        elif command[0] == "CLIENTBATTLESTATUS":
            pass
        elif command[0] == "REQUESTBATTLESTATUS":
            pass
        elif command[0] == "FORCEQUITBATTLE":
            pass
        elif command[0] == "ADDSTARTRECT":
            pass
        elif command[0] == "REMOVESTARTRECT":
            pass
            
        #==================================================
        #Bots and Units
        #==================================================
        elif command[0] == "DISABLEUNITS":
            pass
        elif command[0] == "ENABLEUNITS":
            pass
        elif command[0] == "ENABLEALLUNITS":
            pass
        elif command[0] == "ADDBOT":
            pass
        elif command[0] == "REMOVEBOT":
            pass
        elif command[0] == "UPDATEBOT":
            pass


    def Register(self):
        
        self.send("REGISTER", self.client_username + " " + self.client_password)


    def ping(self):
    ## ==================================================================================================
    ## Sub Module for use with Timer Class
    ## ==================================================================================================
    
        self.sendPing()

    def receive(self):
    ## ==================================================================================================
    ## Sub Module for use with Timer Class
    ## ==================================================================================================

        self.receiveData()
