from ListenerHTTP import ListenerHTTP

def main():
    listener = ListenerHTTP("test", 5050, "0.0.0.0")
    listener.run()

if __name__ == "__main__":
    main()