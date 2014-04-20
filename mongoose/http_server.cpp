//
// Copyright (c) 2014 Nikolay Zapolnov (zapolnov@gmail.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
#include "http_server.h"
#include <yip-imports/cxx-util/macros.h>
#include <new>
#include <stdexcept>
#include <cassert>
#include <iostream>

#define DEFAULT_LISTENING_PORT 8080

HttpServer::HttpServer()
	: m_Error(false)
{
	m_Server = mg_create_server(this, handler);
	if (UNLIKELY(!m_Server))
		throw std::bad_alloc();

	try
	{
		setListeningPort(DEFAULT_LISTENING_PORT);
	}
	catch (...)
	{
		mg_destroy_server(&m_Server);
		throw;
	}
}

HttpServer::~HttpServer()
{
	mg_destroy_server(&m_Server);
}

void HttpServer::setOption(const char * name, const char * value)
{
	const char * msg = mg_set_option(m_Server, name, value);
	if (UNLIKELY(!msg))
		throw std::runtime_error(msg);
}

void HttpServer::setOption(const char * name, const std::string & value)
{
	setOption(name, value.c_str());
}

void HttpServer::setListeningPort(int port)
{
	char buf[64];
	sprintf(buf, "%d", port);
	setOption("listening_port", buf);
}

void HttpServer::setDocumentRoot(const char * path)
{
	setOption("document_root", path);
}

void HttpServer::setDocumentRoot(const std::string & path)
{
	setOption("document_root", path);
}

void HttpServer::poll(int milliseconds)
{
	mg_poll_server(m_Server, milliseconds);
}

void HttpServer::connect(const char * host, int port, bool useSSL, HttpConnectionHandler * handler)
{
	mg_connection * conn = mg_connect(m_Server, (host ? host : ""), port, (useSSL ? MG_TRUE : MG_FALSE));
	if (UNLIKELY(!conn))
	{
		delete handler;
		throw std::bad_alloc();
	}

	assert(conn->connection_param == NULL);
	conn->connection_param = handler;
}

HttpConnectionHandler * HttpServer::newConnectionHandler()
{
	return new HttpConnectionHandler;
}

int HttpServer::handler(mg_connection * conn, mg_event event)
{
	HttpServer * self = reinterpret_cast<HttpServer *>(conn->server_param);

	try
	{
		if (!conn->connection_param)
			conn->connection_param = self->newConnectionHandler();

		HttpConnectionHandler * handler = reinterpret_cast<HttpConnectionHandler *>(conn->connection_param);
		switch (event)
		{
		case MG_AUTH:
			return (handler->authorized() ? MG_TRUE : MG_FALSE);

		case MG_REQUEST:
			switch (handler->handleRequest(conn))
			{
			case HttpConnectionHandler::Complete: return MG_TRUE;
			case HttpConnectionHandler::UseStaticFile: return MG_FALSE;
			case HttpConnectionHandler::Deferred: return MG_MORE;
			}
			throw std::runtime_error("invalid return value from HttpConnectionHandler::handleRequest().");

		case MG_POLL:
			switch (handler->poll(conn))
			{
			case HttpConnectionHandler::KeepConnection: return MG_FALSE;
			case HttpConnectionHandler::CloseConnection: return MG_TRUE;
			}
			throw std::runtime_error("invalid return value from HttpConnectionHandler::poll().");

		case MG_HTTP_ERROR:
			return MG_FALSE;

		case MG_CONNECT:
			switch (handler->sendRequest(conn))
			{
			case HttpConnectionHandler::KeepConnection: return MG_TRUE;
			case HttpConnectionHandler::CloseConnection: return MG_FALSE;
			}
			throw std::runtime_error("invalid return value from HttpConnectionHandler::sendRequest().");

		case MG_REPLY:
			switch (handler->handleResponse(conn))
			{
			case HttpConnectionHandler::KeepConnection: return MG_TRUE;
			case HttpConnectionHandler::CloseConnection: return MG_FALSE;
			}
			throw std::runtime_error("invalid return value from HttpConnectionHandler::handleResponse().");

		case MG_WS_HANDSHAKE:
			return MG_FALSE;

		case MG_CLOSE:
			delete handler;
			conn->connection_param = NULL;
			return MG_TRUE;
		}

		throw std::runtime_error("unknown event from mongoose.");
	}
	catch (const std::exception & e)
	{
		std::cerr << "C++ exception: " << e.what() << std::endl;
		self->m_Error = true;
		return (event == MG_REQUEST || event == MG_POLL ? MG_TRUE : MG_FALSE);
	}
	catch (...)
	{
		std::cerr << "C++ exception." << std::endl;
		self->m_Error = true;
		return (event == MG_REQUEST || event == MG_POLL ? MG_TRUE : MG_FALSE);
	}
}
