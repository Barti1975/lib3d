////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	  <3D Librairies developped during 2002-2021>
//	  Copyright (C) <2021>  <Laurent Canc� Francis, 10/08/1975>
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


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// comments, adds:
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "params.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <math.h>
#include "maths.h"
#include "list.h"
#include "objects3d.h"
#include "triangularisation.h"
////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern bool LOGING;
extern void LIB3DLog(char *str);
extern void LIB3DLog(char *str,CVector u);
#define LOG(str) { if (LOGING) LIB3DLog(str); }
#define LOGV(str,param) { if (LOGING) LIB3DLog(str,param); }
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define					SENS_CW						1
#define					SENS_CCW					2
////////////////////////////////////////////////////////////////////////////////////////////////////////////
int						SENS						=	SENS_CW;
////////////////////////////////////////////////////////////////////////////////////////////////////////////
float					SMALLF_TRIANGULARISATION	=	SMALLF;
float					SMALLF_TRIANGULARISATION2	=	SMALLF2;
float					SMALLF_TRIANGULARISATION3	=	SMALLF2;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
int						count_number_of_triangularisation	=	0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool TRIANGULARISATIONCONV TRIANGULARISATION_PlanSecant(CVertex * base_vertices,CFigure *Figure,float a,float b,float c,float d,int n1,int n2)
{
	bool res;
	float s;
	int *i;
	res=false;

	if (SENS==SENS_CW)
	{
		i=Figure->List.GetFirst();
		while (i)
		{
			if (((*i)!=n1)&&((*i)!=n2))
			{
				s=a*base_vertices[*i].Stok.x +b*base_vertices[*i].Stok.y +c*base_vertices[*i].Stok.z +d;
				if (s>SMALLF_TRIANGULARISATION) return true;
			}
			i=Figure->List.GetNext();
		}
	}
	else
	{
		i=Figure->List.GetFirst();
		while (i)
		{
			if (((*i)!=n1)&&((*i)!=n2))
			{
				s=a*base_vertices[*i].Stok.x +b*base_vertices[*i].Stok.y +c*base_vertices[*i].Stok.z +d;
				if (s<-SMALLF_TRIANGULARISATION) return true;
			}
			i=Figure->List.GetNext();
		}
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool TRIANGULARISATIONCONV TRIANGULARISATION_NouveauSegmentIntersectionnant(CVertex * base_vertices,CFigure *Figure,CVector N,int nn1,int nn2,bool tag_TNSI)
{
	int n;
	int i1,i2;
	CVector m1,m2,u,I,A,B;
	float s1,s2,t;
	float a,b,c,d;
	float a0,b0,c0,d0;
	float a1,b1,c1,d1;
	CVector u1,u2;
	float fn;

	VECTORSUB(m1,base_vertices[nn2].Stok,base_vertices[nn1].Stok);
	VECTOR3INIT(u2,m1);
	CROSSPRODUCT(m2,N,m1);
	VECTORNORMALISE(m2);

	a=m2.x;
	b=m2.y;
	c=m2.z;
	d=-DOT(m2,base_vertices[nn2].Stok);

	VECTORNORMALISE(u2);

	if (tag_TNSI)
	{
		n=0;
		CElement <int>*el=Figure->List.First;
		while (n<Figure->List.Length())
		{
			//CElement <int>*el=Figure->List(n);
			i1=el->data;
			if (el->next) i2=el->next->data; else i2=Figure->List.First->data;

			if ((i1!=nn1)&&(i1!=nn2)&&(i2!=nn1)&&(i2!=nn2))
			{
				VECTORSUB(u1,base_vertices[i2].Stok,base_vertices[i1].Stok);
				fn=VECTORNORM(u1);
				if (fn>SMALLF_TRIANGULARISATION2)
				{
					VECTORDIV(u1,u1,fn);

					if (f_abs(DOT(u1,u2))>=1.0f-SMALLF_TRIANGULARISATION2)
					{
						VECTORSUB(m1,base_vertices[nn2].Stok,base_vertices[nn1].Stok);
						VECTORNORMALISE(m1);
						a1=m1.x;
						b1=m1.y;
						c1=m1.z;
						d1=-DOT(m1,base_vertices[nn2].Stok);

						s1=a1*base_vertices[i1].Stok.x+b1*base_vertices[i1].Stok.y+c1*base_vertices[i1].Stok.z+d1;
						s2=a1*base_vertices[i2].Stok.x+b1*base_vertices[i2].Stok.y+c1*base_vertices[i2].Stok.z+d1;
						t=-s1/(s2-s1);
						VECTORINTERPOL(B,t,base_vertices[i2].Stok,base_vertices[i1].Stok);

						s1=a1*base_vertices[nn1].Stok.x+b1*base_vertices[nn1].Stok.y+c1*base_vertices[nn1].Stok.z+d1;
						s2=a1*base_vertices[nn2].Stok.x+b1*base_vertices[nn2].Stok.y+c1*base_vertices[nn2].Stok.z+d1;
						t=-s1/(s2-s1);
						VECTORINTERPOL(A,t,base_vertices[nn2].Stok,base_vertices[nn1].Stok);

						VECTORSUB(u,A,B);
						if (VECTORNORM(u)<SMALLF_TRIANGULARISATION2) return true;
					}
					else
					{
						s1=a*base_vertices[i1].Stok.x+b*base_vertices[i1].Stok.y+c*base_vertices[i1].Stok.z+d;
						s2=a*base_vertices[i2].Stok.x+b*base_vertices[i2].Stok.y+c*base_vertices[i2].Stok.z+d;

						t=-s1/(s2-s1);

						if ((t>=-SMALLF_TRIANGULARISATION)&&(t<=1.0f+SMALLF_TRIANGULARISATION))
						{
							VECTORSUB(m1,base_vertices[i2].Stok,base_vertices[i1].Stok);
							CROSSPRODUCT(m2,N,m1);
							VECTORNORMALISE(m2);

							a0=m2.x;
							b0=m2.y;
							c0=m2.z;
							d0=-DOT(m2,base_vertices[i2].Stok);

							s1=a0*base_vertices[nn1].Stok.x+b0*base_vertices[nn1].Stok.y+c0*base_vertices[nn1].Stok.z+d0;
							s2=a0*base_vertices[nn2].Stok.x+b0*base_vertices[nn2].Stok.y+c0*base_vertices[nn2].Stok.z+d0;

							t=-s1/(s2-s1);
							if ((t>=-SMALLF_TRIANGULARISATION)&&(t<=1.0f+SMALLF_TRIANGULARISATION))
							{
								return true;
							}
						}
					}
				}
			}

			if (el) el=el->next;
			n++;
		}

	}
	else
	{
		n=0;
		CElement <int>*el=Figure->List.First;
		while (n<Figure->List.Length())
		{
			i1=el->data;
			if (el->next) i2=el->next->data; else i2=Figure->List.First->data;

			if ((i1!=nn1)&&(i1!=nn2)&&(i2!=nn1)&&(i2!=nn2))
			{
				VECTORSUB(u1,base_vertices[i2].Stok,base_vertices[i1].Stok);
				fn=VECTORNORM(u1);
				if (fn>SMALLF_TRIANGULARISATION2)
				{
					VECTORDIV(u1,u1,fn);

					s1=a*base_vertices[i1].Stok.x+b*base_vertices[i1].Stok.y+c*base_vertices[i1].Stok.z+d;
					s2=a*base_vertices[i2].Stok.x+b*base_vertices[i2].Stok.y+c*base_vertices[i2].Stok.z+d;

					if (s1*s2<0.0f)
					{
						t=-s1/(s2-s1);

						if ((t>=SMALLF_TRIANGULARISATION2)&&(t<=1.0f-SMALLF_TRIANGULARISATION2))
						{
							VECTORSUB(m1,base_vertices[i2].Stok,base_vertices[i1].Stok);
							CROSSPRODUCT(m2,N,m1);
							VECTORNORMALISE(m2);

							a0=m2.x;
							b0=m2.y;
							c0=m2.z;
							d0=-DOT(m2,base_vertices[i2].Stok);

							s1=a0*base_vertices[nn1].Stok.x+b0*base_vertices[nn1].Stok.y+c0*base_vertices[nn1].Stok.z+d0;
							s2=a0*base_vertices[nn2].Stok.x+b0*base_vertices[nn2].Stok.y+c0*base_vertices[nn2].Stok.z+d0;

							if (s1*s2<0.0f)
							{
								t=-s1/(s2-s1);
								if ((t>=SMALLF_TRIANGULARISATION2)&&(t<=1.0f-SMALLF_TRIANGULARISATION2)) return true;
							}
						}
					}
				}
			}
			if (el) el=el->next;
			n++;
		}
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool TRIANGULARISATIONCONV TRIANGULARISATION_PointsInsideTriangle(CVertex * base_vertices,CFigure *Figure,CVector N,int a,int b,int c)
{
	bool res=false;
	CVector m1,m2;
	float A[3],B[3],C[3],D[3];
	int *pi;
	int n,cc;
	float s;
	float l1,l2,l3;

	if ((a!=b)&&(b!=c)&&(a!=c))
	{
		VECTORSUB(m1,base_vertices[b].Stok,base_vertices[a].Stok);
		l1=VECTORNORM(m1);
		CROSSPRODUCT(m2,N,m1);
		VECTORNORMALISE(m2);

		A[0]=m2.x;
		B[0]=m2.y;
		C[0]=m2.z;
		D[0]=-DOT(m2,base_vertices[a].Stok);

		VECTORSUB(m1,base_vertices[c].Stok,base_vertices[b].Stok);
		l2=VECTORNORM(m1);
		CROSSPRODUCT(m2,N,m1);
		VECTORNORMALISE(m2);

		A[1]=m2.x;
		B[1]=m2.y;
		C[1]=m2.z;
		D[1]=-DOT(m2,base_vertices[b].Stok);
		
		VECTORSUB(m1,base_vertices[a].Stok,base_vertices[c].Stok);
		l3=VECTORNORM(m1);
		CROSSPRODUCT(m2,N,m1);
		VECTORNORMALISE(m2);

		A[2]=m2.x;
		B[2]=m2.y;
		C[2]=m2.z;
		D[2]=-DOT(m2,base_vertices[c].Stok);

		if ((l1>SMALLF_TRIANGULARISATION2)&&(l2>SMALLF_TRIANGULARISATION2)&&(l3>SMALLF_TRIANGULARISATION2))
		{
			pi=Figure->List.GetFirst();
			while ((pi)&&(!res))
			{
				if (((*pi)!=a)&&((*pi)!=b)&&((*pi)!=c))
				{
					VECTORSUB(m1,base_vertices[*pi].Stok,base_vertices[a].Stok);
					l1=VECTORNORM(m1);
					VECTORSUB(m1,base_vertices[*pi].Stok,base_vertices[b].Stok);
					l2=VECTORNORM(m1);
					VECTORSUB(m1,base_vertices[*pi].Stok,base_vertices[c].Stok);
					l3=VECTORNORM(m1);

					if ((l1>SMALLF_TRIANGULARISATION2)&&(l2>SMALLF_TRIANGULARISATION2)&&(l3>SMALLF_TRIANGULARISATION2))
					{
						cc=0;
						for (n=0;n<3;n++)
						{
							s=A[n]*base_vertices[*pi].Stok.x +B[n]*base_vertices[*pi].Stok.y +C[n]*base_vertices[*pi].Stok.z +D[n];
							cc+=(s>0.0f);
						}

						if (cc==0) res=true;
					}
				}
				pi=Figure->List.GetNext();
			}
		}
	}

	return res;
}

int OPTIMIZE_FORDEGEN=0;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool TRIANGULARISATIONCONV TriangularisationSimple(CVertex * base_vertices,CFigure *Figure,CVector N,
							 CVertex *Vertices,CShortFace *Faces,int *nv,int *nf,
							 int nT,int nT2,int nL,unsigned char ref,int TRIANGULARISATION_TAG)
{
	CVector u,v,U,u1,u2,lN;
	int n,nn;
	int i0,i1,i2,i3;
	int *ii;
	CShortFace *f;
	int * corres;
	CVertex *s;
	int NBRE_ITER;
	int precedent;
	float d,ss;
	bool valid=false;
	bool bug=false;

	if (Figure->List.Length()<3) return true;

	SMALLF_TRIANGULARISATION=SMALLF;
	SMALLF_TRIANGULARISATION2=SMALLF2;
	SMALLF_TRIANGULARISATION3=SMALLF2;

	NBRE_ITER=0;

	n=0;
	ii=Figure->List.GetFirst();
	while (ii)
	{
		if (*ii>n) n=*ii;
		ii=Figure->List.GetNext();
	}

	corres=(int*) malloc ((n+1)*sizeof(int));
	for (nn=0;nn<(n+1);nn++) corres[nn]=-1;

	ii=Figure->List.GetFirst();
	while (ii)
	{
		n=(*ii);
		
		if (corres[n]==-1)
		{
			corres[n]=*nv;
			s=&Vertices[*nv];
			
            VECTOR3INIT(s->Stok,base_vertices[n].Stok);
			VECTOR3INIT(s->Calc,base_vertices[n].Calc);
			VECTOR3INIT(s->Norm,base_vertices[n].Norm);
			
            s->Map=base_vertices[n].Map;
			s->Map2=base_vertices[n].Map2;
            
            s->Index[0]=base_vertices[n].Index[0];
            s->Index[1]=base_vertices[n].Index[1];
            s->Index[2]=base_vertices[n].Index[2];
            s->Index[3]=base_vertices[n].Index[3];
            
            s->Weight[0]=base_vertices[n].Weight[0];
            s->Weight[1]=base_vertices[n].Weight[1];
            s->Weight[2]=base_vertices[n].Weight[2];
            s->Weight[3]=base_vertices[n].Weight[3];
            
            s->tag2=base_vertices[n].tag2;
            
			(*nv)++;
		}
		ii=Figure->List.GetNext();
	}

	if (Figure->List.Length()==3)
	{
		CElement <int>*el=Figure->List.First;
		i0=el->data;
		i1=el->next->data;
		i2=el->next->next->data;

		f=&Faces[(*nf)++];
		f->n0=corres[i0]; f->n1=corres[i1]; f->n2=corres[i2];
		f->nT=nT; f->nT2=nT2; f->nL=nL; f->ref=ref; f->tag=TRIANGULARISATION_TAG;

		free(corres);

		return true;
	}

	if (Figure->List.Length()==4)
	{
		CElement <int>*el=Figure->List.First;
		i0=el->data;
		i1=el->next->data;
		i2=el->next->next->data;
		i3=el->next->next->next->data;

		if ((TRIANGULARISATION_PointsInsideTriangle(base_vertices,Figure,N,i0,i1,i2))||(TRIANGULARISATION_PointsInsideTriangle(base_vertices,Figure,N,i0,i2,i3)))
		{
			f=&Faces[(*nf)++];
			f->n0=corres[i0]; f->n1=corres[i1]; f->n2=corres[i3];
			f->nT=nT; f->nT2=nT2; f->nL=nL; f->ref=ref; f->tag=TRIANGULARISATION_TAG;

			f=&Faces[(*nf)++];
			f->n0=corres[i1];  f->n1=corres[i2]; f->n2=corres[i3];
			f->nT=nT; f->nT2=nT2; f->nL=nL; f->ref=ref; f->tag=TRIANGULARISATION_TAG;
		}
		else
		{
			f=&Faces[(*nf)++];
			f->n0=corres[i0]; f->n1=corres[i1]; f->n2=corres[i2];
			f->nT=nT; f->nT2=nT2; f->nL=nL; f->ref=ref; f->tag=TRIANGULARISATION_TAG;
			
			f=&Faces[(*nf)++];
			f->n0=corres[i0]; f->n1=corres[i2]; f->n2=corres[i3];
			f->nT=nT; f->nT2=nT2; f->nL=nL; f->ref=ref; f->tag=TRIANGULARISATION_TAG;
		}

		free(corres);

		return true;
	}


	SENS=SENS_CW;
	n=0;
	nn=Figure->List.Length();
	precedent=0;

	CElement <int>*el=Figure->List.First;

	while ((nn>1)&&(NBRE_ITER<2*Figure->List.Length()))
	{
		i1=el->data;
		if (el->next) i2=el->next->data; else i2=Figure->List.First->data;

		VECTORSUB(u,base_vertices[i2].Stok,base_vertices[i1].Stok);
		CROSSPRODUCT(U,N,u);
		VECTORNORMALISE(U);
		d=-DOT(U,base_vertices[i1].Stok);

		bool sec=true;
		if (precedent>=0) sec=TRIANGULARISATION_PlanSecant(base_vertices,Figure,U.x,U.y,U.z,d,i1,i2);

		if (sec)
		{
			if (el->prev) i0=el->prev->data; else i0=Figure->List.Closing->data;
			
			ss=(DOT(U,base_vertices[i0].Stok))+d;

			if (ss<=0.0f) valid=(!TRIANGULARISATION_NouveauSegmentIntersectionnant(base_vertices,Figure,N,i0,i2,false));
			else valid=false;

			if (valid) valid=(!TRIANGULARISATION_PointsInsideTriangle(base_vertices,Figure,N,i0,i1,i2));

			if (valid)
			{
				f=&Faces[*nf];
				(*nf)++;
				
				f->n0=corres[i0];
				f->n1=corres[i1];
				f->n2=corres[i2];
				f->nT=nT;
				f->nT2=nT2;
				f->nL=nL;
				f->ref=ref;
				f->tag=TRIANGULARISATION_TAG;

				precedent=0;

				CElement <int>*ael=el;

				el=el->prev;
				if (!el) el=Figure->List.Closing;
				//Figure->List.Del(n);

				Figure->List.index=-1;

				if (Figure->List.nb>n)
				{
					if (ael==Figure->List.First) Figure->List.DeleteFirst();
					else
					{
						if (ael==Figure->List.Closing) Figure->List.DeleteLast();
						else
						{
							(ael->prev)->next=ael->next;
							(ael->next)->prev=ael->prev;
							delete ael;
							Figure->List.nb--;
						}
					}
				}

				nn=Figure->List.Length();
				
				if (n-1>=0) n=n-1; else n=Figure->List.Length()-1;

				NBRE_ITER=0;
			}
			else 
			{ 
				n++;
				nn=Figure->List.Length();
				precedent=-1;
				el=el->next;
				if (!el) el=Figure->List.First;
			}
		}
		else
		{
			precedent=0;
			n++;
			nn--;
			el=el->next;
			if (!el) el=Figure->List.First;
		}

		if (n>=Figure->List.Length()) n=0;
		NBRE_ITER++;
	}

	if (Figure->List.Length()>2)
	{
		i0=Figure->List.First->data;
		CElement <int>*el=Figure->List.First->next;
		
		i1=el->data;
		VECTORSUB(u1,base_vertices[i1].Stok,base_vertices[i0].Stok);

		float sc2=SMALLF_TRIANGULARISATION2*SMALLF_TRIANGULARISATION2;
        
        if (OPTIMIZE_FORDEGEN)
        {
        
            int listvertex[2048];
            
            listvertex[0]=i0;
            listvertex[1]=i1;
            int nb=2;
            
            for (n=1;n<Figure->List.Length()-1;n++)
            {
                i2=el->next->data;
                listvertex[nb++]=i2;
                el=el->next;
            }
            
            int rez=0;
        
            for (int nk=0;nk<nb-1;nk++)
            {
                CVector uu1,uu2,uu3;
                int err=0;
                
                i0=listvertex[nk];
                
                for (n=1;n<nb-1;n++)
                {
                    i1=listvertex[(nk+n)%nb];
                    i2=listvertex[(nk+n+1)%nb];

                    VECTORSUB(uu1,base_vertices[i0].Stok,base_vertices[i1].Stok);
                    if (VECTORNORM2(uu1)<SMALLF) { err=1; break; }

                    VECTORSUB(uu2,base_vertices[i1].Stok,base_vertices[i2].Stok);
                    if (VECTORNORM2(uu2)<SMALLF) { err=1; break; }

                    VECTORSUB(uu3,base_vertices[i2].Stok,base_vertices[i0].Stok);
                    if (VECTORNORM2(uu3)<SMALLF) { err=1; break; }
                    
                    VECTORNORMALISE(uu1);
                    VECTORNORMALISE(uu2);
                    VECTORNORMALISE(uu3);
                    
                    if (f_abs(DOT(uu1,uu3))>0.999f) { err=1; break; }
                    if (f_abs(DOT(uu1,uu2))>0.999f) { err=1; break; }
                    if (f_abs(DOT(uu2,uu3))>0.999f) { err=1; break; }
                }
                
                if (err==0)
                {
                    rez=nk;
                    break;
                }
            }
            
            i0=listvertex[rez];

            n=1;
            i1=listvertex[(rez+n)%nb];
            i2=listvertex[(rez+n+1)%nb];
            VECTORSUB(u1,base_vertices[i1].Stok,base_vertices[i0].Stok);

            for (n=1;n<Figure->List.Length()-1;n++)
            {
                i1=listvertex[(rez+n)%nb];
                i2=listvertex[(rez+n+1)%nb];
                
                f=&Faces[*nf];
                (*nf)++;
                
                f->n0=corres[i0];
                f->n1=corres[i1];
                f->n2=corres[i2];
                f->nT=nT;
                f->nT2=nT2;
                f->nL=nL;
                f->ref=ref;
                f->tag=TRIANGULARISATION_TAG;
                
                VECTORSUB(u2,base_vertices[i2].Stok,base_vertices[i0].Stok);
                CROSSPRODUCT(lN,u2,u1);
                
                if (VECTORNORM2(lN)>sc2)
                {
                    if (DOT(lN,N)<0)
                    {
                        bug=true;
                    }
                }
                
                u1=u2;
            }

        }
        else
        {
            for (n=1;n<Figure->List.Length()-1;n++)
            {
                i2=el->next->data;

                f=&Faces[*nf];
                (*nf)++;

                f->n0=corres[i0];
                f->n1=corres[i1];
                f->n2=corres[i2];
                f->nT=nT;
                f->nT2=nT2;
                f->nL=nL;
                f->ref=ref;
                f->tag=TRIANGULARISATION_TAG;
                
                VECTORSUB(u2,base_vertices[i2].Stok,base_vertices[i0].Stok);
                CROSSPRODUCT(lN,u2,u1);

                if (VECTORNORM2(lN)>sc2)
                {
                    if (DOT(lN,N)<0)
                    {
                        bug=true;
                    }
                }

                i1=i2;
                u1=u2;
                el=el->next;
            }
        }
	}

	free(corres);

	return (!bug);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool TRIANGULARISATIONCONV TriangularisationSimpleTag(CVertex * base_vertices,CFigure *Figure,CVector N,
                             CVertex *Vertices,CShortFace *Faces,int *nv,int *nf,
                             int nT,int nT2,int nL,unsigned char ref,int TRIANGULARISATION_TAG)
{
    CVector u,v,U,u1,u2,lN;
    int n,nn;
    int i0,i1,i2,i3;
    int *ii;
    CShortFace *f;
    int * corres;
    CVertex *s;
    int NBRE_ITER;
    int precedent;
    float d,ss;
    bool valid=false;
    bool bug=false;

    if (Figure->List.Length()<3) return true;

    SMALLF_TRIANGULARISATION=SMALLF;
    SMALLF_TRIANGULARISATION2=SMALLF2;
    SMALLF_TRIANGULARISATION3=SMALLF2;

    NBRE_ITER=0;

    n=0;
    ii=Figure->List.GetFirst();
    while (ii)
    {
        if (*ii>n) n=*ii;
        ii=Figure->List.GetNext();
    }

    corres=(int*) malloc ((n+1)*sizeof(int));
    for (nn=0;nn<(n+1);nn++) corres[nn]=-1;

    ii=Figure->List.GetFirst();
    while (ii)
    {
        n=(*ii);
        
        if (corres[n]==-1)
        {
            corres[n]=*nv;
            s=&Vertices[*nv];
            
            VECTOR3INIT(s->Stok,base_vertices[n].Stok);
            VECTOR3INIT(s->Calc,base_vertices[n].Calc);
            VECTOR3INIT(s->Norm,base_vertices[n].Norm);
            
            s->Map=base_vertices[n].Map;
            s->Map2=base_vertices[n].Map2;
            
            s->Index[0]=base_vertices[n].Index[0];
            s->Index[1]=base_vertices[n].Index[1];
            s->Index[2]=base_vertices[n].Index[2];
            s->Index[3]=base_vertices[n].Index[3];
            
            s->Weight[0]=base_vertices[n].Weight[0];
            s->Weight[1]=base_vertices[n].Weight[1];
            s->Weight[2]=base_vertices[n].Weight[2];
            s->Weight[3]=base_vertices[n].Weight[3];
            
            s->tag2=base_vertices[n].tag2;
            
            (*nv)++;
        }
        ii=Figure->List.GetNext();
    }

    if (Figure->List.Length()==3)
    {
        CElement <int>*el=Figure->List.First;
        i0=el->data;
        i1=el->next->data;
        i2=el->next->next->data;

        f=&Faces[(*nf)++];
        f->n0=corres[i0]; f->n1=corres[i1]; f->n2=corres[i2];
        f->nT=nT; f->nT2=nT2; f->nL=nL; f->ref=ref; f->tag=TRIANGULARISATION_TAG;

        free(corres);

        return true;
    }

    if (Figure->List.Length()==4)
    {
        CElement <int>*el=Figure->List.First;
        i0=el->data;
        i1=el->next->data;
        i2=el->next->next->data;
        i3=el->next->next->next->data;

        if ((TRIANGULARISATION_PointsInsideTriangle(base_vertices,Figure,N,i0,i1,i2))||(TRIANGULARISATION_PointsInsideTriangle(base_vertices,Figure,N,i0,i2,i3)))
        {
            f=&Faces[(*nf)++];
            f->n0=corres[i0]; f->n1=corres[i1]; f->n2=corres[i3];
            f->nT=nT; f->nT2=nT2; f->nL=nL; f->ref=ref; f->tag=TRIANGULARISATION_TAG;

            f=&Faces[(*nf)++];
            f->n0=corres[i1];  f->n1=corres[i2]; f->n2=corres[i3];
            f->nT=nT; f->nT2=nT2; f->nL=nL; f->ref=ref; f->tag=TRIANGULARISATION_TAG;
        }
        else
        {
            f=&Faces[(*nf)++];
            f->n0=corres[i0]; f->n1=corres[i1]; f->n2=corres[i2];
            f->nT=nT; f->nT2=nT2; f->nL=nL; f->ref=ref; f->tag=TRIANGULARISATION_TAG;
            
            f=&Faces[(*nf)++];
            f->n0=corres[i0]; f->n1=corres[i2]; f->n2=corres[i3];
            f->nT=nT; f->nT2=nT2; f->nL=nL; f->ref=ref; f->tag=TRIANGULARISATION_TAG;
        }

        free(corres);

        return true;
    }


    SENS=SENS_CW;
    n=0;
    nn=Figure->List.Length();
    precedent=0;

    CElement <int>*el=Figure->List.First;

    while ((nn>1)&&(NBRE_ITER<20*Figure->List.Length()))
    {
        i1=el->data;
        if (el->next) i2=el->next->data; else i2=Figure->List.First->data;

        VECTORSUB(u,base_vertices[i2].Stok,base_vertices[i1].Stok);
        CROSSPRODUCT(U,N,u);
        VECTORNORMALISE(U);
        d=-DOT(U,base_vertices[i1].Stok);

        bool sec=true;
        if (precedent>=0) sec=TRIANGULARISATION_PlanSecant(base_vertices,Figure,U.x,U.y,U.z,d,i1,i2);

        if (sec)
        {
            if (el->prev) i0=el->prev->data; else i0=Figure->List.Closing->data;
            
            ss=(DOT(U,base_vertices[i0].Stok))+d;

            if (ss<=0.0f) valid=(!TRIANGULARISATION_NouveauSegmentIntersectionnant(base_vertices,Figure,N,i0,i2,false));
            else valid=false;

            if (valid) valid=(!TRIANGULARISATION_PointsInsideTriangle(base_vertices,Figure,N,i0,i1,i2));
            
            int mm=base_vertices[i1].tag+base_vertices[i2].tag;
            if (!(((mm==2)&&(base_vertices[i0].tag==0))||((mm==0)&&(base_vertices[i0].tag==1)))) valid=false;
            
            if (valid)
            {
                f=&Faces[*nf];
                (*nf)++;
                
                f->n0=corres[i0];
                f->n1=corres[i1];
                f->n2=corres[i2];
                f->nT=nT;
                f->nT2=nT2;
                f->nL=nL;
                f->ref=ref;
                f->tag=TRIANGULARISATION_TAG;

                precedent=0;

                CElement <int>*ael=el;

                el=el->prev;
                if (!el) el=Figure->List.Closing;
                //Figure->List.Del(n);

                Figure->List.index=-1;

                if (Figure->List.nb>n)
                {
                    if (ael==Figure->List.First) Figure->List.DeleteFirst();
                    else
                    {
                        if (ael==Figure->List.Closing) Figure->List.DeleteLast();
                        else
                        {
                            (ael->prev)->next=ael->next;
                            (ael->next)->prev=ael->prev;
                            delete ael;
                            Figure->List.nb--;
                        }
                    }
                }

                nn=Figure->List.Length();
                
                if (n-1>=0) n=n-1; else n=Figure->List.Length()-1;

                NBRE_ITER=0;
            }
            else
            {
                n++;
                nn=Figure->List.Length();
                precedent=-1;
                el=el->next;
                if (!el) el=Figure->List.First;
            }
        }
        else
        {
            precedent=0;
            n++;
            nn--;
            el=el->next;
            if (!el) el=Figure->List.First;
        }

        if (n>=Figure->List.Length()) n=0;
        NBRE_ITER++;
    }

    if (Figure->List.Length()>2)
    {
        i0=Figure->List.First->data;
        CElement <int>*el=Figure->List.First->next;
        
        i1=el->data;
        VECTORSUB(u1,base_vertices[i1].Stok,base_vertices[i0].Stok);

        float sc2=SMALLF_TRIANGULARISATION2*SMALLF_TRIANGULARISATION2;
        
        if (OPTIMIZE_FORDEGEN)
        {
        
            int listvertex[2048];
            
            listvertex[0]=i0;
            listvertex[1]=i1;
            int nb=2;
            
            for (n=1;n<Figure->List.Length()-1;n++)
            {
                i2=el->next->data;
                listvertex[nb++]=i2;
                el=el->next;
            }
            
            int rez=0;
        
            for (int nk=0;nk<nb-1;nk++)
            {
                CVector uu1,uu2,uu3;
                int err=0;
                
                i0=listvertex[nk];
                
                for (n=1;n<nb-1;n++)
                {
                    i1=listvertex[(nk+n)%nb];
                    i2=listvertex[(nk+n+1)%nb];

                    VECTORSUB(uu1,base_vertices[i0].Stok,base_vertices[i1].Stok);
                    if (VECTORNORM2(uu1)<SMALLF) { err=1; break; }

                    VECTORSUB(uu2,base_vertices[i1].Stok,base_vertices[i2].Stok);
                    if (VECTORNORM2(uu2)<SMALLF) { err=1; break; }

                    VECTORSUB(uu3,base_vertices[i2].Stok,base_vertices[i0].Stok);
                    if (VECTORNORM2(uu3)<SMALLF) { err=1; break; }
                    
                    VECTORNORMALISE(uu1);
                    VECTORNORMALISE(uu2);
                    VECTORNORMALISE(uu3);
                    
                    if (f_abs(DOT(uu1,uu3))>0.999f) { err=1; break; }
                    if (f_abs(DOT(uu1,uu2))>0.999f) { err=1; break; }
                    if (f_abs(DOT(uu2,uu3))>0.999f) { err=1; break; }
                }
                
                if (err==0)
                {
                    rez=nk;
                    break;
                }
            }
            
            i0=listvertex[rez];

            n=1;
            i1=listvertex[(rez+n)%nb];
            i2=listvertex[(rez+n+1)%nb];
            VECTORSUB(u1,base_vertices[i1].Stok,base_vertices[i0].Stok);

            for (n=1;n<Figure->List.Length()-1;n++)
            {
                i1=listvertex[(rez+n)%nb];
                i2=listvertex[(rez+n+1)%nb];
                
                f=&Faces[*nf];
                (*nf)++;
                
                f->n0=corres[i0];
                f->n1=corres[i1];
                f->n2=corres[i2];
                f->nT=nT;
                f->nT2=nT2;
                f->nL=nL;
                f->ref=ref;
                f->tag=TRIANGULARISATION_TAG;
                
                VECTORSUB(u2,base_vertices[i2].Stok,base_vertices[i0].Stok);
                CROSSPRODUCT(lN,u2,u1);
                
                if (VECTORNORM2(lN)>sc2)
                {
                    if (DOT(lN,N)<0)
                    {
                        bug=true;
                    }
                }
                
                u1=u2;
            }

        }
        else
        {
            for (n=1;n<Figure->List.Length()-1;n++)
            {
                i2=el->next->data;

                f=&Faces[*nf];
                (*nf)++;

                f->n0=corres[i0];
                f->n1=corres[i1];
                f->n2=corres[i2];
                f->nT=nT;
                f->nT2=nT2;
                f->nL=nL;
                f->ref=ref;
                f->tag=TRIANGULARISATION_TAG;
                
                VECTORSUB(u2,base_vertices[i2].Stok,base_vertices[i0].Stok);
                CROSSPRODUCT(lN,u2,u1);

                if (VECTORNORM2(lN)>sc2)
                {
                    if (DOT(lN,N)<0)
                    {
                        bug=true;
                    }
                }

                i1=i2;
                u1=u2;
                el=el->next;
            }
        }
    }

    free(corres);

    return (!bug);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TRIANGULARISATIONCONV TriangularisationSimplePreserve(CVertex * base_vertices,CFigure *Figure,CVector N,CShortFace *Faces,int *nv,int *nf)
{
	CVector u,U,u1,u2,lN;
	int n,nn;
	int *i0,*i1,*i2;
	CShortFace *f;
	int NBRE_ITER;
	int precedent;
	float d,ss;
	CVector vec;
	bool valid=false;
	bool valid0=false;

	SMALLF_TRIANGULARISATION=SMALLF;
	SMALLF_TRIANGULARISATION2=SMALLF2;
	SMALLF_TRIANGULARISATION3=SMALLF2;

	NBRE_ITER=0;

	SENS=SENS_CW;
	n=0;
	nn=Figure->List.Length();
	precedent=0;

	while ((nn>0)&&(NBRE_ITER<500))
	{
		i1=Figure->List[n];
		i2=Figure->List[(n+1)%Figure->List.Length()];

		vec=u=base_vertices[*i2].Stok-base_vertices[*i1].Stok;
		u.Normalise();
		U=N^u;
		U.Normalise();
		d=-(U||base_vertices[*i1].Stok);

		if ( ((TRIANGULARISATION_PlanSecant(base_vertices,Figure,U.x,U.y,U.z,d,*i1,*i2))||(precedent<0)) )
		{
			i0=Figure->List[(Figure->List.Length()+n-1)%Figure->List.Length()];
			ss=(U||base_vertices[*i0].Stok)+d;
			//if (SENS!=SENS_CW) ss=-ss;
			valid=(!TRIANGULARISATION_NouveauSegmentIntersectionnant(base_vertices,Figure,N,*i0,*i2,false));
			if ((valid)&&(ss<=0.0f))
			{
				// face i0,i1,i2
				f=&Faces[*nf];
				(*nf)++;
				f->n0=*i0;
				f->n1=*i1;
				f->n2=*i2;
				precedent=0;
				Figure->List.Del(n);
				nn=Figure->List.Length();

				if (n-1>=0) n=n-1; else n=Figure->List.Length()-1;
			}
			else 
			{ 
				n++;nn=Figure->List.Length();
				precedent=-1;
			}
		}
		else
		{
			precedent=0;
			n++;
			nn--;
		}
		if (n>=Figure->List.Length()) n=0;
		NBRE_ITER++;
	}

	if (Figure->List.Length()>2)
	{
		for (n=1;n<Figure->List.Length()-1;n++)
		{
			f=&Faces[*nf];
			(*nf)++;
			f->n0=*(Figure->List[0]);
			f->n1=*(Figure->List[n]);
			f->n2=*(Figure->List[n+1]);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool TRIANGULARISATIONCONV TriangularisationSimple2(CVertex * base_vertices,CFigure *Figure,CVector N0,CVertex *Vertices,CShortFace *Faces,int *nv,int *nf,int nT,int nT2,int nL,int nLVectors,unsigned char ref,int thetag)
{
	return TriangularisationSimple(base_vertices,Figure,N0,Vertices,Faces,nv,nf,nT,nT2,nL,ref,thetag);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool TRIANGULARISATIONCONV TriangularisationSimple(CVertex * base_vertices,CFigure *Figure,CVector N0,CVertex *Vertices,CShortFace *Faces,int *nv,int *nf,int nT,int nT2,int nL,int nLVectors,unsigned char ref)
{
	return TriangularisationSimple(base_vertices,Figure,N0,Vertices,Faces,nv,nf,nT,nT2,nL,ref,0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TRIANGULARISATIONCONV FIGURE_Recurence(CFigure *f,CVertex *vertices,int nv)
{
	int *pi;
	CVector p,u;
	int res=0;
	float SM2=SMALLF2*SMALLF2;

	VECTOR3INIT(p,vertices[nv].Stok);
	pi=f->List.GetFirst();
	while (pi)
	{
		VECTORSUB(u,vertices[*pi].Stok,p);
		if (VECTORNORM2(u)<SM2) res++;
		pi=f->List.GetNext();
	}

	pi=f->List.GetFirst();
	VECTORSUB(u,vertices[*pi].Stok,p);
	if (VECTORNORM2(u)<SM2) res++;

	return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TRIANGULARISATIONCONV FIGURE_Recurence(CFigure *f,CVertex *vertices,int nv,int nv_prev,int nv_next)
{
	CVector u1,u2;
	int res=0;

	if (FIGURE_Recurence(f,vertices,nv)>=2) res=1;
	else
	{
		VECTORSUB(u1,vertices[nv_prev].Stok,vertices[nv].Stok);
		VECTORSUB(u2,vertices[nv_next].Stok,vertices[nv].Stok);
		VECTORNORMALISE(u1);
		VECTORNORMALISE(u2);
		if (DOT(u1,u2)>1.0f-SMALLF2) res=1;
	}
	return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool TRIANGULARISATIONCONV Triangularisation0(CVertex * base_vertices,CList <CFigure> *Figures,CVector N,
    			   CVertex *Vertices,CShortFace *Faces,int *nv,int *nf,
				   int nT,int nT2,int nL,unsigned char ref,int TRIANGULARISATION_TAG)
{
	int pop;
	CFigure figure,*f,*ff,*fm;
	CVector u,U;
	int w1,w2;
	int *i1,*i2;
	float max,min,d;
	int nn,n1,n2,N0;
	bool tmp,res;
	int nb_f;
	bool bug=false;
	bool tag_=false;
	int A,B,C,D;
	float SM3=SMALLF3*SMALLF3;

	count_number_of_triangularisation++;

	SMALLF_TRIANGULARISATION=SMALLF;
	SMALLF_TRIANGULARISATION2=SMALLF2;
	SMALLF_TRIANGULARISATION3=SMALLF2;

	if (Figures->Length()==1)
	{
		tag_=false;
		if (!TriangularisationSimple(base_vertices,Figures->GetFirst(),N,Vertices,Faces,nv,nf,nT,nT2,nL,ref,TRIANGULARISATION_TAG)) bug=true;
		Figures->GetFirst()->List.Free();
		Figures->Free();
	}
	else
	if (Figures->Length()>1)
	{
		nb_f=Figures->Length();

		fm=NULL;
		max=0;
		nn=0;
		f=Figures->GetFirst();
		while (f)
		{
			CElement <int>*el=f->List.First;
			while (el)
			{
				w1=el->data;
				if (el->next) w2=el->next->data; else w2=f->List.First->data;
				VECTORSUB(u,base_vertices[w2].Stok,base_vertices[w1].Stok);
				d=VECTORNORM2(u);
				if (d>max)
				{
					fm=f;
					max=d;
					N0=nn;
				}
				el=el->next;
			}
			f=Figures->GetNext();
			nn++;
		}

		figure.List.Free();
		figure.Copy(fm);

		fm->List.Free();
		
		Figures->Del(N0);

		int NBRE_ITER=0;
		while ((Figures->Length()>0)&&(NBRE_ITER<Figures->Length()))
		{
			NBRE_ITER++;
			f=Figures->GetFirst();
			while (f)
			{
				res=false;
				min=10000000;

				int evict=1;

				CElement <int>*el=f->List.First;
				while (el)
				{
					w1=el->data;
					if (el->next) w2=el->next->data; else w2=f->List.First->data;
					VECTORSUB(u,base_vertices[w2].Stok,base_vertices[w1].Stok);
					if (VECTORNORM2(u)>SM3) evict=0;
					el=el->next;
				}

				if (evict==0)
				{
					pop=0;
					while ((pop<2)&&(!res))
					{
						CElement <int>*elfigure=figure.List.First;
						n1=0;
						while (elfigure)
						{
							w1=elfigure->data;
							bool bfw1=(FIGURE_Recurence(f,base_vertices,w1)<2);
							bool bfigurew1=(FIGURE_Recurence(&figure,base_vertices,w1)<2);

							CElement <int>*elf=f->List.First;
							n2=0;
							if (bfw1&&bfigurew1)
							while (elf)
							{
								w2=elf->data;
							
								tmp=false;

								if (pop==0) tag_=true;
								else tag_=false;
								
								ff=Figures->GetFirst2();
								while ((ff)&&(!tmp))
								{
									if (TRIANGULARISATION_NouveauSegmentIntersectionnant(base_vertices,ff,N,w1,w2,tag_)) tmp=true;
									ff=Figures->GetNext2();
								}

								if (!tmp)
								if (TRIANGULARISATION_NouveauSegmentIntersectionnant(base_vertices,&figure,N,w1,w2,tag_)) tmp=true;

								tag_=false;
								if (!tmp)
								{
									if (elfigure->prev) A=elfigure->prev->data; else A=figure.List.Closing->data;
									if (elfigure->next) D=elfigure->next->data; else D=figure.List.First->data;

									if (elf->prev) C=elf->prev->data; else C=f->List.Closing->data;
									if (elf->next) B=elf->next->data; else B=f->List.First->data;

									if ((FIGURE_Recurence(&figure,base_vertices,w2)<2)&&(FIGURE_Recurence(f,base_vertices,w2)<2))
									{
										if (!((FIGURE_Recurence(&figure,base_vertices,w1,A,D))||(FIGURE_Recurence(&figure,base_vertices,w2,C,B))))
										{
											VECTORSUB(u,base_vertices[w2].Stok,base_vertices[w1].Stok);
											d=VECTORNORM2(u);
											if (d<min)
											{
												f->a=n1;
												f->b=n2;
												min=d;
												res=true;
											}
										}
									}
								}
								n2++;
								elf=elf->next;
							}
							n1++;
							elfigure=elfigure->next;
						}
						pop++;
					}

					if (res)
					{
						i1=figure.List[f->a];
						i2=f->List[f->b];

						figure.Add(f->a,f,f->b);
						f->List.Free();
						f=Figures->DeleteAndGetNext();

						NBRE_ITER=0;
					}
					else
					{
						f=Figures->GetNext();
					}
				}
				else
				{
					f->List.Free();
					f=Figures->DeleteAndGetNext();
				}
			}
		}

		tag_=false;
		if (!TriangularisationSimple(base_vertices,&figure,N,Vertices,Faces,nv,nf,nT,nT2,nL,ref,TRIANGULARISATION_TAG)) bug=true;

		figure.List.Free();

		if (Figures->Length()>0)
		{
			f=Figures->GetFirst();
			while (f)
			{
				f->List.Free();
				f=Figures->GetNext();
			}
			Figures->Free();
		}
	}

	return (!bug);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool TRIANGULARISATIONCONV Triangularisation(CVertex * base_vertices,CList <CFigure> *Figures,CVector N,CVertex *Vertices,CShortFace *Faces,int *nv,int *nf,int nT,int nT2,int nL,unsigned char ref)
{
	return Triangularisation0(base_vertices,Figures,N,Vertices,Faces,nv,nf,nT,nT2,nL,ref,0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool TRIANGULARISATIONCONV Triangularisation2(CVertex * base_vertices,CList <CFigure> *Figures,CVector N,CVertex *Vertices,CShortFace *Faces,int *nv,int *nf,int nT,int nT2,int nL,int nLVectors,unsigned char ref,int thetag)
{
	return Triangularisation0(base_vertices,Figures,N,Vertices,Faces,nv,nf,nT,nT2,nL,ref,thetag);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool TRIANGULARISATIONCONV TRIANGULARISATION_PlanSecant2(CVertex * base_vertices,CFigure *Figure,CVector N,int n1,int n2)
{
	float s;
	int *i;
	int cp,cm;
	CVector m1,m2;
	float a,b,c,d;

	VECTORSUB(m1,base_vertices[n2].Stok,base_vertices[n1].Stok);
	CROSSPRODUCT(m2,N,m1);
	VECTORNORMALISE(m2);

	a=m2.x;
	b=m2.y;
	c=m2.z;
	d=-DOT(m2,base_vertices[n2].Stok);

	cp=cm=0;
	i=Figure->List.GetFirst();
	while (i)
	{
		if (((*i)!=n1)&&((*i)!=n2))
		{
			s=a*base_vertices[*i].Stok.x +b*base_vertices[*i].Stok.y +c*base_vertices[*i].Stok.z +d;
			if (s>SMALLF_TRIANGULARISATION) cp++;
			if (s<-SMALLF_TRIANGULARISATION) cm++;
		}
		i=Figure->List.GetNext();
	}

	return ((cp!=0)&&(cm!=0));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
