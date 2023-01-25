/*
	Various utility functions that are used in other files
*/

// TODO - clean up these initial functions

int initWinSock() {
	// initialize winsock - we only need to do this once, so might as well do it now
	WSADATA wsa; // TODO - make this global?

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Winsock initialization failed. Error Code : %d", WSAGetLastError());
		return 1;
	}
}

int connectTCP(addr, port) {
	//Create a socket
	Socket cmdSock; // todo - create in a way that lets us pass it back out
	if ((cmdSock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create command socket : %d", WSAGetLastError());
		return 1;
	}

	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons(8080);

	//Connect to remote server
	if (connect(cmdSock, (struct sockaddr*)&server, sizeof(server)) < 0)
	{
		printf("Could not connect to command server");
		return 1;
	}
}