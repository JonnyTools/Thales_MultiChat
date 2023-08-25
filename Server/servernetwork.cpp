#include "servernetwork.hpp"
/*
*   Instanciate Server
*   Start Listening 
*/
ServerNetwork::ServerNetwork(unsigned short port) : listen_port(port)
{
    logl("Chat Server Started");

    if (listener.listen(listen_port) != sf::Socket::Done)
    {
        logl("Could not listen");
    }
}
/*
*   Setup Connection for new Client One Every Cycle  
*   Store them in SocketObj Vector 
*/
void ServerNetwork::ConnectClients(std::vector<sf::TcpSocket*>* client_array)
{
    while (true)
    {   /*
        *   Allokate Space for TcpSocket Object
        */
        sf::TcpSocket* new_client = new sf::TcpSocket();
        if (listener.accept(*new_client) == sf::Socket::Done)
        {
            new_client->setBlocking(false);
            client_array->push_back(new_client);
            logl("Added client " << new_client->getRemoteAddress() << ":" << new_client->getRemotePort() << " on slot " << client_array->size());
        }
        else
        {
            /*
            * Free Space 
            */
            logl("Server listener error, restart the server");
            delete (new_client);
            break;
        }
    }
}

/*
    Close Sockets of disconnecting Client
    Free up Space in Client List and Socket Objekt
*/
void ServerNetwork::DisconnectClient(sf::TcpSocket* socket_pointer, size_t position)
{
    logl("Client " << socket_pointer->getRemoteAddress() << ":" << socket_pointer->getRemotePort() << " disconnected, removing");
    socket_pointer->disconnect();
    delete (socket_pointer);
    client_array.erase(client_array.begin() + position);
}

/*
    Check Clientlist Send to All but the sender
    Check if Messages are send correctly
*/
void ServerNetwork::BroadcastPacket(sf::Packet& packet, sf::IpAddress exclude_address, unsigned short port)
{
    for (size_t iterator = 0; iterator < client_array.size(); iterator++)
    {
        sf::TcpSocket* client = client_array[iterator];
        if (client->getRemoteAddress() != exclude_address || client->getRemotePort() != port)
        {
            if (client->send(packet) != sf::Socket::Done)
            {
                logl("Could not send packet on broadcast");
            }
        }
    }
}
/*
*   Check if Client Dissconnects 
*   If Message is here Broadcast Message and Display it
*/
void ServerNetwork::ReceivePacket(sf::TcpSocket* client, size_t iterator)
{
    sf::Packet packet;
    if (client->receive(packet) == sf::Socket::Disconnected)
    {
        DisconnectClient(client, iterator);
        return;
    }

    if (packet.getDataSize() > 0)
    {
        std::string received_message;
        packet >> received_message;
        packet.clear();

        packet << received_message << client->getRemoteAddress().toString() << client->getRemotePort();

        BroadcastPacket(packet, client->getRemoteAddress(), client->getRemotePort());
        logl(client->getRemoteAddress().toString() << ":" << client->getRemotePort() << " '" << received_message << "'");
    }
}
/*
*   Polling to Check if Messages Arrived
*/
void ServerNetwork::ManagePackets()
{
    while (true)
    {
        for (size_t iterator = 0; iterator < client_array.size(); iterator++)
        {
            ReceivePacket(client_array[iterator], iterator);
        }

        std::this_thread::sleep_for((std::chrono::milliseconds)250);
    }
}

void ServerNetwork::Run()
{
    /**
    *   Construct Threads to Handel ConnectClientsClients(this, client_array)
    *   Endless Loop
    **/

    std::thread connetion_thread(&ServerNetwork::ConnectClients, this, &client_array);
    /*
    *  
    *   Endless Loop
    */
    ManagePackets();
}