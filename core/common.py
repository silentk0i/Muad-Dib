from colorama import init as colorama_init
from colorama import Fore
from colorama import Style
import pickle
import os
from collections import OrderedDict

colorama_init()

RED     = Fore.LIGHTRED_EX
GREEN   = Fore.LIGHTGREEN_EX
YELLOW  = Fore.LIGHTYELLOW_EX
cRESET  = Style.RESET_ALL

listenersDB = "data/databases/listeners.db"
agentsDB    = "data/databases/agents.db"

def prompt(name):
    
    prompt = "\n" + YELLOW + "(" + name + ")" + RED + "> " + cRESET
    return prompt

def error(message):
    
    print("\n" + RED + "[!] " + message + cRESET)

def success(message):
    
    print("\n" + GREEN + "[+] " + message + "\n" + cRESET)

def progress(message):
    
    print("\n" + YELLOW + "[*] " + message + "\n" + cRESET)

def readFromDatabase(database):
    
    data = []

    with open(database, 'rb') as d:
        
        while True:
            try:
                data.append(pickle.load(d))
            except EOFError:
                break
    
    return data

def writeToDatabase(database,newData):
    
    with open(database, "ab") as d:
        pickle.dump(newData, d, pickle.HIGHEST_PROTOCOL)

def removeFromDatabase(database,name):
    from core.agentshelpers import agents
    data = readFromDatabase(database)
    final = OrderedDict()
    for agent in data:
        for agent_key, agent_object in agent.items():
            final[agent_object.name] = agent_object
    
    del final[name]
    del agents[name]
    
    with open(database, "wb") as d:
        for i in final:
            pickle.dump(final[i], d , pickle.HIGHEST_PROTOCOL)
    
def clearDatabase(database):

    if os.path.exists(database):
        os.remove(database)
    else:
        pass