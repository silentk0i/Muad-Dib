#!/usr/bin/python3 

import logging
import os

from core.menu import *

def main():

	if os.path.exists("./data/") == False:
		os.mkdir("./data/")

	if os.path.exists("./data/listeners/") == False:
		os.mkdir("./data/listeners/")

	if os.path.exists("./data/databases/") == False:
		os.mkdir("./data/databases/")

	if os.path.exists("./certs/") == False:
		os.mkdir("./certs/")
		os.system('openssl req -x509 -newkey rsa:4096 -keyout ./certs/key.pem -out ./certs/cert.pem -sha256 -days 3650 -nodes -subj "/C=NY/ST=NewYork/L=NewYorkCity/O=ABC/OU=COM/CN=ABC.com"')

	log = logging.getLogger('werkzeug')
	log.disabled = True

	loadListeners()
	uagents()
	
	home()

if __name__ == "__main__":
    main()