"""
    Various utility functions that are used in other modules
"""

import socket

"""
	Receives an incoming TCP connection and returns the associated socket
    NOTE: This is a blocking function, and will wait until a connection is received

    input:
        port - what port to listen on
    returns:
        socket - a client socket associated with the connection received
"""
def receiveTCP(port):
	print(f"Listening for connection on port {port} ...")

	# create an IPv4 TCP server socket and set it up to listen
	serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	serversocket.bind(('', port))
	serversocket.listen()

	# listen for incoming connections and accept them
	(clientsocket, address) = serversocket.accept()
	print("Connection established")

	# close the server socket and return the socket associated with the connection
	serversocket.close()
	return clientsocket