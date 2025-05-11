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
//	@file bsp_2d.cpp 
//	@date 2004
////////////////////////////////////////////////////////////////////////

#include "maths.h"
#include "objects3d.h"
#include "bsp_2d.h"

bool CBSP2D::Inside(CBSP2DNode *node,CVector &v)
{
	bool res=false;
	float s;

	if (node->id==ID2D_NODE)
	{
		s=node->a*v.x+node->b*v.y+node->c*v.z+node->d;
		
		if (s>0) res=Inside(node->p,v);
		else res=Inside(node->m,v);
	}
	else
	{
		if (node->id==ID2D_IN) res=true;
		else res=false;
	}

	return res;
}

void CBSP2D::Build(CFigure *f,CVertex *vertices,CVector &Normal)
{
	int n,nmax;
	CSegment seg;
	CList <CSegment> lseg;
	int a,b;

	nmax=0;
	lseg.Free();
	for (n=0;n<f->List.Length();n++)
	{
		a=*(f->List[n]);
		b=*(f->List[(n+1)%f->List.Length()]);

		seg.a=vertices[a].Stok;
		seg.b=vertices[b].Stok;

		if (a>nmax) nmax=a;
		if (b>nmax) nmax=b;

		lseg.Add(seg);
	}

	N=Normal;

	root=BuildNode(&lseg);
	lseg.Free();
}

void CBSP2D::Apply(CBSP2DNode *node,CList <CSegmenti> *lseg)
{
	CSegmenti *seg,sp,sm;
	CList <CSegmenti> plus,moins;
	float a,b,c,d;
	float t,sa,sb;
	int cc;
	CVector C;


	if (node->id==ID2D_NODE)
	{
		a=node->a;
		b=node->b;
		c=node->c;
		d=node->d;

		seg=lseg->GetFirst();
		while (seg)
		{
			sa=a*points[seg->a].x+b*points[seg->a].y+c*points[seg->a].z +d;
			sb=a*points[seg->b].x+b*points[seg->b].y+c*points[seg->b].z +d;

			cc=(sa>0)+(sb>0);

			if (cc==0) moins.Add(*seg);
			else 
			{
				if (cc>=2) plus.Add(*seg);
				else
				{
					if (sa<0)
					{
						t=-sa/(sb-sa);

						C=points[seg->a] +t*(points[seg->b] - points[seg->a]);

						sm.a=seg->a;
						sm.b=npoints;

						sp.a=npoints;
						sp.b=seg->b;

						points[npoints]=C;
						npoints++;
		
						moins.Add(sm);
						plus.Add(sp);
					}
					else
					{
						t=-sa/(sb-sa);

						C=points[seg->a] +t*(points[seg->b] - points[seg->a]);

						sp.a=seg->a;
						sp.b=npoints;

						sm.a=npoints;
						sm.b=seg->b;

						points[npoints]=C;
						npoints++;
		
						moins.Add(sm);
						plus.Add(sp);
					}
				}
			}

			seg=lseg->GetNext();
		}

		if (plus.Length()>0) Apply(node->p,&plus);
		if (moins.Length()>0) Apply(node->m,&moins);

		plus.Free();
		moins.Free();
	}
	
	if (node->id==ID2D_IN)
	{
		seg=lseg->GetFirst();
		while (seg)
		{
			seg_in.Add(*seg);
			seg=lseg->GetNext();
		}
	}

	if (node->id==ID2D_OUT)
	{
		seg=lseg->GetFirst();
		while (seg)
		{
			seg_out.Add(*seg);
			seg=lseg->GetNext();
		}
	}
}

void CBSP2D::FreeNode(CBSP2DNode *node)
{
	if (node->id==ID2D_NODE)
	{
		FreeNode(node->p);
		FreeNode(node->m);
	}
	delete node;
}

CBSP2DNode *CBSP2D::BuildNode(CList <CSegment> *lseg)
{
	CBSP2DNode *node=NULL;
	float a,b,c,d;
	float sa,sb,t;
	int cc;
	CVector C,u;
	CSegment *seg;
	CList <CSegment> plus,moins;
	CSegment sg1,sg2;

	if (lseg->Length()>0)
	{
		node=new CBSP2DNode;
		seg=lseg->GetFirst();
		u=seg->b-seg->a;
		u=N^u;
		u.Normalise();
		a=u.x;
		b=u.y;
		c=u.z;
		d=-(u||seg->a);

		node->a=a;
		node->b=b;
		node->c=c;
		node->d=d;

		node->id=ID2D_NODE;

		if (lseg->Length()>1)
		{
			seg=lseg->GetNext();
			while (seg)
			{
				sa=a*seg->a.x+b*seg->a.y+c*seg->a.z +d;
				sb=a*seg->b.x+b*seg->b.y+c*seg->b.z +d;

				cc=(sa>SMALLF3)+(sb>SMALLF3);

				if (cc==0) moins.Add(*seg);
				else 
				{
					if ((sa<SMALLF3)&&(f_abs(sa)<SMALLF3)) cc++;
					if ((sb<SMALLF3)&&(f_abs(sb)<SMALLF3)) cc++;
					if (cc>=2) plus.Add(*seg);
					else
					{

						t=-sa/(sb-sa);

						C=seg->a +t*(seg->b - seg->a);
					
						sg1.a=seg->a;
						sg1.b=C;

						sg2.a=C;
						sg2.b=seg->b;

						if (sa<SMALLF3)
						{
							moins.Add(sg1);
							plus.Add(sg2);
						}
						else
						{
							moins.Add(sg2);
							plus.Add(sg1);
						}

					}
					/**/
					
				}
				seg=lseg->GetNext();
			}

			if (plus.Length()>0)
			{
				node->p=BuildNode(&plus);
			}
			else
			{
				node->p=new CBSP2DNode;
				node->p->id=ID2D_OUT;
			}

			if (moins.Length()>0)
			{
				node->m=BuildNode(&moins);
			}
			else
			{
				node->m=new CBSP2DNode;
				node->m->id=ID2D_IN;
			}

			plus.Free();
			moins.Free();

		}
		else
		{
			node->p=new CBSP2DNode;
			node->p->id=ID2D_OUT;
			node->m=new CBSP2DNode;
			node->m->id=ID2D_IN;

		}
	}

	return node;
}


void CBSP2D::Free()
{
	FreeNode(root);
}
