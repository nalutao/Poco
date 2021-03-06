//
// Binder.cpp
//
// $Id: //poco/1.4/Data/ODBC/src/Binder.cpp#1 $
//
// Library: Data/ODBC
// Package: ODBC
// Module:  Binder
//
// Copyright (c) 2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#include "Poco/Data/ODBC/Binder.h"
#include "Poco/Data/ODBC/Utility.h"
#include "Poco/Data/BLOB.h"
#include "Poco/Data/ODBC/ODBCException.h"
#include "Poco/Exception.h"
#include <sql.h>


namespace Poco {
namespace Data {
namespace ODBC {


Binder::Binder(const StatementHandle& rStmt, Binder::ParameterBinding dataBinding):
	_rStmt(rStmt),
	_paramBinding(dataBinding)
{
}


Binder::~Binder()
{
	std::vector<SQLLEN*>::iterator it = _lengthIndicator.begin();
	std::vector<SQLLEN*>::iterator itEnd = _lengthIndicator.end();
	for(; it != itEnd; ++it) delete *it;
}


void Binder::bind(std::size_t pos, const std::string& val)
{
	SQLINTEGER size = (SQLINTEGER) val.size();
	SQLLEN* pLenIn = new SQLLEN;
	*pLenIn = SQL_NTS;

	if (PB_AT_EXEC == _paramBinding)
		*pLenIn = SQL_LEN_DATA_AT_EXEC(size);

	_lengthIndicator.push_back(pLenIn);
	_dataSize.insert(SizeMap::value_type((SQLPOINTER) val.c_str(), size));

	if (Utility::isError(SQLBindParameter(_rStmt, 
		(SQLUSMALLINT) pos + 1, 
		SQL_PARAM_INPUT, 
		SQL_C_CHAR, 
		SQL_LONGVARCHAR, 
		(SQLUINTEGER) size,
		0,
		(SQLPOINTER) val.c_str(), 
		(SQLINTEGER) size, 
		_lengthIndicator.back())))
	{
		throw StatementException(_rStmt, 
				"SQLBindParameter()");
	}
}


void Binder::bind(std::size_t pos, const Poco::Data::BLOB& val)
{
		SQLINTEGER size = (SQLINTEGER) val.size();
		SQLLEN* pLenIn = new SQLLEN;
		*pLenIn  = size;

		if (PB_AT_EXEC == _paramBinding)
			*pLenIn  = SQL_LEN_DATA_AT_EXEC(size);

		_lengthIndicator.push_back(pLenIn);
		_dataSize.insert(SizeMap::value_type((SQLPOINTER) val.rawContent(), size));

		if (Utility::isError(SQLBindParameter(_rStmt, 
			(SQLUSMALLINT) pos + 1, 
			SQL_PARAM_INPUT, 
			SQL_C_BINARY, 
			SQL_LONGVARBINARY, 
			(SQLUINTEGER) size,
			0,
			(SQLPOINTER) val.rawContent(), 
			(SQLINTEGER) size, 
			_lengthIndicator.back())))
		{
			throw StatementException(_rStmt, 
				"SQLBindParameter()");
		}
}


void Binder::bind(std::size_t pos)
{
	_lengthIndicator.push_back(0);
	_dataSize.insert(SizeMap::value_type(static_cast<SQLPOINTER>(0), static_cast<SQLLEN>(0)));
		// NOTE: stupid casts required by VS2010.

	if (Utility::isError(SQLBindParameter(_rStmt, 
		(SQLUSMALLINT) pos + 1, 
		SQL_PARAM_INPUT, 
		SQL_C_SHORT, 
		SQL_TYPE_NULL, 
		0,
		0,
		(SQLPOINTER) 0, 
		0, 
		_lengthIndicator.back())))
	{
		throw StatementException(_rStmt, "SQLBindParameter()");
	}
}


std::size_t Binder::dataSize(SQLPOINTER pAddr) const
{
	SizeMap::const_iterator it = _dataSize.find(pAddr);
	if (it != _dataSize.end()) return it->second;
	
	throw NotFoundException("Requested data size not found.");
}


void Binder::bind(std::size_t pos, const char* const &pVal)
{
	//no-op
}


} } } // namespace Poco::Data::ODBC
