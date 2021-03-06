#include "StdAfx.h"
#include "Rendering/GL/myGL.h"
#include <SDL_timer.h>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/ptr_container/ptr_deque.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/shared_ptr.hpp>
#include <deque>

#include "mmgr.h"

#include "NetProtocol.h"

#include "Game/GameSetup.h"
#include "LogOutput.h"
#include "DemoRecorder.h"
#include "Platform/ConfigHandler.h"
#include "Net/UDPConnection.h"
#include "Net/LocalConnection.h"
#include "Net/UDPSocket.h"


CNetProtocol::CNetProtocol()
{
	localDemoPlayback = false;
}

CNetProtocol::~CNetProtocol()
{
	Send(CBaseNetProtocol::Get().SendQuit());
	logOutput.Print(server->Statistics());
}

void CNetProtocol::InitClient(const char *server_addr, unsigned portnum,unsigned sourceport, const unsigned wantedNumber)
{
	GML_STDMUTEX_LOCK(net);

	boost::shared_ptr<netcode::UDPSocket> sock(new netcode::UDPSocket(sourceport));
	sock->SetBlocking(false);
	netcode::UDPConnection* conn = new netcode::UDPConnection(sock, server_addr, portnum);
	conn->SetMTU(configHandler.GetInt("MaximumTransmissionUnit", 0));
	server.reset(conn);
	server->SendData(CBaseNetProtocol::Get().SendAttemptConnect(wantedNumber, NETWORK_VERSION));
	server->Flush(true);

	if (!gameSetup || !gameSetup->hostDemo)	//TODO do we really want this?
	{
		record.reset(new CDemoRecorder());
	}
	
	logOutput.Print("Connecting to %s:%i using number %i", server_addr, portnum, wantedNumber);
}

void CNetProtocol::InitLocalClient(const unsigned wantedNumber)
{
	GML_STDMUTEX_LOCK(net);

	server.reset(new netcode::CLocalConnection);
	server->Flush();
	if (!localDemoPlayback)
	{
		record.reset(new CDemoRecorder());
	}
	
	logOutput.Print("Connecting to local server using number %i", wantedNumber);
}

bool CNetProtocol::Active() const
{
	return !server->CheckTimeout();
}

bool CNetProtocol::Connected() const
{
	return (server->GetDataReceived() > 0);
}

boost::shared_ptr<const netcode::RawPacket> CNetProtocol::Peek(unsigned ahead) const
{
	return server->Peek(ahead);
}

boost::shared_ptr<const netcode::RawPacket> CNetProtocol::GetData()
{
	boost::shared_ptr<const netcode::RawPacket> ret = server->GetData();
	
	if (record && ret)
		record->SaveToDemo(ret->data, ret->length);
	
	return ret;
}

void CNetProtocol::Send(boost::shared_ptr<const netcode::RawPacket> pkt)
{
	GML_STDMUTEX_LOCK(net);

	server->SendData(pkt);
}

void CNetProtocol::Send(const netcode::RawPacket* pkt)
{
	boost::shared_ptr<const netcode::RawPacket> ptr(pkt);
	Send(ptr);
}

void CNetProtocol::UpdateLoop()
{
	loading = true;
	while (loading)
	{
		Update();
		SDL_Delay(400);
	}
}

void CNetProtocol::Update()
{
	GML_STDMUTEX_LOCK(net);

	server->Update();
}

CNetProtocol* net=0;
