#include "stdafx.h"
#include "Server.h"

const std::string currentDateTime() {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	localtime_s(&tstruct, &now);
	strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

	return buf;
}

Server::Server()
{
	serverThread = new thread(&Server::ServerThread);
}

Server::~Server()
{
}

void Server::Join()
{
	serverThread->join();
}

void Server::ServerThread()
{
	io_service service;
	ip::tcp::endpoint ep(ip::tcp::v4(), 2001); // listen on 2001
	ip::tcp::acceptor acc(service, ep);

	try
	{

		while (true)
		{
			socket_ptr sock(new ip::tcp::socket(service));

			acc.accept(*sock);

			boost::asio::socket_base::keep_alive keepAlive(true);
			sock->set_option(keepAlive);

			cout << "[" << currentDateTime() << "] : Accept client " << endl;

			ClientSession(sock);
		}

	}
	catch (boost::system::system_error &e)
	{
		error_code ec = e.code();
		cout << "[" << currentDateTime() << "] : Exception caught in initialization " << ec.value() << " " << ec.category().name() << " "  << ec.message() << endl;
	}
}



void Server::ClientSession(socket_ptr sock)
{
	try
	{
		while (true)
		{
			char data[512];

			size_t len = sock->read_some(buffer(data));

			if (len > 0)
			{
				cout << "[" << currentDateTime() << "] : got request > " << data << endl;

				ParseCommand(sock, string(data));
			}
			else
			{
				write(*sock, buffer("\r\n"));
			}
		}
	}
	catch (boost::system::system_error &e)
	{
	    error_code ec = e.code();

		switch (ec.value())
		{
		case 2:
			cout << "[" << currentDateTime() << "] : Client closed connection" << endl;
			break;
		default:
			cout << "[" << currentDateTime() << "] : Exception: " << ec.message() << endl;
			break;
		}

	}
}

void Server::ParseCommand(socket_ptr sock, string command)
{
	vector<string> cmds = CommandCenter::Parse(command);

	if (cmds[0] == "echo")
	{
		CmdEcho(sock, cmds);
	}
	else if (cmds[0] == "time")
	{
		CmdTime(sock, cmds);
	}
	else
	{
		write(*sock, buffer("Unrecognized command"));
	}
}

void Server::CmdEcho(socket_ptr sock, vector<string> argv)
{
	string result = "";
	char outputBuffer[512];

	for (uint32_t i = 1; i < argv.size(); i++)
	{
		result += argv[i] + " ";
	}

	strcpy_s(outputBuffer, result.c_str());
	write(*sock, buffer(outputBuffer));
}

void Server::CmdTime(socket_ptr sock, vector<string> cmds)
{
	char outputBuffer[512];

	strcpy_s(outputBuffer, currentDateTime().c_str());

	write(*sock, buffer(outputBuffer));
}