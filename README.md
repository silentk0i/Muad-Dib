
# Muad-Dib

A bare-bones command and control framework (C2) for providing the most minimal functionallity for loading another C2.

![Muad-Dib Banner](https://github.com/silentk0i/Muad-Dib/blob/main/assets/home.png)
## Installation

Clone the repository:

```bash
git clone https://github.com/silentk0i/Muad-Dib.git
```
Install all requirements:

```bash
cd Muad-Dib/
pip install -r requirements.txt
```

## Usage

```bash
┌──(silentk0i㉿kali)-[~/Tooling/Muad-Dib]
└─$ python3 muad-dib.py 

(home)> help

[+] Avaliable commands: 


 Command                         Description                         Arguments
---------                       -------------                       -----------
 help                            Show help.                          
 home                            Return home.                        
 exit                            Exit.                               
 listeners                       Manage listeners.                   
 agents                          Manage active agents.               


(home)> 
```

Code generation is still a work-in-progress, so you must compile the Atreides agent using the solution provided in ```~/payloads/Atreides/```. When you choose a listener change the ip & port variables with the values provided from the server and compile. They should be easy to find as I put a ```//REPLACE THIS``` comment in Atreides.cpp.
## Features

#### Server
- HTTPS communication
- Multiple Agent accessibility
- Persistent access to Agents/Listeners
- Extremely minimal & lightweight

#### Atreides Agent
- Remote shellcode injection via threadpools
- Syscall spoofing via VEH & HWBP's
- Enumeration of all running processes
## Detections

The Atreides agent currently yields 8/73 detections as of 09/09/2024. However, most main stream AV platforms (Crowdstrike, Microsoft, Sophos) were evaded. All except for SentinelOne.

![VirusTotal Detections](https://github.com/silentk0i/Muad-Dib/blob/main/assets/detections.png)

The Atreides agent was also tested against an Elastic EDR environment with Elastic Defender enableded and yieled no alerts.


## Additional Notes
- Although more evasion techniques could be added to the Agent like cough cough* API Hashing, I believe this should be left to the user and how they would like to personalize the Agent.
- This is also my first "real" public repository so if I messed something up message me at silentk0i@proton.me 
- This project was mainly built for my learning, but I hope others can learn from this repository as well.
- Also if you haven't seen Dune watch that and you'll understand.
## Acknowledgements

 - Please please please check out [0xRick's Blog](https://0xrick.github.io/) as a lot of the core design of this C2 stems from his blog on C2's.
 - Thanks to rad9800 for providing an incredibly detailed write up and code for [Tampering Syscalls](https://github.com/rad9800/TamperingSyscalls)
 - Finally, give thanks to Uri3n for the write up on different techniques for threadpool injection. You can find the write up [here](https://diagolima.com/html/posts/threadpools.html) and the code [here](https://github.com/Uri3n/Thread-Pool-Injection-PoC)


## License

This code is licensed under the [GPL-3.0](https://www.gnu.org/licenses/gpl-3.0.en.html) license.

## TODO

- [ ] Diferent payload formats
- [ ] Domain-Fronting Support
- [ ] More execution options (BOF loading, Reflective DLL injection)
- [ ] Sleep obfuscation
- [ ] Call Stack Spoofing
