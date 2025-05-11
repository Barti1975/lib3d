
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

#ifndef _EDGES_H_
#define _EDGES_H_

#ifdef WIN32
#define EDGESCONV __fastcall
#else
#define EDGESCONV
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB Edge
{
public:
	int a,b;
	unsigned int tag;
	float value;
	int nf1,nf2;
	int ab;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB _Edge
{
public:
	int a,b;
	unsigned int tag;
    int nt;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB EdgeG
{
public:
	int a,b;
	int cp;
	int num1;
	int num2;
	void * p1;
	void * p2;
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Edge lists 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class API3D_STUB EdgeList
{
public:
	Edge *List;
	int nList;
	int Max;

	EdgeList();
	~EdgeList();

	void EDGESCONV Init(int nmax);
	int EDGESCONV WitchEdge(int a, int b);
	int EDGESCONV WitchEdge2(unsigned int base,int a, int b);
	int EDGESCONV WitchEdgeStrict(int a, int b);
	int EDGESCONV Next(int a);
    void EDGESCONV AddEdge(int a,int b);
    int EDGESCONV AddEdgeR(int a,int b);
	int EDGESCONV iAddEdge(int a,int b);
	void EDGESCONV AddEdge_(int a,int b);
	void EDGESCONV Free();
	void EDGESCONV AddEdge2(int a,int b);
	void EDGESCONV Del(int ne);
	void EDGESCONV DelDoubleEdges();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB Edge2
{
public:
	unsigned int a,b;
	unsigned int tag;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB EdgeList2
{
public:
	Edge2 *List;
	int nList;
	int Max;

	EdgeList2() {}

	void EDGESCONV Init(int nmax);
	int EDGESCONV WitchEdge(unsigned int a, unsigned int b);
	void EDGESCONV AddEdge(unsigned int a,unsigned int b);
	void EDGESCONV Free();
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB EdgeListD
{
public:
	Edge *List;
	int nList;
	Edge *ListD;
	int nListD;
	int Max;

	EdgeListD();

	void EDGESCONV Init(int nmax);
	int EDGESCONV QuelEdge(int a, int b);
	int EDGESCONV QuelEdge0(int a, int b);
	int EDGESCONV QuelEdge2(int a, int b);
	int EDGESCONV WitchEdge(int a, int b);
	int EDGESCONV QuelEdgeD(int a, int b);
    int EDGESCONV QuelEdgeD2(int a, int b);
	int EDGESCONV Suivant(int a);
	void EDGESCONV AddEdge(int a,int b);
    void EDGESCONV AddEdge22(int a,int b);
    void EDGESCONV AddEdgeD(int a,int b);
    void EDGESCONV AddEdgeD2(int a,int b);
    int EDGESCONV nbEdge(int a,int b);
    int EDGESCONV nbEdgeD(int a,int b);
	void EDGESCONV AddEdge2(int a,int b);
	void EDGESCONV AddEdge3(int a,int b);
	void EDGESCONV AddEdge0(int a,int b,int tag);
	int EDGESCONV Next(int a);
    int EDGESCONV Nextz(int a);
	int EDGESCONV Next2(int a);
	int EDGESCONV Next2(int a,int *corres);
	void EDGESCONV Free();
	void EDGESCONV Elimine(int ne);
    void EDGESCONV Elimine2(int ne);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB _EdgeListD
{
public:
	_Edge *List;
	int nList;
	_Edge *ListD;
	int nListD;
	int Max;

	_EdgeListD();

	void EDGESCONV Init(int nmax);
	int EDGESCONV QuelEdge(int a, int b);
	int EDGESCONV QuelEdge0(int a, int b);
	int EDGESCONV QuelEdge2(int a, int b);
	int EDGESCONV WitchEdge(int a, int b);
	int EDGESCONV QuelEdgeD(int a, int b);
	int EDGESCONV Suivant(int a);
	void EDGESCONV AddEdge(int a,int b);
    void EDGESCONV AddEdgeNT(int a,int b,int nt);
	void EDGESCONV AddEdge2(int a,int b);
	void EDGESCONV AddEdge3(int a,int b);
	void EDGESCONV AddEdge0(int a,int b,int tag);
	int EDGESCONV Next(int a);
	int EDGESCONV Next2(int a);
	int EDGESCONV Next2(int a,int *corres);
	void EDGESCONV Free();
	void EDGESCONV Elimine(int ne);
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EDGESCONV memcopy(void * _dst,void * _src,int _len);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class EdgeInfo
{
public:
    int b,ndx,tag;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "list.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class QuickEdges
{
public:
    CList <EdgeInfo> *edges;
    int nb;
    int index;
    int *refa;
    int *refb;
    int *tags;
    
    QuickEdges() { edges=NULL; tags=refa=refb=NULL; nb=index=0; }
    
    void Init(int nv)
    {
        edges=new CList <EdgeInfo>[nv];
        nb=nv;
        index=0;
        tags=refa=refb=NULL;
    }
    
    void AddOrder(int a,int b)
    {
        EdgeInfo *pi=edges[a].GetFirst();
        while (pi)
        {
            if (pi->b==b)
            {
                pi->tag=1;
                return;
            }
            pi=edges[a].GetNext();
        }
        
        EdgeInfo *nfo=edges[a].InsertLast();
        nfo->b=b;
        nfo->tag=0;
        nfo->ndx=index++;
    }
    
    void Add(int a,int b)
    {
        if (a<b) AddOrder(a,b);
        else AddOrder(b,a);
    }
    
    int OrderWitch(int a,int b)
    {
        EdgeInfo *pi=edges[a].GetFirst();
        while (pi)
        {
            if (pi->b==b) return pi->ndx;
            pi=edges[a].GetNext();
        }
        return -1;
    }
    
    int Witch(int a,int b)
    {
        if (a<b) return OrderWitch(a,b);
        else return OrderWitch(b,a);
    }
    
    void Free()
    {
        if (edges)
        {
            for (int n=0;n<nb;n++) edges[n].Free();
            delete[] edges;
            edges=NULL;
            nb=0;
            index=0;
            if (refa) delete[] refa;
            if (refb) delete[] refb;
            if (tags) delete[] tags;
            tags=refa=refb=NULL;
        }
    }
    
    void Fix()
    {
        refa=new int[index];
        refb=new int[index];
        tags=new int[index];
        
        for (int n=0;n<nb;n++)
        {
            EdgeInfo *pi=edges[n].GetFirst();
            while (pi)
            {
                refa[pi->ndx]=n;
                refb[pi->ndx]=pi->b;
                tags[pi->ndx]=pi->tag;
                pi=edges[n].GetNext();
            }
        }
    }
    
    int a(int n) { return refa[n]; }
    int b(int n) { return refb[n]; }
    int tag(int n) { return tags[n]; }
    
    int Nb()
    {
        return index;
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
