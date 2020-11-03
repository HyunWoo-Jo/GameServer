#include <iostream>
#include "../../ImaysNet/ImaysNet.h"


using namespace std;


int main()
{
	try {
		Socket s(SocketType::Tcp);
		s.Bind(Endpoint::Any);
		cout << "Connecting to server... \n";
		s.Connect(Endpoint("172.30.1.58", 8760));

		cout << "Sending data...\n";
		const char* text = "hello";
		s.Send(text, strlen(text) + 1);

		std::this_thread::sleep_for(1s);

		cout << "Closing socket...\n";
		s.Close();
	}
	catch (Exception& e) {
		cout << "Exception! " << e.what() << endl;
	}
	return 0;
}