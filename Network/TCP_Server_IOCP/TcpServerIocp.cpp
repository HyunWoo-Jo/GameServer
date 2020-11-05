#include "../TCP_Server/stdafx.h"
#include "../ImaysNet/ImaysNet.h"
#include <stdio.h>
#include <signal.h>
#include <string>
#include <thread>
#include <unordered_map>
#include <memory>
#include <vector>
#include <iostream>
#include <mutex>

using namespace std;

volatile bool stopWorking = false;

void ProcessSignalAction(int sig_number) {
	if (sig_number == SIGINT) {
		stopWorking = true;
	}
}
class RemoteClient {
public:
	shared_ptr<thread> thread;
	Socket tcpConnection;

	RemoteClient() {}
	RemoteClient(SocketType socketType) : tcpConnection(socketType) {}
};

unordered_map<RemoteClient*, shared_ptr<RemoteClient>> remoteClients;

void ProcessClientLeave(shared_ptr<RemoteClient> remoteClient) {
	remoteClient->tcpConnection.Close();
	remoteClients.erase(remoteClient.get());

	cout << "Client left. There are " << remoteClients.size() << " connections.\n";
}


int main() {
	signal(SIGINT, ProcessSignalAction);
	// 사용자가 ctl-c를 누르면 메인루프를 종료하게 만듭니다.
	signal(SIGINT, ProcessSignalAction);

	try
	{
		// IOCP를 써서 많은 수의 클라이언트를 받아 처리한다.

		// 많은 수의 클라가 tcp 5555로 연결 들어오고,
		// 그들은 hello world를 열심히 보낼 것이다.
		// 그것을 그냥 에코해주도록 하자.
		// 서버에서는 총 처리한 바이트수를 지속적으로 출력만 하자.

		// 성능보다는 소스 가독성을 우선으로 만들어져 있습니다. 
		// 가령 string 객체의 로컬변수 생성,삭제가 잦는 등의
		// 성능상 문제되는 부분은 알아서 개선하고 싶으시면 개선하십시오.



		// IOCP를 준비한다.
		Iocp iocp(1); // 본 예제는 스레드를 딱 하나만 쓴다. 따라서 여기도 1이 들어간다.

		// TCP 연결을 받는 소켓
		Socket listenSocket(SocketType::Tcp);
		listenSocket.Bind(Endpoint("0.0.0.0", 5555));

		listenSocket.Listen();

		// IOCP에 추가한다.
		iocp.Add(listenSocket, nullptr);

		// overlapped accept를 걸어둔다. 
		auto remoteClientCandidate = make_shared<RemoteClient>(SocketType::Tcp);

		string errorText;
		if (!listenSocket.AcceptOverlapped(remoteClientCandidate->tcpConnection , errorText)
			&& WSAGetLastError() != ERROR_IO_PENDING)
		{
			throw Exception("Overlapped AcceptEx failed.");
		}
		listenSocket.m_isReadOverlapped = true;

		cout << "서버가 시작되었습니다.\n";
		cout << "CTL-C키를 누르면 프로그램을 종료합니다.\n";

		// 리슨 소켓과 TCP 연결 소켓 모두에 대해서 I/O 가능(avail) 이벤트가 있을 때까지 기다린다. 
		// 그리고 나서 I/O 가능 소켓에 대해서 일을 한다.

		while (!stopWorking)
		{
			// I/O 완료 이벤트가 있을 때까지 기다립니다.
			IocpEvents readEvents;
			iocp.Wait(readEvents, 100);

			// 받은 이벤트 각각을 처리합니다.
			for (int i = 0; i < readEvents.m_eventCount; i++)
			{
				auto& readEvent = readEvents.m_events[i];
				if (readEvent.lpCompletionKey == 0) // 리슨소켓이면
				{
					listenSocket.m_isReadOverlapped = false;
					// 이미 accept은 완료되었다. 귀찮지만, Win32 AcceptEx 사용법에 따르는 마무리 작업을 하자. 
					if (remoteClientCandidate->tcpConnection.UpdateAcceptContext(listenSocket) != 0)
					{
						//리슨소켓을 닫았던지 하면 여기서 에러날거다. 그러면 리슨소켓 불능상태로 만들자.
						listenSocket.Close();
					}
					else // 잘 처리함
					{
						shared_ptr<RemoteClient> remoteClient = remoteClientCandidate;

						// 새 TCP 소켓도 IOCP에 추가한다.
						iocp.Add(remoteClient->tcpConnection, remoteClient.get());

						// overlapped 수신을 받을 수 있어야 하므로 여기서 I/O 수신 요청을 걸어둔다.
						if (remoteClient->tcpConnection.ReceiveOverlapped() != 0
							&& WSAGetLastError() != ERROR_IO_PENDING)
						{
							// 에러. 소켓을 정리하자. 그리고 그냥 버리자.
							remoteClient->tcpConnection.Close();
						}
						else
						{
							// I/O를 걸었다. 완료를 대기하는 중 상태로 바꾸자.
							remoteClient->tcpConnection.m_isReadOverlapped = true;

							// 새 클라이언트를 목록에 추가.
							remoteClients.insert({ remoteClient.get(), remoteClient });

							cout << "Client joined. There are " << remoteClients.size() << " connections.\n";
						}

						// 계속해서 소켓 받기를 해야 하므로 리슨소켓도 overlapped I/O를 걸자.
						remoteClientCandidate = make_shared<RemoteClient>(SocketType::Tcp);
						string errorText;
						if (!listenSocket.AcceptOverlapped(remoteClientCandidate->tcpConnection, errorText)
							&& WSAGetLastError() != ERROR_IO_PENDING)
						{
							// 에러나면 리슨소켓 불능 상태로 남기자. 
							listenSocket.Close();
						}
						else
						{
							// 리슨소켓은 연결이 들어옴을 기다리는 상태가 되었다.
							listenSocket.m_isReadOverlapped = true;
						}
					}
				}
				else  // TCP 연결 소켓이면
				{
					// 받은 데이터를 그대로 회신한다.
					shared_ptr<RemoteClient> remoteClient = remoteClients[(RemoteClient*)readEvent.lpCompletionKey];

					if (readEvent.dwNumberOfBytesTransferred <= 0)
					{
						int a = 0;
					}


					if (remoteClient)
					{
						// 이미 수신된 상태이다. 수신 완료된 것을 그냥 꺼내 쓰자.
						remoteClient->tcpConnection.m_isReadOverlapped = false;
						int ec = readEvent.dwNumberOfBytesTransferred;

						if (ec <= 0)
						{
							// 읽은 결과가 0 즉 TCP 연결이 끝났다...
							// 혹은 음수 즉 뭔가 에러가 난 상태이다...
							ProcessClientLeave(remoteClient);
						}
						else
						{
							// 이미 수신된 상태이다. 수신 완료된 것을 그냥 꺼내 쓰자.
							char* echoData = remoteClient->tcpConnection.m_receiveBuffer;
							int echoDataLength = ec;

							// 원칙대로라면 TCP 스트림 특성상 일부만 송신하고 리턴하는 경우도 고려해야 하나,
							// 지금은 독자의 이해가 우선이므로, 생략하도록 한다.
							// 원칙대로라면 여기서 overlapped 송신을 해야 하지만 
							// 독자의 이해를 위해서 그냥 블로킹 송신을 한다.
							remoteClient->tcpConnection.Send(echoData, echoDataLength);

							// 다시 수신을 받으려면 overlapped I/O를 걸어야 한다.
							if (remoteClient->tcpConnection.ReceiveOverlapped() != 0
								&& WSAGetLastError() != ERROR_IO_PENDING)
							{
								ProcessClientLeave(remoteClient);
							}
							else
							{
								// I/O를 걸었다. 완료를 대기하는 중 상태로 바꾸자.
								remoteClient->tcpConnection.m_isReadOverlapped = true;
							}
						}
					}
				}
			}
		}

		// 사용자가 CTL-C를 눌러서 루프를 나갔다. 모든 종료를 하자.
		// 그러나 overlapped I/O가 모두 완료되기 전에는 그냥 무작정 나가면 안된다. 운영체제가 백그라운드로 overlapped I/O가 진행중이기 때문이다.
		// 완료를 모두 체크하고 나가도록 하자.
		listenSocket.Close();
		for (auto i : remoteClients)
		{
			i.second->tcpConnection.Close();
		}

		cout << "서버를 종료하고 있습니다...\n";
		while (remoteClients.size() > 0 || listenSocket.m_isReadOverlapped)
		{
			// I/O completion이 없는 상태의 RemoteClient를 제거한다.
			for (auto i = remoteClients.begin(); i != remoteClients.end();)
			{
				if (!i->second->tcpConnection.m_isReadOverlapped)
				{
					i = remoteClients.erase(i);
				}
				else
					i++; // 좀 더 기다려보자.
			}

			// I/O completion이 발생하면 더 이상 Overlapped I/O를 걸지 말고 '이제 정리해도 돼...'를 플래깅한다.
			IocpEvents readEvents;
			iocp.Wait(readEvents, 100);

			// 받은 이벤트 각각을 처리합니다.
			for (int i = 0; i < readEvents.m_eventCount; i++)
			{
				auto& readEvent = readEvents.m_events[i];
				if (readEvent.lpCompletionKey == 0) // 리슨소켓이면
				{
					listenSocket.m_isReadOverlapped = false;
				}
				else
				{
					shared_ptr<RemoteClient> remoteClient = remoteClients[(RemoteClient*)readEvent.lpCompletionKey];
					if (remoteClient)
					{
						remoteClient->tcpConnection.m_isReadOverlapped = false;
					}
				}
			}
		}

		cout << "서버 끝.\n";
	}
	catch (Exception& e) {
		cout << "Error :" << e.what() << endl;
	}



	return 0;
}