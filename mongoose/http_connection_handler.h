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
#ifndef __d6539852f4aa2fa11847e6e2070c76fc__
#define __d6539852f4aa2fa11847e6e2070c76fc__

#include "../mongoose.h"

class HttpConnectionHandler
{
public:
	enum RequestStatus
	{
		Complete = 0,
		Deferred,
		UseStaticFile
	};

	enum PollStatus
	{
		KeepConnection = 0,
		CloseConnection
	};

	inline HttpConnectionHandler() {}
	virtual inline ~HttpConnectionHandler() {}

	// Server
	virtual bool authorized() { return true; }
	virtual RequestStatus handleRequest(mg_connection * conn) { (void)conn; return UseStaticFile; }
	virtual PollStatus poll(mg_connection * conn) { (void)conn; return KeepConnection; }

	// Client
	virtual PollStatus sendRequest(mg_connection * conn) { (void)conn; return CloseConnection; }
	virtual PollStatus handleResponse(mg_connection * conn) { (void)conn; return CloseConnection; }
};

#endif
