/* 
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _NEPOMUK_SERVICE_DBUS_INTERFACE_H_
#define _NEPOMUK_SERVICE_DBUS_INTERFACE_H_

#include <QDBusInterface>

namespace Nepomuk {
    namespace Middleware {

	class Message;
	class Result;

	namespace DBus {
	    /**
	     * @internal proxy interface for the service
	     */
	    class ServiceBackend : public QDBusInterface
		{
		    Q_OBJECT

		public:
		    ServiceBackend( const QString& url, const QString& type, QObject* parent = 0 );
		    ~ServiceBackend();

		    bool isAvailable() const;

		    Result methodCall( const Message& message );
		};
	}
    }
}

#endif
