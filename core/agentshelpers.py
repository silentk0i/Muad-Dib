from core.common import *
from core.agent import Agent

from collections import OrderedDict
from shutil import rmtree
from base64 import b64decode

agents = OrderedDict()

def checkAgentsEmpty(s):
    
    uagents()

    global agents

    if len(agents) == 0:
        
        if s == 1:
            error("There are no active agents.")
            return True
        else:
            return True
    
    else:
        return False

def isValidAgent(name, s):

    uagents()
    vAgents = []
    for agent in agents:
        vAgents.append(agents[agent].name)

    if name in vAgents:
        return True
    else:
        if s == 1:
            error("Invalid agent.")
            return False
        else:
            return False

def viewAgents():
    if not checkAgentsEmpty(1):
        success("Active Agents:")
        
        print(GREEN)
        headers = ["Name", "Listener", "External IP", "Hostname"]
        column_widths = [30, 35, 40, 30]
        separator_widths = [10, 15, 20, 10]
        
        header_line = " ".join(f"{header:^{width}}" for header, width in zip(headers, column_widths))
        print(header_line)
        
        separator = " ".join(f"{'-' * sep_width:^{width}}" for sep_width, width in zip(separator_widths, column_widths))
        print(separator)
        
        for count, agent_id in enumerate(agents, 1):
            agent = agents[agent_id]
            agent_info = [
                f"{count}. {agent.name}",
                agent.listener,
                agent.remoteip,
                agent.hostname
            ]
            agent_line = " ".join(f"{info:^{width}}" for info, width in zip(agent_info, column_widths))
            print(agent_line)
        
        print(cRESET)

def renameAgent(args):

    if len(args) != 2:
        error("Invalid arguments.")
    else:
        
        name    = args[0]
        newname = args[1]

        if isValidAgent(name, 1) == True:
            
            if isValidAgent(newname, 0) == True:
                error("Agent {} already exists.".format(newname))
                return 0
            
            agents[name].rename(newname)
            
            if os.path.exists(agents[name].Path):
                rmtree(agents[name].Path)

            removeFromDatabase(agentsDB, name)
            agents[name].name = newname
            agents[name].update()
            writeToDatabase(agentsDB, agents[name])
            
            uagents()

        else:
            return 0

def removeAgent(args):
    
    if len(args) != 1:
        error("Invalid arguments.")
    else:
        name = args[0]
        if isValidAgent(name, 1):
            taskAgentToQuit(name)
            rmtree(agents[name].Path)
            removeFromDatabase(agentsDB,name)
            uagents()
        else:
            pass

def getAgentsForListener(name):
    
    result = []

    for agent in agents:
        if agents[agent].listener == name:
            result.append(agents[agent].name)

    return result

def interactWithAgent(args):
    if len(args) != 1:
        error("Invalid arguments.")
    else:
        try:
            # Convert argument to integer and adjust for zero-based indexing
            num = int(args[0]) - 1
            
            # Convert OrderedDict to a list of items to access by index
            agent_items = list(agents.items())
            
            # Validate the index
            if num < 0 or num >= len(agent_items):
                error("Index out of range.")
                return
            
            # Get the agent by index
            agent_key, agent_object = agent_items[num]
            
            # Validate the agent
            if isValidAgent(agent_object.name, 1):
                agent_object.interact()
            else:
                error("Invalid agent.")
                
        except ValueError:
            error("Invalid input; please enter a numeric index.")
        except Exception as e:
            error(f"An error occurred: {e}")

def clearAgentTasks(name):
    if isValidAgent(name, 0):
        agents[name].clearTasks()
    else:
        pass

def displayResults(name, result):

    if isValidAgent(name,0) == True:

        if result == "":
            success("Agent {} completed task.".format(name))
        else:
            success("Agent {} returned results:".format(name))
            success(result)
                
def taskAgentToQuit(name):
    agents[name].Quit()

def uagents():
    global agents
    
    try:        
        data = readFromDatabase(agentsDB)
        for agent_dict in data:
            for agent_key,value in agent_dict.items():
                agents[agent_key] = value
    except:
        pass