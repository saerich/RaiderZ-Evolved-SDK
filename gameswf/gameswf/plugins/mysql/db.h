// mydb.cpp	-- Vitaly Alexeev <tishka92@yahoo.com>	2007

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// MYSQL plugin implementation for the gameswf SWF player library.

#ifndef GAMESWF_MYDB_H
#define GAMESWF_MYDB_H

#include <mysql/mysql.h>
#include "table.h"

using namespace gameswf;

namespace mysql_plugin
{

	struct mydb : public as_object
	{

		// Unique id of a gameswf resource
		enum { m_class_id = AS_PLUGIN_MYDB };
		virtual bool is(int class_id) const
		{
			if (m_class_id == class_id) return true;
			else return as_object::is(class_id);
		}

		exported_module mydb(player* player);
		exported_module ~mydb();

		exported_module bool connect(const char* host, const char* dbname, const char* user,
			const char* pwd, const char* socket);
		exported_module void disconnect();
		exported_module mytable* open(const char* sql);
		exported_module int run(const char *sql);
		exported_module void set_autocommit(bool autocommit);
		exported_module void commit();

	private:

		bool runsql(const char* sql);
		MYSQL* m_db;
	};

}

#endif // GAMESWF_MYDB_H
