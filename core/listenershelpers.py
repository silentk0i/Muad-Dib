from core.common import *
from core.agentshelpers import getAgentsForListener, removeAgent, taskAgentToQuit
from core.listenerHTTPS import ListenerHTTPS

from collections import OrderedDict
from shutil import rmtree

import os
import netifaces

listeners = OrderedDict()

def checkListenersEmpty(s):
    
    if len(listeners) == 0:
        
        if s == 1:
            error("There are no active listeners.")
            return True
        else:
            return True
    
    else:
        return False

def isValidListener(name, s):

    vListeners = ulisteners()

    if name in vListeners:
        return True
    else:
        if s == 1:
            error("Invalid listener.")
            return False
        else:
            return False

def viewListeners():
    if not checkListenersEmpty(1):
        success("Active Listeners:")
        
        print(GREEN)
        headers = ["Name", "IP:Port", "URL", "Status"]
        column_widths = [30, 35, 40, 30]
        separator_widths = [10, 15, 20, 10]
        
        header_line = " ".join(f"{header:^{width}}" for header, width in zip(headers, column_widths))
        print(header_line)
        
        separator = " ".join(f"{'-' * sep_width:^{width}}" for sep_width, width in zip(separator_widths, column_widths))
        print(separator)
        
        for count, (listener_name, listener_obj) in enumerate(listeners.items(), 1):
            status = "Running" if listener_obj.isRunning else "Stopped"
            listener_info = [
                f"{count}. {listener_name}",
                f"{listener_obj.ipaddress}:{listener_obj.port}",
                f"https://{listener_obj.ipaddress}:{listener_obj.port}/",
                status
            ]
            listener_line = " ".join(f"{info:^{width}}" for info, width in zip(listener_info, column_widths))
            print(listener_line)
        
        print(cRESET)

def ulisteners():
    
    l = []
    
    for listener in listeners:
        l.append(listeners[listener].name)
    
    return l

def startListener(args):

    if len(args) == 1:
        name = args[0]
        if listeners[name].isRunning == False:
            try:
                listeners[name].start()
                success("Started listener {}.".format(name))
            except:
                error("Invalid listener.")
        else:
            error("Listener {} is already running.".format(name))
    else:
        if len(args) != 3:
            error("Invalid arguments.")
        else:
            name = args[0]

            try:
                port = int(args[1])
            except:
                error("Invalid port.")
                return 0
            
            iface = args[2]

            try:
                netifaces.ifaddresses(iface)
                ipaddress = netifaces.ifaddresses(iface)[netifaces.AF_INET][0]['addr']
            except:
                error("Invalid interface.")
                return 0

            if isValidListener(name, 0):
                error("Listener {} already exists.".format(name))
            else:
            
                listeners[name] = ListenerHTTPS(name, port, ipaddress)
                progress("Starting listener {} on {}:{}.".format(name, ipaddress, str(port)))

                try:
                    listeners[name].start()
                    success("Listener started.")
                except:
                    error("Failed. Check your options.")
                    del listeners[name]

def stopListener(args):

    if len(args) != 1:
        error("Invalid arguments.")
    else:
        
        name = args[0]
        
        if isValidListener(name, 1):
            
            if listeners[name].isRunning == True:
                progress("Stopping listener {}".format(name))
                listeners[name].stop()
                success("Stopped.")
            else:
                error("Listener {} is already stopped.".format(name))
        else:
            pass

def removeListener(args):
    
    if len(args) != 1:
        error("Invalid arguments.")
    else:
        
        name = args[0]
        
        if isValidListener(name,1):
            
            listenerAgents = getAgentsForListener(name)

            for agent in listenerAgents:
                removeAgent([agent])

            rmtree(listeners[name].Path)
            
            if listeners[name].isRunning == True:
                stopListener([name])
                del listeners[name]
            else:
                del listeners[name]

        else:
            pass

def saveListeners():

    if len(listeners) == 0:
        clearDatabase(listenersDB)
    else:
        data = OrderedDict()
        clearDatabase(listenersDB)
        
        for listener in listeners:
        
            if listeners[listener].isRunning == True:
                
                name       = listeners[listener].name
                port       = str(listeners[listener].port)
                ipaddress  = listeners[listener].ipaddress
                flag       = "1"
                data[name] = name + " " + port + " " + ipaddress + " " + flag
                
                listeners[listener].stop()
            else:
                name       = listeners[listener].name
                port       = str(listeners[listener].port)
                ipaddress  = listeners[listener].ipaddress
                flag       = "0"
                data[name] = name + " " + port + " " + ipaddress + " " + flag
    
        writeToDatabase(listenersDB, data)

def loadListeners():
    
    if os.path.exists(listenersDB):
        
        data = readFromDatabase(listenersDB)
        temp = data[0]

        for listener in temp:
            
            listener = temp[listener].split()

            name      = listener[0]
            port      = int(listener[1])
            ipaddress = listener[2]
            flag      = listener[3]

            listeners[name] = ListenerHTTPS(name, port, ipaddress)

            if flag == "1":
                listeners[name].start()

    else:
        pass