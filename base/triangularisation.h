
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
  
#ifndef _TRIANGULARISATION_H_
#define _TRIANGULARISATION_H_

#ifdef WIN32
#define TRIANGULARISATIONCONV __fastcall
#else
#define TRIANGULARISATIONCONV
#endif

#define TRIANGULARISATION
bool TRIANGULARISATIONCONV TriangularisationSimple(CVertex * base_vertices,CFigure *Figure,CVector N,CVertex *Vertices,CShortFace *Faces,int *nv,int *nf,int nT,int nT2,int nL,unsigned char ref);
bool TRIANGULARISATIONCONV TriangularisationSimple(CVertex * base_vertices,CFigure *Figure,CVector N,CVertex *Vertices,CShortFace *Faces,int *nv,int *nf,int nT,int nT2,int nL,unsigned char ref,int TRIANGULARISATION_TAG);
bool TRIANGULARISATIONCONV TriangularisationSimpleTag(CVertex * base_vertices,CFigure *Figure,CVector N,CVertex *Vertices,CShortFace *Faces,int *nv,int *nf,int nT,int nT2,int nL,unsigned char ref,int TRIANGULARISATION_TAG);
bool TRIANGULARISATIONCONV TriangularisationSimple2(CVertex * base_vertices,CFigure *Figure,CVector N0,CVertex *Vertices,CShortFace *Faces,int *nv,int *nf,int nT,int nT2,int nL,int nLVectors,unsigned char ref,int thetag);
bool TRIANGULARISATIONCONV Triangularisation(CVertex * base_vertices,CList <CFigure> *Figures,CVector N,CVertex *Vertices,CShortFace *Faces,int *nv,int *nf,int nT,int nT2,int nL,unsigned char ref);
bool TRIANGULARISATIONCONV Triangularisation2(CVertex * base_vertices,CList <CFigure> *Figures,CVector N,CVertex *Vertices,CShortFace *Faces,int *nv,int *nf,int nT,int nT2,int nL,int nLVectors,unsigned char ref,int thetag);
bool TRIANGULARISATIONCONV TRIANGULARISATION_NouveauSegmentIntersectionnant2(CVector A,CVector B,CVertex * base_vertices,CFigure *Figure,CVector N,int nn1,int nn2);
bool TRIANGULARISATIONCONV TRIANGULARISATION_PlanSecant2(CVertex * base_vertices,CFigure *Figure,CVector N,int n1,int n2);
void TRIANGULARISATIONCONV TriangularisationSimplePreserve(CVertex * base_vertices,CFigure *Figure,CVector N,CShortFace *Faces,int *nv,int *nf);

#endif
