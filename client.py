import socket
import time
import threading
import smtplib

IPaddr = input("What is the IP of the server? ")


def list():
    print("Listing")


def query(val):
    print("Query", val)


def allQuery():
    print("Query All")


def set(val):
    print("Setting", val)


# Peter Code -----------------
gmail_user = 'comp342gccf19@gmail.com'
gmail_password = 'P@$$word1!'
destination = "lowrancepd1@gcc.edu"

def alarmEmail(sensor, sensorValue, value, isGreater):
    print("Sending alarm email\n")
    condition = "greater" if isGreater else "less"
    msg = "From: Home Alarm System\r\nTo: User\r\nSubject: Alarm\r\n\r\nThe sensor " + str(sensor) + " has value " + str(sensorValue) + " which is " + condition + " than the trigger value of " + str(value) + "."
    mailer = smtplib.SMTP_SSL('smtp.gmail.com', 465)
    mailer.ehlo()
    mailer.login(gmail_user, gmail_password)
    mailer.sendmail(gmail_user, destination, msg)
    mailer.close()    

def runAlarm(sensor, value, isGreater):
    # Run the loop until the program closes
    while(True):
        # TODO: get value of sensor
        sensorValue = 10
        if((isGreater and sensorValue > value) or (not isGreater and sensorValue < value)):
            print("Alarm!", sensor)
            alarmEmail(sensor, sensorValue, value, isGreater)
            return
        time.sleep(15)

def setAlarm(sensor):
    # Find what type of alarm the user wants
    condition = input("Alarm when > (greater than) or < (less than) a value: ")
    isGreater = False
    if(condition == ">"):
        isGreater = True
    elif(condition != "<"):
        print("Invalid condition, please enter < or >")
        return
    value = int(input("Enter the value: "))
    
    print("Starting alarm", sensor)
    # This thread is a daemon thread so it will quit running when the other thread ends
    alarmThread = threading.Thread(target=runAlarm, args=[sensor, value, isGreater], daemon = True)
    alarmThread.start()

# Global array to keep track of what logs are running
runningLog = [False, False, False, False, False]

# Logging thread that logs every 15 seconds
def logging(val):
    global runningLog
    while(runningLog[val]):
        print("Logging", val)
        # TODO: get data from server
        # TODO: append data to logging file
        time.sleep(15)

# Start a logging thread
def logStart(val):
    if(not runningLog[val]):
        print("Started Log", val)
        runningLog[val] = True
        logThread = threading.Thread(target=logging, args=({val}), daemon = True)
        logThread.start()
    else:
        print("Already logging", val)

# Stop a logging thread
def logStop(val):
    print("Stopping Log", val)
    runningLog[val] = False
# End Peter Code --------------------
    
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    #s.connect(('10.17.106.76', 80)) commented out for testing

    while(True):
        print("Sensors: 0 = proximity, 1 = humidity, 2 = temperature, 3 = touch, 4 = light")
        print("Actuators: 0 = on board LED, 1 = external LED, 2 = buzzer")
        print("VALID COMMANDS:\n"
              "LIST, QUERY #, ALLQUERY, SET #, SETALARM #, LOGSTART #, LOGSTOP #\n")
        cmd = input("Enter a command: ")
        try:
            if(cmd == "LIST"):
                list()
            elif(cmd.split(" ")[0] == "QUERY"):
                query(cmd.split(" ")[1])
            elif (cmd == "ALLQUERY"):
                allQuery()
            elif (cmd.split(" ")[0] == "SET"):
                set(cmd.split(" ")[1])
            elif (cmd.split(" ")[0] == "SETALARM"):
                setAlarm(int(cmd.split(" ")[1]))
            elif (cmd.split(" ")[0] == "LOGSTART"):
                logStart(int(cmd.split(" ")[1]))
            elif (cmd.split(" ")[0] == "LOGSTOP"):
                logStop(int(cmd.split(" ")[1]))
            else:
                print("INVALID COMMAND\n")
        except:
            print("INVALID COMMAND\n")

    s.send(b'GET /sensors HTTP/1.1\r\n\r\n')
    reply = s.recv(1024)
    print(reply)
