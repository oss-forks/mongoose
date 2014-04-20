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
#ifndef __a2bbc82da4b0e13991ce40b37bc2d70b__
#define __a2bbc82da4b0e13991ce40b37bc2d70b__

#include "../mongoose.h"
#include "http_connection_handler.h"
#include <string>

class HttpServer
{
public:
	HttpServer();
	virtual ~HttpServer();

	void setOption(const char * name, const char * value);
	void setOption(const char * name, const std::string & value);

	void setListeningPort(int port);

	void setDocumentRoot(const char * path);
	void setDocumentRoot(const std::string & path);

	void connect(const char * host, int port, bool useSSL, HttpConnectionHandler * handler);

	void poll(int milliseconds);

protected:
	virtual HttpConnectionHandler * newConnectionHandler();

private:
	mg_server * m_Server;
	bool m_Error;

	static int handler(mg_connection * conn, mg_event event);

	HttpServer(const HttpServer &) = delete;
	HttpServer & operator=(const HttpServer &) = delete;
};

template <class HANDLER> class HttpServerEx : public HttpServer
{
public:
	inline HttpServerEx() {}
	HttpConnectionHandler * newConnectionHandler() { return new HANDLER; }
};

#endif
