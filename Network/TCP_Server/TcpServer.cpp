#include <iostream>
#include "../ImaysNet/ImaysNet.h"
#include "stdafx.h"

using namespace std;

int main() {
	try {
		Socket listenSocket(SocketType::Tcp);
		listenSocket.Bind(Endpoint("0.0.0.0", 8760));
		listenSocket.Listen();
		cout << "Server start.\n";

		Socket tcpConnection;
		string ignore;
		listenSocket.Accept(tcpConnection, ignore);

		auto a = tcpConnection.GetPeerAddr().ToString();
		cout << "Socket From " << a << " is accepted. \n";
		while (true) {
			string receiveData;
			cout << "Receiving data...\n";
			int result = tcpConnection.Receive();

			if (result == 0) {
				cout << "Connection closed.\n";
				break;
			}
			else if (result < 0) {
				cout << "Connect lost " << GetLastErrorAsString() << endl;
			}
			cout << "Received: " << tcpConnection.m_receiveBuffer << endl;
		}
		tcpConnection.Close();

	}
	catch(Exception& e){
		cout << "Exception! " << e.what() << endl;
	}


	return 0;
}