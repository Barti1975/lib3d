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
//	@file edges.cpp
//	@date 2014-08-06
////////////////////////////////////////////////////////////////////////

#include "params.h"

#include <math.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "edges.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class : EdgeList
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EdgeList::EdgeList() {List=NULL;}
EdgeList::~EdgeList() {if (List) delete List; List=NULL;}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV EdgeList::Init(int nmax) { List=new Edge[nmax];nList=0;Max=nmax;}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int EDGESCONV EdgeList::WitchEdge(int a, int b)
{
	int AB=a+b;
	int n=0;
	while (n<nList)
	{	
		if (List[n].ab==AB)
		{
			if (((List[n].a==a)&&(List[n].b==b))||((List[n].a==b)&&(List[n].b==a))) return n;
			else n++;
		}
		else n++;
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int EDGESCONV EdgeList::WitchEdgeStrict(int a, int b)
{
	int n=0;
	while (n<nList)
	{	
		if ((List[n].a==a)&&(List[n].b==b)) return n;
		else n++;
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int EDGESCONV EdgeList::WitchEdge2(unsigned int base,int a, int b)
{	
	int n=base;
	int AB=a+b;
	while (n<nList)
	{	
		if (List[n].ab==AB)
		{
			if (((List[n].a==a)&&(List[n].b==b))||((List[n].a==b)&&(List[n].b==a))) return n;
			else n++;
		}
		else n++;
	}
	return -1;
}
	
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int EDGESCONV EdgeList::Next(int a)
{
	int n=0;
	while (n<nList)
	{	
		if (List[n].tag==0)
		{
			if (List[n].a==a) return n;
			else n++;
		}
		else n++;
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int EDGESCONV EdgeList::iAddEdge(int a,int b)
{		
	int ed=WitchEdge(a,b);
	if (ed==-1)
	{
		List[nList].ab=a+b;
		List[nList].a=a;
		List[nList].b=b;
		List[nList].tag=0;
		nList++;
		return (nList-1);
	}
	return -1-ed;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV EdgeList::AddEdge(int a,int b)
{		
	if (WitchEdge(a,b)==-1)
	{
		List[nList].ab=a+b;
		List[nList].a=a;
		List[nList].b=b;
		List[nList].tag=0;
		nList++;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int EDGESCONV EdgeList::AddEdgeR(int a,int b)
{
    List[nList].ab=a+b;
    List[nList].a=a;
    List[nList].b=b;
    List[nList].tag=0;
    nList++;
    return (nList-1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV EdgeList::AddEdge_(int a,int b)
{
	int n=WitchEdge(a,b);
	if (n==-1)
	{
		List[nList].ab=a+b;
		List[nList].a=a;
		List[nList].b=b;
		List[nList].tag=0;
		nList++;
	}
	else List[n].tag=1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV EdgeList::Free()
{
	delete[] List;
	List=NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV EdgeList::AddEdge2(int a,int b)
{
	int n=nList;
	List[n].ab=a+b;
	List[n].a=a;
	List[n].b=b;
	List[n].tag=0;
	nList++;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV EdgeList::Del(int ne)
{
	int n;

	if (nList>0)
	{
		for (n=ne;n<nList;n++) List[n]=List[n+1];
		nList--;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV EdgeList::DelDoubleEdges()
{
	int n1,n2;

	n1=0;
	while (n1<nList)
	{
		n2=n1+1;
		while (n2<nList)
		{
			if (((List[n1].a==List[n2].a)&&(List[n1].b==List[n2].b))||((List[n1].a==List[n2].b)&&(List[n1].b==List[n2].a))) 
			{ Del(n2);Del(n1); return; }
			else n2++;
		}
		n1++;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class : EdgeList2
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV EdgeList2::Init(int nmax) { List=new Edge2[nmax];nList=0;Max=nmax;}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int EDGESCONV EdgeList2::WitchEdge(unsigned int a, unsigned int b)
{
	int n=0;
	unsigned int tag;

	if (a<b) tag=(a<<16) + b;
	else tag=(b<<16) + a;

	while (n<nList)
	{	
		if (List[n].tag==tag) return n;
		else n++;
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV EdgeList2::AddEdge(unsigned int a,unsigned int b)
{
	unsigned int tag;
	if (WitchEdge(a,b)==-1)
	{
		if (a<b) tag=(a<<16) + b;
		else tag=(b<<16) + a;
		List[nList].a=a;
		List[nList].b=b;
		List[nList].tag=tag;
		nList++;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV EdgeList2::Free()
{
	delete[] List;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class : EdgeListD
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EdgeListD::EdgeListD() { List=ListD=NULL;}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV EdgeListD::Init(int nmax) { List=new Edge[nmax];ListD=new Edge[nmax];nList=0;nListD=0;Max=nmax;}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
int EDGESCONV EdgeListD::QuelEdge(int a, int b)
{
	int n=0;

	while (n<nList)
	{	
		//if (((List[n].a==a)&&(List[n].b==b))||((List[n].a==b)&&(List[n].b==a))) res=n;
		if (((List[n].a==b)&&(List[n].b==a))) return n;
		else n++;
	}

	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int EDGESCONV EdgeListD::QuelEdge0(int a, int b)
{
	int n=0;

	while (n<nList)
	{				
		if (((List[n].a==a)&&(List[n].b==b))) return n;
		else n++;
	}

	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int EDGESCONV EdgeListD::QuelEdge2(int a, int b)
{
	int n=0;

	while (n<nList)
	{	
		if (((List[n].a==a)&&(List[n].b==b))||((List[n].a==b)&&(List[n].b==a))) return n;
		else n++;
	}

	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int EDGESCONV EdgeListD::WitchEdge(int a, int b)
{
	int n=0;

	while (n<nList)
	{	
		if (((List[n].a==a)&&(List[n].b==b))||((List[n].a==b)&&(List[n].b==a))) return n;
		else n++;
	}

	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int EDGESCONV EdgeListD::QuelEdgeD(int a, int b)
{
	int n=0;

	if (nListD>0)
	while (n<nListD)
	{	
		if (((ListD[n].a==a)&&(ListD[n].b==b))||((ListD[n].a==b)&&(ListD[n].b==a))) return n;
		else n++;
	}

	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int EDGESCONV EdgeListD::QuelEdgeD2(int a, int b)
{
    int n=0;

    if (nListD>0)
    while (n<nListD)
    {
        if ((ListD[n].a==a)&&(ListD[n].b==b)) return n;
        else n++;
    }

    return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int EDGESCONV EdgeListD::Suivant(int a)
{
	int n=0;
	while (n<nList)
	{	
		if (List[n].tag==0)
		{
			if (List[n].a==a) return n;
			else n++;
		}
		else n++;
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV EdgeListD::AddEdge(int a,int b)
{
	int n;

	n=QuelEdge(a,b);

	if (n==-1)
	{
		List[nList].a=a;	
		List[nList].b=b;
		List[nList].tag=0;
		nList++;
	}
	else
	{
		Elimine(n);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV EdgeListD::AddEdge22(int a,int b)
{
    int n;

    n=QuelEdge(a,b);

    if (n==-1)
    {
        List[nList].a=a;
        List[nList].b=b;
        List[nList].tag=0;
        nList++;
    }
    else
    {
        Elimine2(n);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int EDGESCONV EdgeListD::nbEdge(int a,int b)
{
    int n;
    int nb=0;
    for (n=0;n<nList;n++)
        if (((List[n].a==a)&&(List[n].b==b))||((List[n].a==b)&&(List[n].b==a))) nb++;
    return nb;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int EDGESCONV EdgeListD::nbEdgeD(int a,int b)
{
    int n;
    int nb=0;
    for (n=0;n<nListD;n++)
        if (((ListD[n].a==a)&&(ListD[n].b==b))||((ListD[n].a==b)&&(ListD[n].b==a))) nb++;
    return nb;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV EdgeListD::AddEdgeD(int a,int b)
{
    int n,nn;

    n=QuelEdge(a,b);
    nn=QuelEdgeD(a,b);

    if ((n==-1)&&(nn==-1))
    {
        List[nList].a=a;
        List[nList].b=b;
        List[nList].tag=0;
        nList++;
    }
    else
    {
        if (n>=0) Elimine(n);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV EdgeListD::AddEdgeD2(int a,int b)
{
    int n,nn;

    n=QuelEdge(a,b);
    nn=QuelEdgeD(a,b);

    if ((n==-1)&&(nn==-1))
    {
        List[nList].a=a;
        List[nList].b=b;
        List[nList].tag=0;
        nList++;
    }
    else
    {
        if (n>=0) Elimine2(n);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV EdgeListD::AddEdge2(int a,int b)
{
	int n;

	n=QuelEdge(a,b);

	if (n==-1)
	{
		List[nList].a=a;	
		List[nList].b=b;
		List[nList].tag=0;
		nList++;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV EdgeListD::AddEdge3(int a,int b)
{
	int n;

	n=QuelEdge(a,b);

	if (n==-1)
	{
		if (QuelEdge2(a,b)==-1)
		{
			List[nList].a=a;	
			List[nList].b=b;
			List[nList].tag=0;
			nList++;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV EdgeListD::AddEdge0(int a,int b,int tag)
{
	List[nList].a=a;	
	List[nList].b=b;
	List[nList].tag=tag;
	nList++;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int EDGESCONV EdgeListD::Next(int a)
{
	int n=0;
	while (n<nList)
	{	
		if (List[n].tag==0)
		{
			if (List[n].a==a) return n;
			else n++;
		}
		else n++;
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int EDGESCONV EdgeListD::Nextz(int a)
{
	int n=0;
	while (n<nList)
	{	
		if (List[n].a==a) return n;
		else n++;
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int EDGESCONV EdgeListD::Next2(int a)
{
	int n=0;
	while (n<nList)
	{	
		if (List[n].tag==0)
		{
			if (List[n].a==a) { List[n].tag=1;return List[n].b;}
			if (List[n].b==a) { List[n].tag=1;return List[n].a;}
			n++;
		}
		else n++;
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int EDGESCONV EdgeListD::Next2(int a,int *corres)
{
	int n=0;
	while (n<nList)
	{	
		if (List[n].tag==0)
		{
			if (corres[List[n].a]==corres[a]) { List[n].tag=1;return List[n].b;}
			if (corres[List[n].b]==corres[a]) { List[n].tag=1;return List[n].a;}
			n++;
		}
		else n++;
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV EdgeListD::Free()
{
	if (List) delete [] List;
	if (ListD) delete [] ListD;
	List=ListD=NULL;
	nList=nListD=0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV EdgeListD::Elimine(int ne)
{
	if (nList>0)
	{
		ListD[nListD++]=List[ne];
        if (nList>ne) memcopy(&List[ne],&List[ne+1],(nList-ne-1)*sizeof(Edge));
		nList--;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV EdgeListD::Elimine2(int ne)
{
    if (nList>0)
    {
        ListD[nListD++]=List[ne];
        nList--;
        if (nList>ne) memcopy(&List[ne],&List[nList],sizeof(Edge));
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class : _EdgeListD
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
_EdgeListD::_EdgeListD() { List=ListD=NULL;}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV _EdgeListD::Init(int nmax) { List=new _Edge[nmax];ListD=new _Edge[nmax];nList=0;nListD=0;Max=nmax;}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
int EDGESCONV _EdgeListD::QuelEdge(int a, int b)
{
	int n=0;

	while (n<nList)
	{	
		//if (((List[n].a==a)&&(List[n].b==b))||((List[n].a==b)&&(List[n].b==a))) res=n;
		if (((List[n].a==b)&&(List[n].b==a))) return n;
		else n++;
	}

	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int EDGESCONV _EdgeListD::QuelEdge0(int a, int b)
{
	int n=0;

	while (n<nList)
	{				
		if (((List[n].a==a)&&(List[n].b==b))) return n;
		else n++;
	}

	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int EDGESCONV _EdgeListD::QuelEdge2(int a, int b)
{
	int n=0;

	while (n<nList)
	{	
		if (((List[n].a==a)&&(List[n].b==b))||((List[n].a==b)&&(List[n].b==a))) return n;
		else n++;
	}

	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int EDGESCONV _EdgeListD::WitchEdge(int a, int b)
{
	int n=0;

	while (n<nList)
	{	
		if (((List[n].a==a)&&(List[n].b==b))||((List[n].a==b)&&(List[n].b==a))) return n;
		else n++;
	}

	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int EDGESCONV _EdgeListD::QuelEdgeD(int a, int b)
{
	int n=0;

	if (nListD>0)
	while (n<nListD)
	{	
		if (((ListD[n].a==a)&&(ListD[n].b==b))||((ListD[n].a==b)&&(ListD[n].b==a))) return n;
		else n++;
	}

	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int EDGESCONV _EdgeListD::Suivant(int a)
{
	int n=0;
	while (n<nList)
	{	
		if (List[n].tag==0)
		{
			if (List[n].a==a) return n;
			else n++;
		}
		else n++;
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV _EdgeListD::AddEdge(int a,int b)
{
	int n;

	n=QuelEdge(a,b);

	if (n==-1)
	{
		List[nList].a=a;	
		List[nList].b=b;
		List[nList].tag=0;
		nList++;
	}
	else
	{
		Elimine(n);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV _EdgeListD::AddEdgeNT(int a,int b,int nt)
{
	int n;

	n=QuelEdge(a,b);

	if (n==-1)
	{
		List[nList].a=a;	
		List[nList].b=b;
		List[nList].tag=0;
		List[nList].nt=nt;
		nList++;
	}
	else
	{
		Elimine(n);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV _EdgeListD::AddEdge2(int a,int b)
{
	int n;

	n=QuelEdge(a,b);

	if (n==-1)
	{
		List[nList].a=a;	
		List[nList].b=b;
		List[nList].tag=0;
		nList++;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV _EdgeListD::AddEdge3(int a,int b)
{
	int n;

	n=QuelEdge(a,b);

	if (n==-1)
	{
		if (QuelEdge2(a,b)==-1)
		{
			List[nList].a=a;	
			List[nList].b=b;
			List[nList].tag=0;
			nList++;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV _EdgeListD::AddEdge0(int a,int b,int tag)
{
	List[nList].a=a;	
	List[nList].b=b;
	List[nList].tag=tag;
	nList++;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int EDGESCONV _EdgeListD::Next(int a)
{
	int n=0;
	while (n<nList)
	{	
		if (List[n].tag==0)
		{
			if (List[n].a==a) return n;
			else n++;
		}
		else n++;
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int EDGESCONV _EdgeListD::Next2(int a)
{
	int n=0;
	while (n<nList)
	{	
		if (List[n].tag==0)
		{
			if (List[n].a==a) { List[n].tag=1;return List[n].b;}
			if (List[n].b==a) { List[n].tag=1;return List[n].a;}
			n++;
		}
		else n++;
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int EDGESCONV _EdgeListD::Next2(int a,int *corres)
{
	int n=0;
	while (n<nList)
	{	
		if (List[n].tag==0)
		{
			if (corres[List[n].a]==corres[a]) { List[n].tag=1;return List[n].b;}
			if (corres[List[n].b]==corres[a]) { List[n].tag=1;return List[n].a;}
			n++;
		}
		else n++;
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV _EdgeListD::Free()
{
	if (List) delete [] List;
	if (ListD) delete [] ListD;
	List=ListD=NULL;
	nList=nListD=0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV _EdgeListD::Elimine(int ne)
{
	if (nList>0)
	{
		ListD[nListD++]=List[ne];
		if (nList>ne) memcopy(&List[ne],&List[ne+1],(nList-ne-1)*sizeof(_Edge));
		nList--;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV memcopy(void * _dst,void * _src,int _len)
{
	unsigned int * dst=(unsigned int *)_dst;
	unsigned int * src=(unsigned int *)_src;
	int nb=_len>>2;
	int n;
	for (n=0;n<nb;n++) *dst++=*src++;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
