import readline
from collections import OrderedDict
from os import system

from core.common import *

from core.listenershelpers import *
from core.agentshelpers import *

import os
import glob

firstRun = True;

class AutoComplete(object):
    
    def __init__(self, options, complete_paths=False):
        self.options = sorted(options)
        self.complete_paths = complete_paths  # New flag for file path autocompletion
        return

    def complete(self, text, state):
        response = None

        # If we're completing a file path, use the filesystem
        if self.complete_paths:
            if state == 0:
                if text:
                    # Expand relative paths and ~ to full paths
                    text = os.path.expanduser(text)
                    # If it's a directory, suggest contents; otherwise suggest matching files
                    self.matches = glob.glob(text + '*')
                else:
                    self.matches = glob.glob('*')

            try:
                response = self.matches[state]
            except IndexError:
                response = None
        else:
            # Default behavior for command completion
            if state == 0:
                if text:
                    self.matches = [s 
                                    for s in self.options
                                    if s and s.startswith(text)]
                else:
                    self.matches = self.options[:]

            try:
                response = self.matches[state]
            except IndexError:
                response = None

        return response


class Menu:

    def __init__(self, name):
        self.name = name
        
        self.commands = OrderedDict()
        self.Commands = []

        self.commands["help"] = ["Show help.", ""]
        self.commands["home"] = ["Return home.", ""]
        self.commands["exit"] = ["Exit.", ""]

    def registerCommand(self, command, description, args):

        self.commands[command] = [description, args]

    def showHelp(self):

        success("Avaliable commands: ")
        
        print(GREEN)
        print(" Command                         Description                                                 Arguments")
        print("---------                       -------------                                               -----------")

        for i in self.commands:
            print(" {}".format(i) + " " * (32 - len(i)) + "{}".format(self.commands[i][0]) + " " * (60 - len(self.commands[i][0])) + "{}".format(self.commands[i][1]))
        
        print(cRESET)
    
    def clearScreen(self):
        system("clear")

    def uCommands(self):
        for i in self.commands:
            self.Commands.append(i)

    def parse(self):
        readline.set_completer(AutoComplete(self.Commands).complete)
        readline.parse_and_bind('tab: complete')

        cmd = input(prompt(self.name))

        cmd = cmd.split()
        
        if len(cmd) == 0:
            return None, []

        command = cmd[0]
        args = []

        # Enable file path autocompletion after the first argument for 'loadshell'
        if command == "loadshell" and len(cmd) > 1:
            # Switch to file path autocompletion
            readline.set_completer(AutoComplete([], complete_paths=True).complete)
        else:
            readline.set_completer(AutoComplete(self.Commands).complete)

        for i in range(1, len(cmd)):
            args.append(cmd[i])

        return command, args






def evListeners(command, args):

    if command == "list":
        viewListeners()
    
    elif command == "start":
        startListener(args)

    elif command == "stop":
        stopListener(args)
    
    elif command == "remove":
        removeListener(args)

def evAgents(command, args):

    if command == "list":
        viewAgents()
    elif command == "remove":
        removeAgent(args)
    elif command == "rename":
        renameAgent(args)
    elif command == "interact":
        interactWithAgent(args)

def evHome(command, args):

    if command == "help":
        Hmenu.showHelp()
    elif command == "home":
        home()
    elif command == "listeners":
        listenersHelper()
    elif command == "agents":
        agentsHelper()
    elif command == "exit":
        Exit()


def listenersHelper():
    
    Lmenu.clearScreen()
    
    while True:
        
        try:
            command, args = Lmenu.parse()
        except:
            continue

        if command not in ListenersCommands:
            error("Invalid command.")
        elif command == "home":
            home()
        elif command == "help":
            Lmenu.showHelp()
        elif command == "exit":
            Exit()
        else:
            evListeners(command, args)

def agentsHelper():
    
    Amenu.clearScreen()

    while True:
        
        try:
            command, args = Amenu.parse()
        except:
            continue
            
        if command not in AgentsCommands:
            error("Invalid command.")
        elif command == "home":
            home()
        elif command == "help":
            Amenu.showHelp()
        elif command == "exit":
            Exit()
        else:
            evAgents(command, args)

def home():

    Hmenu.clearScreen()

    global firstRun
    banner = """
    __  ___                __            ____  _ __  
   /  |/  /_  ______ _____/ /           / __ \\(_) /_ 
  / /|_/ / / / / __ `/ __  /  ______   / / / / / __ \\
 / /  / / /_/ / /_/ / /_/ /  /_____/  / /_/ / / /_/ /
/_/  /_/\\__,_/\\__,_/\\__,_/           /_____/ /_.___/ 

            created by silentk0i               
         https://github.com/silentk0i         

       _  _       
      (.)(.)      
     /   () \\     
    / |     | \\   
   `'|_   _|'`'   
      | (_) |     
       '---'
"""
    if(firstRun == True):
        print(YELLOW + banner + cRESET)
        firstRun = False
    
    
    while True:
        
        try:
            command, args = Hmenu.parse()
        except:
            continue

        if command not in homeCommands:
            error("Invalid command.")
        else:
            evHome(command, args)

def Exit():
    saveListeners()
    exit()

Amenu = Menu("agents")
Lmenu = Menu("listeners")
Hmenu = Menu("home")

Amenu.registerCommand("list", "List active agents.", "")
Amenu.registerCommand("interact", "Interact with an agent.", "<name>")
Amenu.registerCommand("rename", "Rename agent.", "<agent> <new name>")
Amenu.registerCommand("remove", "Remove an agent.", "<name>")

Lmenu.registerCommand("list", "List active listeners.", "")
Lmenu.registerCommand("start", "Start a listener.", "<name> <port> <interface> | <name>")
Lmenu.registerCommand("stop", "Stop an active listener.","<name>")
Lmenu.registerCommand("remove", "Remove a listener.", "<name>")

Hmenu.registerCommand("listeners", "Manage listeners.", "")
Hmenu.registerCommand("agents", "Manage active agents.", "")

Amenu.uCommands()
Lmenu.uCommands()
Hmenu.uCommands()

AgentsCommands    = Amenu.Commands
ListenersCommands = Lmenu.Commands
homeCommands      = Hmenu.Commands