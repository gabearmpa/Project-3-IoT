import socket
import time
import threading
import smtplib
import os.path
import re

IP = input("What is the IP of the server? ")


def list():
    print("Sensors: 0 = proximity, 1 = temperature, 2 = humidity")
    print("Actuators: 0 LED, 1 = buzzer")

def query(val, text):
    cmds = ["proximity","temperature","humidity"]
    message = 'GET /sensors/' + str(cmds[val]) + '\r\n\r\n'
    s.send(message.encode())
    time.sleep(.1)
    if(text):
        print("The", str(cmds[val]), "value is ")
    rec = re.findall('\d+', s.recv(1024).decode())[3]
    return rec#.split('\n')[3]

def allQuery():
    s.send(b'GET /sensors\r\n\r\n')
    time.sleep(.1)
    print(s.recv(1024).decode().split('\n')[3])

def set(val):
    print("Setting", val)
    #maybe have one for onboard lED
    if(val == 0):
        temp = "led/"
        temp += input("LED commands: on, off, toggle, flash: ")
    elif(val == 1):
        temp = "buzz/"
        temp += input("buzzer commands: single, beeps, off: ")
    message = 'PUT /'+temp+'\r\n\r\n'
    s.send(message.encode())
    time.sleep(.1)
    print(s.recv(1024).decode().split('\n')[3])

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
        sensorValue = int(query(sensor, False))
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
    
    print("Starting alarm for when sensor " + str(sensor) + " has value  " + condition + " " + str(value))
    # This thread is a daemon thread so it will quit running when the other thread ends
    alarmThread = threading.Thread(target=runAlarm, args=[sensor, value, isGreater], daemon = True)
    alarmThread.start()

# Global array to keep track of what logs are running
runningLog = [False, False, False, False, False]

# Logging thread that logs every 15 seconds
def logging(sensor):
    global runningLog
    while(runningLog[sensor]):
        #print("Logging", sensor)
        if(not os.path.isfile("log.csv")):
            with open("log.csv", "a") as logFile:
                logFile.write("Sensor, Value\n")
        data = query(sensor, False)
        with open("log.csv", "a") as logFile:
            logFile.write(str(sensor) + ", " + str(data) + "\n")
        time.sleep(15)

# Start a logging thread
def logStart(sensor):
    if(not runningLog[sensor]):
        print("Started Log", sensor)
        runningLog[sensor] = True
        logThread = threading.Thread(target=logging, args=({sensor}), daemon = True)
        logThread.start()
    else:
        print("Already logging", sensor)

# Stop a logging thread
def logStop(sensor):
    print("Stopping Log", sensor)
    runningLog[sensor] = False
    
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((IP, 80))
    firstTime = True
    while(True):
        print("\nVALID COMMANDS:\n"
              "LIST, QUERY #, ALLQUERY, SET #, SETALARM #, LOGSTART #, LOGSTOP #\n")
        cmd = input("Enter a command: ").upper()
        try:
            if(cmd == "LIST"):
                list()
            elif(cmd.split(" ")[0] == "QUERY"):
                print(query(int(cmd.split(" ")[1]), True))
            elif (cmd == "ALLQUERY"):
                allQuery()
            elif (cmd.split(" ")[0] == "SET" and firstTime):
                password = input("First Time setting actuator, please enter password: ")
                if(password == "arduino"):
                    firstTime = False
                    set(int(cmd.split(" ")[1]))
                else:
                    print("Wrong password")
            elif (cmd.split(" ")[0] == "SET" and not firstTime):
                set(int(cmd.split(" ")[1]))
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
            