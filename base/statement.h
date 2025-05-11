////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	  <3D Librairies developped during 2002-2021>
//	  Copyright (C) <2021>  <Laurent Cancé Francis, 10/08/1975>
//	  laurent.francis.cance@outlook.fr
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//	@file statement.h 
//	@author Laurent Cancé
////////////////////////////////////////////////////////////////////////
#ifndef _STATEMENT_H_
#define _STATEMENT_H_

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CState
{
public:
	int state;
	int prev;
	int active;

	CState();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CStatement
{
public:

	CState states[256];

	CStatement();
	~CStatement();

	void add(unsigned char s);
	void add(unsigned char s,int value);
	void set(unsigned char s,int value);
	int getState(unsigned char s);
	void save();
	void restore();

	inline int operator()(unsigned char s)
	{ 
		return getState(s); 
	}

	inline int * operator[](unsigned char s)
	{
        states[s].active=1;
        return &states[s].state;
	}
};

#endif
