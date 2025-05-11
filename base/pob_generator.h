
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
  
#ifndef _POB_GENERATOR_H_
#define _POB_GENERATOR_H_

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class API3D_STUB CPOBGenerator
{
public:
	int NITERATIONS;
	int nPOB;

    CPOBGenerator() { NITERATIONS=0; nPOB=0; }


	CMatrix Jacobi(CMatrix M0);
    CMatrix JacobiNew(CMatrix M0);
	CPOB * POBFromEvictNT(CObject3D * obj,int nt);
	CPOB * POBFrom2(CObject3D * obj,int tag,int NITER);
    CPOB * POBFrom2zero(CObject3D * obj,int tag,int NITER);
    CPOB * POBFrom2F(CObject3D * obj,int tag,int NITER);
	CPOB * POBFrom1(CObject3D * obj,int tag,int NITER);
	CPOB * POBFrom(CObject3D * obj,int NITER);
	CPOB * POBFromPoints(CObject3D * obj,int tag);
    CPOB * POBFrom2Topo(CObject3D * obj,int tag);

};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#endif
