import socket

IPaddr = input("what is your IP?")

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect(('10.17.106.76', 80))

    while(True):
        print("VALID COMMANDS:\n"
              "LIST, QUERY #, ALLQUERY, SET #, SETALARM #, LOGSTART #, LOGSTOP #\n")
        cmd = input("enter a command:")
        if(cmd == "LIST"):
            list()
        elif(cmd.split(" ")[0] == "QUERY"):
            query(cmd.split(" ")[1])
        elif (cmd == "ALLQUERY"):
            allQuery()
        elif (cmd.split(" ")[0] == "SET"):
            set(cmd.split(" ")[1])
        elif (cmd.split(" ")[0] == "SETALARM"):
            setAlarm(cmd.split(" ")[1])
        elif (cmd.split(" ")[0] == "LOGSTART"):
            logStart(cmd.split(" ")[1])
        elif (cmd.split(" ")[0] == "LOGSTOP"):
            logStop(cmd.split(" ")[1])
        else:
            print("INVALID COMAND\n")

    s.send(b'GET /sensors HTTP/1.1\r\n\r\n')
    reply = s.recv(1024)
    print(reply)


def list():
    print("Listing")


def query(val):
    print("Query", val)


def allQuery():
    print("Query All")


def set(val):
    print("Setting", val)


def setAlarm(val):
    print("alarm", val)


def logStart(val):
    print("Started Log", val)


def logStop(val):
    print("Started Log", val)
