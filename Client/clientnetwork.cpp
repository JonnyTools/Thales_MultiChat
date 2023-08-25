#include "clientnetwork.hpp"

/*
    Init Client Network
*/
ClientNetwork::ClientNetwork()
{
    logl("Chat Client Started");
}

/*
    Try to connect to Server IP Adress and Port via Socket
*/
void ClientNetwork::Connect(const char* address, unsigned short port)
{
    if (socket.connect(address, port) != sf::Socket::Done)
    {
        logl("Could not connect to the server\n");
    }
    else
    {
        isConnected = true;
        logl("Connected to the server\n");
    }
}
/*
    Polling: Check if Socket has Recived Messages in last Packet
    Display it on Console

    Wait and try again
*/
void ClientNetwork::ReceivePackets(sf::TcpSocket* socket)
{
    while (true)
    {
        if (socket->receive(last_packet) == sf::Socket::Done)
        {
            std::string received_string;
            std::string sender_address;
            unsigned short sender_port;
            last_packet >> received_string >> sender_address >> sender_port;
            logl("From (" << sender_address << ":" << sender_port << "): " << received_string);
        }

        std::this_thread::sleep_for((std::chrono::milliseconds)250);
    }
}
/*
    If Socket is Ready and Consol Message is there send it via socket
*/
void ClientNetwork::SendPacket(sf::Packet& packet)
{
    if (packet.getDataSize() > 0 && socket.send(packet) != sf::Socket::Done)
    {
        logl("Could not send packet");
    }
}

/*
    Main Thread: 
    Create ReciverThread for Server Messages
    Check if Input is enterd in Console 
    Send Console Text to Server 
*/
void ClientNetwork::Run()
{
    std::thread reception_thred(&ClientNetwork::ReceivePackets, this, &socket);

    while (true)
    {
        if (isConnected)
        {
            std::string user_input;
            std::getline(std::cin, user_input);

            if (user_input.length() < 1)
                continue;

            sf::Packet reply_packet;
            reply_packet << user_input;

            SendPacket(reply_packet);
        }
    }
}