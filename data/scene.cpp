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

////////////////////////////////////////////////////////////////////////
//	@file scene.cpp
//	@date 2004
////////////////////////////////////////////////////////////////////////



#include "scene.h"
#include <math.h>
#include "png.h"

extern bool LOGING;
extern void LIB3DLog(char *str);
#define LOG(str) { if (LOGING) LIB3DLog(str); }

#include "datacontainer.h"

CDataReader BASIC3DREAD;


void BASIC3D_Mesh::SetKeys()
{
	int n,N,nn;
	CQuaternion tmpq,*tq,*tq1,*tq2,Mq;
	CVector tmpp,*tp,*tp1,*tp2;
	int array[2048];
	BASIC3D_Keyframe *key;
	int len;

	if (keys.nb>0)
	{
		len=keys.nb;

		array[0]=fend;
		n=0;
		key=keys.GetFirst();
		while (key)
		{
			if (key->q)
			{
				array[n]=keys.GetId();
				n++;
			}
			key=keys.GetNext();
		}
		N=n;


		if (N>0)
		{
			for (n=0;n<N-1;n++)
			{
				for (nn=array[n]+1;nn<array[n+1];nn++)
				{
					tq=keys[nn]->q=new CQuaternion;

					tq1=keys[array[n]]->q;
					tq2=keys[array[n+1]]->q;

					(*tq)=(*tq1) + ((float)(nn-array[n]))*((*tq2)-(*tq1))/((float)(array[n+1]-array[n]));
					tq->Normalise();
				}
			}
		}

		array[0]=fend;
		n=0;
		key=keys.GetFirst();
		while (key)
		{
			if (key->p)
			{
				array[n]=keys.GetId();
				n++;
			}
			key=keys.GetNext();
		}
		N=n;

		if (N>0)
		{
			for (n=0;n<N-1;n++)
			{
				for (nn=array[n]+1;nn<array[n+1];nn++)
				{
					tp=keys[nn]->p=new CVector;

					tp1=keys[array[n]]->p;
					tp2=keys[array[n+1]]->p;

					(*tp)=(*tp1) + ((float)(nn-array[n]))*((*tp2)-(*tp1))/((float)(array[n+1]-array[n]));
				}
			}
		}
	}
}


void BASIC3D::ComputeFrame(int frame,float t)
{	
	CMatrix itM,tM,tMp,tMo,iM,iMo,M,Mo,Mp,Mpo;
	BASIC3D_Mesh * m;
	CQuaternion q,q1,q2;
	CVector p,p1,p2;
	int n,nn;
	CVector vs[16];
	CVector Ns[16];
	CVector v,N,u;
	int nbones;
	float w,tt,sum;
	CVector offset;

	
	m=Meshes.GetFirst();
	while (m)
	{
		m->tag=0;

		if ((m->keys.nb>1)&&(frame>0))
		{
			tt=t;

			p1=*(m->keys((frame%(m->keys.nb)))->p);
			p2=*(m->keys((frame%(m->keys.nb)))->p);
			q1=*(m->keys((frame%(m->keys.nb)))->q);
			q2=*(m->keys(((frame+1)%(m->keys.nb)))->q);
			
			p=(1-tt)*p1+tt*p2;
			q=(1-tt)*q1+tt*q2;
			q.Normalise();

			q.Id();

			M.Id();
			M.OrientationQuaternion(q);
			M.Translation(p.x,p.y,p.z);

			Mo.Id();
			Mo.OrientationQuaternion(q);
		}
		else
		{
			Mo.Id();
			M.Id();
		}

		iM.Id();
		iM.OrientationQuaternion(m->q);
		iM.Translation(m->p.x,m->p.y,m->p.z);

		iMo.Id();
		iMo.OrientationQuaternion(m->q);


		Mp.Id();
		Mp.Translation(m->pivot_p.x,m->pivot_p.y,m->pivot_p.z);
		//Mp.OrientationQuaternion(m->pivot_q);
		

		Mpo.Id();
		Mpo.OrientationQuaternion(m->pivot_q);

		m->M=M;
		m->Mo=Mo;
		m->Mp=Mp;
		m->Mpo=Mpo;
		m->iM=iM;
		m->iMo=iMo;
		m->fM.Id();
		m->fMo.Id();

		m=Meshes.GetNext();
	}
	
	for (n=0;n<64;n++)
	{
		m=Meshes.GetFirst();
		while (m)
		{
			if (m->tag==0)
			{
				if (n==0)
				{
					if ((m->ParentID==0)||(m->ParentID==-1))
					{
						m->fRoot=m->M;
						m->fM=m->M;
						m->ifM=m->iM;
						m->fMo=m->Mo;
						m->tag=1;
					}
				}
				else
				{
					if (Meshes[m->ParentID]->tag==n)
					{
						m->fRoot=Meshes[m->ParentID]->fRoot;

						if (m->attr&SKELETON)
						{
							Mp=m->Mpo*m->Mp;
							tMp.Inverse4x4(Mp);
							m->fM=Mp*Meshes[m->ParentID]->fM;
							m->fM=m->M*m->fM;
							m->fM=tMp*m->fM;
						
						}
						else
						{
							Mp=m->Mp;
							tMp.Inverse4x4(Mp);
							if (m->ParentID>=0)
							{
								m->ifM=(m->iM*Meshes[m->ParentID]->ifM);
								m->fM=Mp*Meshes[m->ParentID]->fM;
								m->fM=m->M*m->fM;
								m->fM=tMp*m->fM;
								m->fM=m->ifM*m->fM;
							}
							else 
							{
								m->fM=m->M;
								m->ifM=m->iM;
								m->fMo=m->Mo;
							}
							//m->fM=m->ifM*tMp*m->M*Mp*Meshes[m->ParentID]->fM;
							
							
						}

			
						m->tag=n+1;
					}
				}
			}

			m=Meshes.GetNext();
		}

	}
/**/

	m=Meshes.GetFirst();
	while (m)
	{
		if (m->obj)
		{
			if (m->attr&SKIN)
			{
				for (n=0;n<m->obj->nVertices;n++)
				{
					nbones=m->obj->VerticesAdd[n].nbones;
					for (nn=0;nn<nbones;nn++)
					{
						vs[nn]=m->obj->VerticesAdd[n].offsets[nn]*Meshes[m->obj->VerticesAdd[n].bonesID[nn]]->fM;
						Ns[nn]=m->obj->Vertices[n].Norm*Meshes[m->obj->VerticesAdd[n].bonesID[nn]]->fMo;
					}

					sum=0;
					v.Init(0,0,0);
					N.Init(0,0,0);
					for (nn=0;nn<nbones;nn++)
					{
						w=m->obj->VerticesAdd[n].weigths[nn];
						sum+=w;
						v+=vs[nn]*w;
						N+=Ns[nn]*w;
					}

					v/=sum;
					N/=sum;

					m->obj->Vertices[n].Calc=v;
					m->obj->Vertices[n].NormCalc=N;

				}

				M.Id();
				m->WM.CFrame(M);
				m->WMo.CFrame(M);
			}
			else/**/
			{
				if (frame>0)
				{
					m->WM.CFrame(m->fM);
					m->WMo.CFrame(m->fMo);
				}
				else
				{
					m->WM.CFrame(m->ifM);
				}
			}
		
		}
		m=Meshes.GetNext();
	}

}


void BASIC3D::CalculateVertexBuffers(unsigned int flags)
{
	BASIC3D_Mesh * m;

	
	ComputeFrame(-1,0);

	m=Meshes.GetFirst();
	while (m)
	{
		if (m->obj) 
		{
			if (m->env_mapping)
				m->vb=api->CreateMultiGroupFrom(m->obj,flags|API3D_ENABLEUPDATE|API3D_ENVMAPPING);
			else
				m->vb=api->CreateMultiGroupFrom(m->obj,flags|API3D_ENABLEUPDATE);
		}
		m=Meshes.GetNext();
	}
}


void BASIC3D::UpdateVertexBuffers()
{
	BASIC3D_Mesh * m;
	int n;

	m=Meshes.GetFirst();
	while (m)
	{
		if ((m->obj)&&(m->attr&SKIN))
		{
			m->vb->LockVertices();
			for (n=0;n<(int)m->vb->nVerticesActif;n++)
				m->vb->SetVertex(n,m->obj->Vertices[m->vb->CorresVertices[n]].Calc);

			m->vb->UnlockVertices();
		}
		m=Meshes.GetNext();
	}
}


void BASIC3D::FreeVertexBuffers()
{
	BASIC3D_Mesh * m;

	m=Meshes.GetFirst();
	while (m)
	{
		if (m->vb)
		{
			m->vb->Release();
			delete m->vb;
			m->vb=NULL;
		}

		m=Meshes.GetNext();
	}
}

extern bool LOGING;
extern void LIB3DLog(const char *str);
#define LOG(str) { if (LOGING) LIB3DLog(str); }


void BASIC3D::Free()
{
	BASIC3D_Mesh * m;

	m=Meshes.GetFirst();
	while (m)
	{
		if (m->animkeys)
		{
			delete [] m->animkeys;
			m->animkeys=NULL;
			m->nanimkeys=0;
		}

		if (m->obj) 
		{
			m->obj->Free();
			delete (m->obj);
			m->obj=NULL;
		}

		if (m->vb) 
		{
			m->vb->Release();
			delete (m->vb);
			m->vb=NULL;
		}

		if (m->edvb)
		{
			m->edvb->Release();
			delete (m->edvb);
			m->edvb=NULL;
		}

		m->limbs.Free();
		
		m=Meshes.GetNext();
	}

	Limbs.Free();
	Meshes.Free();
	Materials.Free();
	Textures.Free();
}

void BASIC3D::SetKeyframer()
{
	BASIC3D_Mesh *m;

	m=Meshes.GetFirst();
	while (m)
	{
		m->SetKeys();
		m=Meshes.GetNext();
	}
}

void BASIC3D_Mesh::Affect()
{
	float x,y,z;
	float sx,sy,sz;
	float qw,qx,qy,qz;
	int pid;

	name=BASIC3DREAD.readString();

	pid=BASIC3DREAD.readInt();
	ParentID=pid;

	x=BASIC3DREAD.readFloat();
	y=BASIC3DREAD.readFloat();
	z=BASIC3DREAD.readFloat();

	qx=BASIC3DREAD.readFloat();
	qy=BASIC3DREAD.readFloat();
	qz=BASIC3DREAD.readFloat();
	qw=BASIC3DREAD.readFloat();

	sx=BASIC3DREAD.readFloat();
	sy=BASIC3DREAD.readFloat();
	sz=BASIC3DREAD.readFloat();


	p.Init(x,y,z);
	q.Init(qw,qx,qy,qz);
	q.Normalise();
	s.Init(sx,sy,sz);

	x=BASIC3DREAD.readFloat();
	y=BASIC3DREAD.readFloat();
	z=BASIC3DREAD.readFloat();

	qx=BASIC3DREAD.readFloat();
	qy=BASIC3DREAD.readFloat();
	qz=BASIC3DREAD.readFloat();
	qw=BASIC3DREAD.readFloat();

	sx=BASIC3DREAD.readFloat();
	sy=BASIC3DREAD.readFloat();
	sz=BASIC3DREAD.readFloat();

	pivot_p.Init(x,y,z);
	pivot_q.Init(qw,qx,qy,qz);
	pivot_q.Normalise();
	pivot_s.Init(sx,sy,sz);

}

void BASIC3D::UndoStuffs()
{
	api->SetTextureGroup(groupname);
	api->FreeAllTextures();
	api->SetTextureGroup(NULL);
}

extern int FichierExistant(char * str);

void BASIC3D::SetFlag(int n,int value)
{
	flags[n]=value;
}

void BASIC3D::DoStuffs(int nt_base,char * repertoire)
{
	char ss[128];

	sprintf(ss,"BASIC3D_GROUP_%d",nt_base);
	DoStuffs(ss,repertoire);
}

void BASIC3D::DoStuffs(char *group,char * repertoire)
{
	BASIC3D_Mesh *m;
	int n,nn;
	int nt=0;
	float u,v,uu,vv;
	CMatrix M;
	int tex;
	BASIC3D_TEXTURE *T;
	struct IMAGE_DATAS im;
	char name[512];
	char ss[512];
	char ptr[32*32*4];

	sprintf(groupname,"%s",group);
	api->SetTextureGroup(groupname);

	if (flags[LOAD_TEXTURES])
	{
		// load textures
		
		api->SetParams(API3D_CREATETEXTURE,MIPMAP);

		for (n=0;n<32*32*4;n++) ptr[n]=(char) 255;
		blanc=api->AddTexture(32,ptr,32,NO_ALPHA);

		for (n=0;n<32*32*4;n++) ptr[n]=0;

		n=1;
		T=Textures.GetFirst();
		while (T)
		{
			sprintf(ss,"%s",T->filename);
			nn=strlen(ss)-1;
			while ((!((ss[nn]==92)||(ss[nn]=='/')))&&(nn>=0)) nn--;
			sprintf(name,"%s%s",repertoire,&ss[nn+1]);

			LOG(name);
			if (FichierExistant(name))
			{
				//if (T->TileU&CLAMP) api->SetParams(API3D_CREATETEXTURE,CLAMP);
				api->SetParams(API3D_CREATETEXTURE,REPEAT);

				READ_PNG(name,&im,ALPHA1);
		
				if (im.Alpha==1)
				{
					nt=api->AddTexture(im.X,im.ptrImageDATAS,im.X,ALPHA);
					T->Alpha=true;
				}
				else
				{
					nt=api->AddTexture(im.X,im.ptrImageDATAS,im.X,NO_ALPHA);
					T->Alpha=false;
				}
				
				free(im.ptrImageDATAS);
			}
			else
			{
				nt=api->AddTexture(32,ptr,32,NO_ALPHA);
			}
			T->local_nt=nt;
			T=Textures.GetNext();
		}

	}
	else blanc=0;

	// set material values to textures

	if (flags[APPLY_MATERIAL_COEFFICIENT])
	{
		BASIC3D_MATERIAL * mat;
		for (n=0;n<32*32*4;n++) ptr[n]=(char) 255;

		mat=Materials.GetFirst();
		while (mat)
		{
			if ((mat->diffuseID==-1)&&(mat->specularID==-1))
			{
				nt=api->AddTexture(32,ptr,32,NO_ALPHA);
				api->Material(nt)->setAmbient(mat->ar,mat->ag,mat->ab,1);
				api->Material(nt)->setDiffuse(mat->r,mat->g,mat->b,1);
				api->Material(nt)->setSpecular(mat->sr,mat->sg,mat->sb,1);
				mat->diffuseID=nt;
			}
			else
			{
				tex=mat->diffuseID;
				if (tex>=0)
				{
					T=Textures[tex];
					api->Material(nt)->setAmbient(mat->ar,mat->ag,mat->ab,1);
					api->Material(nt)->setDiffuse(mat->r,mat->g,mat->b,1);
					api->Material(nt)->setSpecular(mat->sr,mat->sg,mat->sb,1);
				}
			}
			mat=Materials.GetNext();
		}
	}

	// apply texture transform
	if (flags[APPLY_TEXTURE_TRANSFORM])
	{	
		m=Meshes.GetFirst();
		while (m)
		{
			if (m->obj)
			{
				for (n=0;n<m->obj->nVertices;n++)
					m->obj->Vertices[n].tag=-1;

				for (n=0;n<m->obj->nFaces;n++)
				{
					nt=m->obj->Faces[n].nT;

					if (Materials[nt])
					{
						tex=Materials[nt]->diffuseID;
						if (tex>=0)
						{
							if (Textures[tex])
							{
								M=Textures[tex]->Transform;

								if (Textures[tex]->Alpha) m->obj->Faces[n].ref=1;
								else m->obj->Faces[n].ref=0;

								m->obj->Faces[n].v[0]->tag=tex;
								m->obj->Faces[n].v[1]->tag=tex;
								m->obj->Faces[n].v[2]->tag=tex;

								if (flags[LOAD_TEXTURES]) m->obj->Faces[n].nT=Textures[tex]->local_nt;
							}

						}
						else
						{
							m->obj->Faces[n].ref=0;

							m->obj->Faces[n].v[0]->tag=-1;
							m->obj->Faces[n].v[1]->tag=-1;
							m->obj->Faces[n].v[2]->tag=-1;

							if (flags[LOAD_TEXTURES]) m->obj->Faces[n].nT=blanc;
						}

						if (Materials[nt]->specularID>=0)
						{
							tex=Materials[nt]->specularID;
							if (flags[LOAD_TEXTURES]) m->obj->Faces[n].nT2=Textures[tex]->local_nt;
							m->env_mapping=true;
						}
					}
					
				}

				for (n=0;n<m->obj->nVertices;n++)
				{
					tex=m->obj->Vertices[n].tag;

					if (tex>=0)
					{
						if (Textures[tex])
						{
							M=Textures[tex]->Transform;

							uu=m->obj->Vertices[n].Map.x;
							vv=m->obj->Vertices[n].Map.y;

							u=uu*M.a[0][0] + vv*M.a[1][0];
							v=uu*M.a[0][1] + vv*M.a[1][1];

							m->obj->Vertices[n].Map.Init(u,1-v);
						}
					}
				}
			}
			m=Meshes.GetNext();
		}
	}

	api->SetTextureGroup(NULL);
	/**/
}

void BASIC3D::Save(char *name)
{
	Save(name,0);
}

int anim_stringlen(CObject3D *obj);


void BASIC3D::Save(char *name,int flags)
{
	const char *id_str="XXX3Dv1.1";
	const char *hdr_str="HDR_";
	CDataContainer DATA;
	BASIC3D_HDR hdr;
	BASIC3D_MATERIAL *mat;
	BASIC3D_TEXTURE *tex;
	BASIC3D_Mesh *mesh;
	CVector p,min,max,delta;
	FILE *f;
	int zero=0;
	int len,lenC;
	int n,k;
	CObject3D *obj;
	unsigned int val;
	unsigned int ux,uy,uz;

	hdr.nanims=0;
	hdr.ncams=0;
	hdr.nlights=0;
	hdr.nmat=Materials.Length();
	hdr.nobjs=Meshes.Length();
	hdr.ntex=Textures.Length();

	if ((flags&SAVE3D_ANIMONLY)==0)
	{
		mesh=Meshes.GetFirst();
		while (mesh)
		{
			obj=mesh->obj;

			DATA.storeData("MESH",4);

			DATA.storeInt(mesh->attr);
			DATA.storeInt(Meshes.GetId());

			DATA.storeString(mesh->name);
			DATA.storeInt(mesh->ParentID);

			DATA.storeVector(mesh->p);
			DATA.storeFloat(mesh->q.n.x);
			DATA.storeFloat(mesh->q.n.y);
			DATA.storeFloat(mesh->q.n.z);
			DATA.storeFloat(mesh->q.s);
			DATA.storeVector(mesh->s);

			DATA.storeVector(mesh->pivot_p);
			DATA.storeFloat(mesh->pivot_q.n.x);
			DATA.storeFloat(mesh->pivot_q.n.y);
			DATA.storeFloat(mesh->pivot_q.n.z);
			DATA.storeFloat(mesh->pivot_q.s);
			DATA.storeVector(mesh->pivot_s);

			DATA.storeInt(obj->nVertices);
			DATA.storeInt(obj->nFaces);

			for (n=0;n<obj->nVertices;n++)
			{
				DATA.storeVector(obj->Vertices[n].Stok);
				DATA.storeMap(obj->Vertices[n].Map);
			}

			for (n=0;n<obj->nFaces;n++)
			{
				DATA.storeInt(obj->Faces[n].v0);
				DATA.storeInt(obj->Faces[n].v1);
				DATA.storeInt(obj->Faces[n].v2);
				DATA.storeInt(0);
				DATA.storeInt(obj->Faces[n].nT);
			}
			mesh=Meshes.GetNext();
		}
	}

	if ((flags&SAVE3D_NOANIMS)==0)
	{
		mesh=Meshes.GetFirst();
		while (mesh)
		{
			obj=mesh->obj;
			if (obj->nKeys>0)
			{
				DATA.storeData("ANI2",4);
				DATA.storeInt(Meshes.GetId());
				DATA.storeInt(obj->nKeys);
				DATA.storeInt(0);
				DATA.storeInt(MORPH);
				DATA.storeInt(anim_stringlen(obj));
				DATA.storeInt(obj->nVertices);
				if (obj->anim_strings) DATA.storeData(obj->anim_strings,anim_stringlen(obj));

				min.Init(100000,100000,100000);
				max.Init(-100000,-100000,-100000);

				for (k=0;k<obj->nKeys;k++)
					for (n=0;n<obj->nVertices;n++) 
					{
						p=obj->VerticesKeys[k][n].Stok;
						if (p.x>max.x) max.x=p.x;
						if (p.y>max.y) max.y=p.y;
						if (p.z>max.z) max.z=p.z;
						if (p.x<min.x) min.x=p.x;
						if (p.y<min.y) min.y=p.y;
						if (p.z<min.z) min.z=p.z;	
					}

				delta=max-min;
				min-=0.005f*delta;
				max+=0.005f*delta;

				DATA.storeVector(min);
				DATA.storeVector(max);

				for (k=0;k<obj->nKeys;k++)
					for (n=0;n<obj->nVertices;n++) 
					{
						p=obj->VerticesKeys[k][n].Stok;
						p.x = (p.x - min.x) / (max.x - min.x);
						p.y = (p.y - min.y) / (max.y - min.y);
						p.z = (p.z - min.z) / (max.z - min.z);

						ux=(unsigned int) (((1 << 10)-1)*p.x);
						uy=(unsigned int) (((1 << 10)-1)*p.y);
						uz=(unsigned int) (((1 << 10)-1)*p.z);
						val=(ux<<20)+(uy<<10)+uz;
						DATA.storeUInt(val);

					}
			}
			mesh=Meshes.GetNext();
		}

		/*
			mesh=Meshes.GetFirst();
			while (mesh)
			{
				obj=mesh->obj;
				if (obj->nKeys>0)
				{
					DATA.storeData("ANI3",4);
					DATA.storeInt(Meshes.GetId());
					DATA.storeInt(obj->nKeys);
					DATA.storeInt(0);
					DATA.storeInt(MORPH);
					DATA.storeInt(anim_stringlen(obj));
					DATA.storeInt(obj->nVertices);
					if (obj->anim_strings) DATA.storeData(obj->anim_strings,anim_stringlen(obj));

					for (k=0;k<obj->nKeys;k++)
						for (n=0;n<obj->nVertices;n++) 
						{
							p=obj->VerticesKeys[k][n].Stok;
							DATA.storeFloat(p.x);
							DATA.storeFloat(p.y);
							DATA.storeFloat(p.z);
						}
				}
				mesh=Meshes.GetNext();
			}
		/**/
	}

	if ((flags&SAVE3D_ANIMONLY)==0)
	{
		tex=Textures.GetFirst();
		while (tex)
		{
			DATA.storeData("TEX_",4);
			DATA.storeInt(tex->texID);
			DATA.storeString(tex->filename);
			tex=Textures.GetNext();
		}

		mat=Materials.GetFirst();
		while (mat)
		{
			DATA.storeData("MAT_",4);
			DATA.storeData((char*) mat,sizeof(BASIC3D_MATERIAL));
			mat=Materials.GetNext();
		}
	}

	if (flags&SAVE3D_LIGHTCOMPRESS) DATA.compressFast();
	else DATA.compress();

	len=DATA.getLength();
	lenC=DATA.getLZLength();

	f=fopen(name,"wb");
	if(f==NULL) {
		return;
	}
	fwrite(id_str,9,1,f);
	fwrite(hdr_str,4,1,f);
	fwrite(&hdr,sizeof(BASIC3D_HDR),1,f);
	fwrite(&lenC,4,1,f);
	fwrite(&len,4,1,f);
	fwrite(DATA.getLZData(),lenC,1,f);
	fwrite(&zero,4,1,f);
	fwrite(&zero,4,1,f);
	fclose(f);

	DATA.clean();
}


typedef struct
{
	unsigned short int tag;
	unsigned int size;

} chunk3ds;

/*
{0x4D4D,"Main chunk",&Nothing,&Nothing},
	{0x3D3D,"Chunk 3D",&Nothing,&Nothing},
		{0x4000,"Object",&ObjectReader,&ObjectReader},
		
			{0x4100,"Mesh",&MeshReader,&CountObject},
				{0x4110,"Vertices",&VertexReader,&CountVertices},
				{0x4140,"Mapping Coo.",&MappingReader,NULL},
				{0x4120,"Faces",&FacesReader,&CountFaces},
				{0x4160,"Local coordinate system",&Coordinates,NULL},
				{0x4130,"Faces Material List",&MaterialList,NULL},
*/

void writechunk(FILE *f,chunk3ds *c)
{
    fwrite(&c->tag,2,1,f);
    fwrite(&c->size,4,1,f);
}

void BASIC3D::Save3DS(char *name)
{
#ifndef GLES
	int count=0;
	CMatrix M;
	char bla[8];
	char ss[16];
	int n;
	unsigned short int nv,nf;
	unsigned short int is[4];
	float xyz[3];
	BASIC3D_Mesh *mesh;
	CObject3D *obj;
	chunk3ds c;
	FILE *f;
	float one=1.0f;
	float zero=0.0f;
	int sz=0;
	CVector v;

	f=fopen(name,"wb");
	if(f==NULL) {
		return;
	}

	sz=0;
	mesh=Meshes.GetFirst();
	while (mesh)
	{
		obj=mesh->obj;
		nv=obj->nVertices;
		nf=obj->nFaces;
		sz+=7 + 6 + 6+ 2+6+(int)nv*3*4 +2+6+(int)nv*2*4 +2+6+(int)nf*2*4 + 6+9*4+3*4;
		mesh=Meshes.GetNext();
	}

	c.tag=0x4D4D;
	c.size=6 + 6 +sz;
	writechunk(f,&c);

	c.tag=0x3D3D;
	c.size=6 + sz;
	writechunk(f,&c);


	mesh=Meshes.GetFirst();
	while (mesh)
	{
		obj=mesh->obj;

		nv=obj->nVertices;
		nf=obj->nFaces;

		sprintf(ss,"%d",count);
		for (n=0;n<(int) (6-strlen(ss));n++) bla[n]='X';
		memcpy(&bla[n],ss,strlen(ss));
		bla[7]='\0';
		count++;

		int size_obj=2+6+(int)nv*3*4 +2+6+(int)nv*2*4 +2+6+(int)nf*2*4 + 6+9*4+3*4;

		c.tag=0x4000;
		c.size=7+6+ 6+size_obj;
		writechunk(f,&c);
		fwrite(bla,7,1,f);

		c.tag=0x4100;
		c.size=6+size_obj;
		writechunk(f,&c);

		// vertices chunk
		c.tag=0x4110;
		c.size=6+2+(int)nv*3*4;
		writechunk(f,&c);
		fwrite(&nv,2,1,f);

		M.Id();
        M.Translation(mesh->p.x,mesh->p.y,mesh->p.z);

		for (n=0;n<obj->nVertices;n++) 
		{
			v.Init(obj->Vertices[n].Stok.x,obj->Vertices[n].Stok.y,obj->Vertices[n].Stok.z);
			v=v*M;

			xyz[0]=v.x;
			xyz[1]=-v.z;
			xyz[2]=-v.y;
			fwrite(xyz,4*3,1,f);
		}

		// mapping vertices chunk
		c.tag=0x4140;
		c.size=6+2+(int)nv*2*4;
		writechunk(f,&c);
		fwrite(&nv,2,1,f);
		for (n=0;n<obj->nVertices;n++) 
		{
			xyz[0]=obj->Vertices[n].Map.x;
			xyz[1]=1.0f-obj->Vertices[n].Map.y;
			fwrite(xyz,4*2,1,f);
		}

		c.tag=0x4120;
		c.size=6+2+(int)nf*2*4;
		writechunk(f,&c);
		fwrite(&nf,2,1,f);
		for (n=0;n<obj->nFaces;n++)
		{
			is[0]=obj->Faces[n].v0;
			is[1]=obj->Faces[n].v1;
			is[2]=obj->Faces[n].v2;
			is[3]=0xFFFF;
			fwrite(is,4*2,1,f);
		}

		c.tag=0x4160;
		c.size=6+4*9+3*4;
		writechunk(f,&c);

		// orientation
		fwrite(&one,4,1,f);
		fwrite(&zero,4,1,f);
		fwrite(&zero,4,1,f);

		fwrite(&zero,4,1,f);
		fwrite(&one,4,1,f);
		fwrite(&zero,4,1,f);

		fwrite(&zero,4,1,f);
		fwrite(&zero,4,1,f);
		fwrite(&one,4,1,f);

		// translation
		fwrite(&zero,4,1,f);
		fwrite(&zero,4,1,f);
		fwrite(&zero,4,1,f);

		mesh=Meshes.GetNext();
	}

	c.tag=0xFFFF;
	c.size=0;
	writechunk(f,&c);

	fclose(f);
#endif
}

char BASIC3D_temp_str[64];

char * spaces_number(int n)
{
	char *ss=BASIC3D_temp_str;

	if (n<10)    sprintf(ss,"   %d",n);
	else
	if (n<100)   sprintf(ss,"  %d",n);
	else
	if (n<1000)  sprintf(ss," %d",n);
	else
	if (n<10000) sprintf(ss,"%d",n);

	return ss;
}

void BASIC3D::SaveASC(char *name)
{
#ifndef GLES
	BASIC3D_Mesh *mesh;
	FILE *f;
	int zero=0;
	int n;
	CObject3D *obj;


	f=fopen(name,"w");
	if(f==NULL) {
		return;
	}

	mesh=Meshes.GetFirst();
	while (mesh)
	{
		obj=mesh->obj;

		fprintf(f,"Named object: %c%s%c\n",'"',mesh->name,'"');
		fprintf(f,"Tri-mesh, Vertices: %d     Faces: %d\n",obj->nVertices,obj->nFaces);

		fprintf(f,"Vertex list\n");
		for (n=0;n<obj->nVertices;n++)
		{
			fprintf(f,"Vertex %s:\t X:%6.6f\t Y:%6.6f\t Z:%6.6f\t U:%6.6f\t V:%6.6f\t\n",spaces_number(n),
					obj->Vertices[n].Stok.x,obj->Vertices[n].Stok.y,obj->Vertices[n].Stok.z,
					obj->Vertices[n].Map.x,obj->Vertices[n].Map.y);

		}

		fprintf(f,"Face list\n");
		for (n=0;n<obj->nFaces;n++)
		{
			fprintf(f,"Face %s:\t A:%d\t B:%d\t C:%d\t AB:1\t BC:1\t CA:1\n",spaces_number(n),obj->Faces[n].v0,obj->Faces[n].v1,obj->Faces[n].v2);
		}

		fprintf(f,"\n\n");

		mesh=Meshes.GetNext();
	}

	fclose(f);
#endif
}


void BASIC3D::SaveX(char *name)
{
#ifndef GLES
	BASIC3D_Mesh *mesh;
	char mname[512];
	CMatrix M;
	FILE *f;
	int zero=0;
	int n;
	CObject3D *obj;

	f=fopen(name,"w");
	if(f==NULL) {
		return;
	}

	fprintf(f,"xof 0303txt 0032%c",10);
	fprintf(f,"%c",10);

	fprintf(f,"Frame SCENE_ROOT {%c",10);	

	fprintf(f,"%c",10);

	fprintf(f,"\tFrameTransformMatrix {%c",10);
	fprintf(f,"\t1.000,0.000,0.000,0.000,0.000,1.000,0.000,0.000,0.000,0.000,1.000,0.000,0.000,0.000,0.000,1.000;;%c",10);
	fprintf(f,"\t}%c",10);

	fprintf(f,"%c",10);

	mesh=Meshes.GetFirst();
	while (mesh)
	{
		obj=mesh->obj;

		M.Id();
		M.RotationDegre(obj->Rot.x,obj->Rot.y,obj->Rot.z);
		M.Translation(obj->Coo.x,obj->Coo.y,obj->Coo.z);

		//obj->InverseOrientation();

		sprintf(mname,"%s",mesh->name);

		for (n=0;n<(int) strlen(mname);n++)
		{
			if (mname[n]==' ') mname[n]='_';
			if (mname[n]==' ') mname[n]='_';
		}

		fprintf(f,"%c",10);

		fprintf(f,"\tFrame %s {%c",mname,10);

		fprintf(f,"%c",10);

		fprintf(f,"\t\tFrameTransformMatrix {%c",10);
		fprintf(f,"\t\t%3.3f,%3.3f,%3.3f,%3.3f,%3.3f,%3.3f,%3.3f,%3.3f,%3.3f,%3.3f,%3.3f,%3.3f,%3.3f,%3.3f,%3.3f,%3.3f;;%c",
				M.a[0][0],M.a[0][1],M.a[0][2],M.a[0][3],
				M.a[1][0],M.a[1][1],M.a[1][2],M.a[1][3],
				M.a[2][0],M.a[2][1],M.a[2][2],M.a[2][3],
				M.a[3][0],M.a[3][1],M.a[3][2],M.a[3][3],10);
		fprintf(f,"\t\t}%c",10);

		fprintf(f,"%c",10);

		fprintf(f,"\t\tMesh {%c",10);

		fprintf(f,"\t\t\t%d;%c",obj->nVertices,10);
		for (n=0;n<obj->nVertices;n++) 
		{
			if (n==obj->nVertices-1) fprintf(f,"\t\t\t%3.3f;%3.3f;%3.3f;;%c",obj->Vertices[n].Stok.x,obj->Vertices[n].Stok.y,obj->Vertices[n].Stok.z,10);
			else fprintf(f,"\t\t\t%3.3f;%3.3f;%3.3f;,%c",obj->Vertices[n].Stok.x,obj->Vertices[n].Stok.y,obj->Vertices[n].Stok.z,10);
		}		

		fprintf(f,"\t\t\t%d;%c",obj->nFaces,10);
		for (n=0;n<obj->nFaces;n++) 
		{
			if (n==obj->nFaces-1) fprintf(f,"\t\t\t%d;%d,%d,%d;;%c",3,obj->Faces[n].v0,obj->Faces[n].v1,obj->Faces[n].v2,10);
			else fprintf(f,"\t\t\t%d;%d,%d,%d;,%c",3,obj->Faces[n].v0,obj->Faces[n].v1,obj->Faces[n].v2,10);
		}

		fprintf(f,"%c",10);

		fprintf(f,"\t\t\tMeshNormals {%c",10);

			fprintf(f,"\t\t\t\t%d;%c",obj->nVertices,10);
			for (n=0;n<obj->nVertices;n++) 
			{
				if (n==obj->nVertices-1) fprintf(f,"\t\t\t\t%3.3f;%3.3f;%3.3f;;%c",-obj->Vertices[n].Norm.x,-obj->Vertices[n].Norm.y,-obj->Vertices[n].Norm.z,10);
				else fprintf(f,"\t\t\t\t%3.3f;%3.3f;%3.3f;,%c",-obj->Vertices[n].Norm.x,-obj->Vertices[n].Norm.y,-obj->Vertices[n].Norm.z,10);
			}		

			fprintf(f,"\t\t\t\t%d;%c",obj->nFaces,10);
			for (n=0;n<obj->nFaces;n++) 
			{
				if (n==obj->nFaces-1) fprintf(f,"\t\t\t\t%d;%d,%d,%d;;%c",3,obj->Faces[n].v0,obj->Faces[n].v1,obj->Faces[n].v2,10);
				else fprintf(f,"\t\t\t\t%d;%d,%d,%d;,%c",3,obj->Faces[n].v0,obj->Faces[n].v1,obj->Faces[n].v2,10);
			}

		fprintf(f,"\t\t\t}%c%c",10,10);

			fprintf(f,"\t\t\tMeshTextureCoords {%c",10);
			fprintf(f,"\t\t\t\t%d;%c",obj->nVertices,10);

			for (n=0;n<obj->nVertices;n++)
			{
				if (n==obj->nVertices-1) fprintf(f,"\t\t\t\t%3.3f;%3.3f;;%c",obj->Vertices[n].Map.x,obj->Vertices[n].Map.y,10);
				else fprintf(f,"\t\t\t\t%3.3f;%3.3f;,%c",obj->Vertices[n].Map.x,obj->Vertices[n].Map.y,10);
			}
			fprintf(f,"\t\t\t}%c",10);

			fprintf(f,"%c",10);

			fprintf(f,"\t\t\tMeshMaterialList {%c",10);
			
			int max_nt=0;
			for (n=0;n<obj->nFaces;n++)
			{
				if (obj->Faces[n].nT>max_nt) max_nt=obj->Faces[n].nT;
			}

			fprintf(f,"\t\t\t\t%d;%c",max_nt+1,10);
			fprintf(f,"\t\t\t\t%d;%c",obj->nFaces,10);

			for (n=0;n<obj->nFaces;n++)
			{
				if (n==obj->nFaces-1) fprintf(f,"\t\t\t\t%d;%c",obj->Faces[n].nT,10);
				else fprintf(f,"\t\t\t\t%d,%c",obj->Faces[n].nT,10);
			}

			for (n=0;n<=max_nt;n++)
			{
				fprintf(f,"%c",10);
				fprintf(f,"\t\t\t\tMaterial {%c",10);
				fprintf(f,"\t\t\t\t1.000;1.000;1.000;1.000;;%c",10);
				fprintf(f,"\t\t\t\t0.000;%c",10);
				fprintf(f,"\t\t\t\t1.000;1.000;1.000;;%c",10);
				fprintf(f,"\t\t\t\t0.000;0.000;0.000;;%c",10);
				fprintf(f,"\t\t\t\t}%c",10);
			}

			fprintf(f,"%c",10);

			fprintf(f,"\t\t\t}%c",10);

		fprintf(f,"\t\t}%c",10);
		fprintf(f,"\t}%c",10);

		obj->InverseOrientation();

		mesh=Meshes.GetNext();
	}

	fprintf(f,"}%c",10);

	fclose(f);
#endif
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int __c4(unsigned char * data)
{
	unsigned char tmp[4];
	unsigned int value;
#if defined(_BIG_ENDIAN)
	tmp[0]=data[3];
	tmp[1]=data[2];
	tmp[2]=data[1];
	tmp[3]=data[0];
#else
	tmp[0]=data[0];
	tmp[1]=data[1];
	tmp[2]=data[2];
	tmp[3]=data[3];
#endif
	memcpy(&value,tmp,4);
	return value;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned short int __c2(unsigned char * data)
{
	unsigned char tmp[2];
	unsigned short int value;
	
#if defined(_BIG_ENDIAN)
	tmp[0]=data[1];
	tmp[1]=data[0];
#else
	tmp[0]=data[0];
	tmp[1]=data[1];
#endif

	memcpy(&value,tmp,2);

	return value;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void fread__mac_pc(void * data,int size,int unused,FILE *f)
{
	unsigned char tmp[4];

	if (size==1) fread(data,1,1,f);
	else
	{
		if (size==2)
		{
			fread(tmp,2,1,f);
			unsigned short int readable=__c2(tmp);
			memcpy(data,&readable,2);
		}
		else
		{
			fread(tmp,4,1,f);
			unsigned int readable=__c4(tmp);
			memcpy(data,&readable,4);
		}
	}
}


bool testobj3Dold(char *str)
{
	char temp[10];
	const char *id_str="3Dv1.1";

	FILE *f=fopen(str,"rb");
	if(f==NULL) {
		return false;
	}
	fread(temp,9,1,f);
	temp[9]='\0';

	if (strcmp(&temp[3],id_str)==0)
	{
		fclose(f);
		return false;
	}
	return true;
}

void BASIC3D::Load(char *str)
{
#ifndef GLES
	int n;
	FILE *f;
	char sign[5];
	char signXXX[7];

	for (n=0;n<7;n++) signXXX[n]='\0';
	for (n=0;n<5;n++) sign[n]='\0';

	f=fopen(str,"rb");
	if(f==NULL) {
		return;
	}
	fread(&sign[0],4,1,f);
	fclose(f);

	f=fopen(str,"rb");
	if(f==NULL) {
		return;
	}
	fread(&signXXX[0],6,1,f);
	fclose(f);

	sign[2]='\0';
	signXXX[6]='\0';

	if (strcmp(signXXX,"3Dv1.0")==0)
	{
		BASIC3D_Mesh * actual;
		CObject3D *obj;
		int nv,nf;
		int i0,i1,i2;
		float x,y,z,xm,ym;
		int attr=0;

		f=fopen(str,"rb");
		if(f==NULL) {
			return;
		}
		fread(&signXXX[0],6,1,f);
		fread__mac_pc(&attr,4,1,f);

		fread__mac_pc(&nv,4,1,f);
		fread__mac_pc(&nf,4,1,f);

		obj=new CObject3D;
		obj->Init(nv,nf);

		for (n=0;n<nv;n++)
		{
			fread__mac_pc(&x,4,1,f);
			fread__mac_pc(&y,4,1,f);
			fread__mac_pc(&z,4,1,f);

			fread__mac_pc(&xm,4,1,f);
			fread__mac_pc(&ym,4,1,f);

			obj->Vertices[n].Stok.Init(x,y,z);
			obj->Vertices[n].Map.Init(xm,ym);
			obj->Vertices[n].Diffuse.Init(1,1,1,1);
		}

		for (n=0;n<nf;n++)
		{
			fread__mac_pc(&i0,4,1,f);
			fread__mac_pc(&i1,4,1,f);
			fread__mac_pc(&i2,4,1,f);
			
			obj->Faces[n].Init(i0,i1,i2);

			obj->Faces[n].tag=0;
			obj->Faces[n].tag2=0;
			obj->Faces[n].nT=0;
			obj->Faces[n].ref=0;
			obj->Faces[n].nT2=0;
		}
		
		fclose(f);

		obj->SetFaces();
		obj->CalculateNormals(-1);

		sprintf(obj->Name,"OBJECT");
		actual=Meshes[0];
        actual->name=(char*) malloc(8);
		sprintf(actual->name,"OBJECT");
		actual->obj=obj;
		actual->attr=MESH;
		actual->fstart=0;
		actual->fend=0;

		actual->p.Init(0,0,0);
		actual->q.Id();
		actual->s.Init(1,1,1);

		actual->pivot_p.Init(0,0,0);
		actual->pivot_q.Id();
		actual->pivot_s.Init(1,1,1);
	}
	else
	if (strcmp(sign,"B!")==0)
	{
		BASIC3D_Mesh * actual;
		CObject3D *obj;
		unsigned short int nv;
		unsigned short int nf;
		short int x,y,z,nx,ny,nz,uv,w;
		unsigned short int tab[8];

		f=fopen(str,"rb");
		if(f==NULL) {
			return;
		}
		fseek(f,0,SEEK_END);
		int size=ftell(f);
		fseek(f,0,SEEK_SET);

		fread(&sign[0],4,1,f);

		fread(&nv,2,1,f);
		fread(&nf,2,1,f);

		obj=new CObject3D;
		obj->Init(nv,nf);

		for (n=0;n<nv;n++)
		{
			fread(&x,2,1,f);
			fread(&y,2,1,f);
			fread(&z,2,1,f);
			fread(&nx,2,1,f);
			fread(&ny,2,1,f);
			fread(&nz,2,1,f);
			fread(&uv,2,1,f);
			fread(&w,2,1,f);
			obj->Vertices[n].Stok.Init((float) (x)/256.0f,(float) (-y)/256.0f,(float) (z)/256.0f);
			obj->Vertices[n].Map.Init(((float)(uv&255))/255.0f,((float)((uv>>8)&255))/255.0f);
			obj->Vertices[n].Diffuse.Init(1,1,1,1);
		}

		for (n=0;n<nf;n++)
		{
			fread(tab,16,1,f);
			obj->Faces[n].Init(tab[1],tab[2],tab[3]);
			obj->Faces[n].tag=0;
			obj->Faces[n].tag2=0;
			obj->Faces[n].nT=0;
			obj->Faces[n].ref=0;
			obj->Faces[n].nT2=0;
		}

		int temp=ftell(f);
		if (temp<size)
		{
			int nk=(size-temp)/(16*nv);
			for (int k=0;k<nk;k++)
			{
				obj->VerticesKeys[k]=new CShortVertex[nv];
				for (n=0;n<nv;n++)
				{
					fread(&x,2,1,f);
					fread(&y,2,1,f);
					fread(&z,2,1,f);
					fread(&nx,2,1,f);
					fread(&ny,2,1,f);
					fread(&nz,2,1,f);
					fread(&uv,2,1,f);
					fread(&w,2,1,f);
					obj->VerticesKeys[k][n].InitStok((float) (x)/256.0f,(float) (-y)/256.0f,(float) (z)/256.0f);
				}
			}
			obj->nKeys=nk;

		}
		fclose(f);

		obj->SetFaces();
		obj->CalculateNormals(-1);

		if (obj->nKeys>0) sprintf(obj->Name,"MORPH"); else sprintf(obj->Name,"OBJECT");
		actual=Meshes[0];
		actual->name=(char*) malloc(8);
		if (obj->nKeys>0) sprintf(actual->name,"MORPH"); else sprintf(actual->name,"OBJECT");
		actual->obj=obj;
		actual->attr=MESH;
		actual->fstart=0;
		actual->fend=0;

		actual->p.Init(0,0,0);
		actual->q.Id();
		actual->s.Init(1,1,1);

		actual->pivot_p.Init(0,0,0);
		actual->pivot_q.Id();
		actual->pivot_s.Init(1,1,1);
	}
	else
	{
		if (!testobj3Dold(str)) LoadBASIC3D(str);
		else
		{
			BASIC3D_Mesh * actual;
			CObject3D *obj;
			unsigned short int nv;
			unsigned short int nf;
			short int x,y,z,nx,ny,nz,uv,w;			

			f=fopen(str,"rb");
			if(f==NULL) {
				return;
			}
			fread(&nv,2,1,f);
			fread(&nf,2,1,f);

			obj=new CObject3D;
			obj->Init(nv,nf*2);

			for (n=0;n<nv;n++)
			{
				fread(&x,2,1,f);
				fread(&y,2,1,f);
				fread(&z,2,1,f);
				uv=0;
				obj->Vertices[n].Stok.Init((float) (x)/256.0f,(float) (-y)/256.0f,(float) (z)/256.0f);
				obj->Vertices[n].Map.Init(((float)(uv&255))/255.0f,((float)((uv>>8)&255))/255.0f);
				obj->Vertices[n].Diffuse.Init(1,1,1,1);
			}

			int p=0;
			for (n=0;n<nf;n++)
			{
				unsigned short int i0,i1,i2,i3,nb,nt;
				fread(&nb,2,1,f);
				if (nb<4)
				{
					fread(&nt,2,1,f);

					fread(&i0,2,1,f);
					fread(&i1,2,1,f);
					fread(&i2,2,1,f);

					fread(&nt,2,1,f);

					if ((i0>=nv)||(i1>=nv)||(i2>=nv))
					{
						obj->Free();
						delete obj;
						return;
					}
					obj->Faces[p].Init(i0,i1,i2);
					obj->Faces[p].tag=0;
					obj->Faces[p].tag2=0;
					obj->Faces[p].nT=0;
					obj->Faces[p].ref=0;
					obj->Faces[p].nT2=0;
					p++;
				}
				else
				{
					fread(&nt,2,1,f);

					fread(&i0,2,1,f);
					fread(&i1,2,1,f);
					fread(&i2,2,1,f);
					fread(&i3,2,1,f);

					if ((i0>=nv)||(i1>=nv)||(i2>=nv)||(i3>nv))
					{
						obj->Free();
						delete obj;
						return;
					}
					obj->Faces[p].Init(i0,i1,i2);
					obj->Faces[p].tag=0;
					obj->Faces[p].tag2=0;
					obj->Faces[p].nT=0;
					obj->Faces[p].ref=0;
					obj->Faces[p].nT2=0;
					p++;
					obj->Faces[p].Init(i0,i2,i3);
					obj->Faces[p].tag=0;
					obj->Faces[p].tag2=0;
					obj->Faces[p].nT=0;
					obj->Faces[p].ref=0;
					obj->Faces[p].nT2=0;
					p++;
				}
			}

			obj->nFaces=p;

			fclose(f);

			obj->SetFaces();
			obj->CalculateNormals(-1);

			sprintf(obj->Name,"OBJ94");
			actual=Meshes[0];
			actual->name=(char*) malloc(8);
			sprintf(actual->name,"OBJ94");
			actual->obj=obj;
			actual->attr=MESH;
			actual->fstart=0;
			actual->fend=0;

			actual->p.Init(0,0,0);
			actual->q.Id();
			actual->s.Init(1,1,1);

			actual->pivot_p.Init(0,0,0);
			actual->pivot_q.Id();
			actual->pivot_s.Init(1,1,1);

		}
	}
#else
	LoadBASIC3D(str);
#endif
}

void BASIC3D::LoadBASIC3D(char *str)
{
	const char *id_str="3Dv1.1";
	BASIC3D_HDR hdr;
	BASIC3D_INFO nfo;
	BASIC3D_MATERIAL mat;
	BASIC3D_TEXTURE tex;
//	CList <CObject3D*> *list;
	CObject3D *obj;
#if defined(ANDROID)&&!defined(GLESFULLNOZIP)
	ZIPFILE *f;
#else
	FILE *f;
#endif
	char temp[10];
	char chunk[5];
	unsigned int attr;
	float x=0;
	float y=0;
	float z=0;
	int nv,np,nf;
	bool end;
	int nt,smg;
	int n,nn;
	char *buf;
	int id;
	float qx,qy,qz,qw,sx,sy,sz;
	int i[3];
	float v[3];
	BASIC3D_Mesh * actual;
	int sampling_rate;
	int n1,n2;
	float cfar,cnear;
	int target,fovtype;
	float fov;
	int id_proj_map,id_proj_shadow;
	float r,g,b;
	CDataReader cdr1,cdr2;

	chunk[4]='\0';

	end=false;

#ifdef NOT_COMPRESSED

	int size;

	f=fopen(str,"rb");
	if(f==NULL) {
		return;
	}
	fseek(f,0L,SEEK_END);
	size=ftell(f);
	fclose(f);

	f=fopen(str,"rb");
	if(f==NULL) {
		return;
	}
	fread(temp,9,1,f);
	BASIC3DREAD.get(f,size-9);
	fclose(f);

#else
#if defined(ANDROID)&&!defined(GLESFULLNOZIP)

	f=ZIPfopen(str,"rb");
	if(f==NULL) {
		return;
	}
	ZIPfread(temp,9,1,f);
	temp[9]='\0';

	if (strcmp(&temp[3],id_str)!=0)
	{
		ZIPfclose(f);
		return;
	}

	ZIPfread(chunk,4,1,f);					// HDR_
	ZIPfread(&hdr,sizeof(BASIC3D_HDR),1,f);
	cdr1.getLZ(f);
	cdr2.getLZ(f);
	ZIPfclose(f);
	BASIC3DREAD=cdr1+cdr2;
	cdr1.clean();
	cdr2.clean();
#else
	f=fopen(str,"rb");
	if(f==NULL) {
		return;
	}
	fread(temp,9,1,f);
	temp[9]='\0';

	if (strcmp(&temp[3],id_str)!=0)
	{
		fclose(f);
		return;
	}

	fread(chunk,4,1,f);					// HDR_
	fread(&hdr,sizeof(BASIC3D_HDR),1,f);
	cdr1.getLZ(f);
	cdr2.getLZ(f);
	fclose(f);
	BASIC3DREAD=cdr1+cdr2;
	cdr1.clean();
	cdr2.clean();
#endif
#endif


	while ((!end)&&(!BASIC3DREAD.eod()))
	{
		BASIC3DREAD.readData(chunk,4);
		
		if (strcmp(chunk,"ENDF")==0) end=true;

		if (strcmp(chunk,"HDR_")==0) 
		{
			BASIC3DREAD.readData(&hdr,sizeof(BASIC3D_HDR));
		}
		
		if (strcmp(chunk,"MAT_")==0) 
		{
			BASIC3DREAD.readData(&mat,sizeof(BASIC3D_MATERIAL));
			Materials.Add(mat);
		}

		if (strcmp(chunk,"INFO")==0) 
		{
			BASIC3DREAD.readData(&nfo,sizeof(BASIC3D_INFO));
			
			keystart=nfo.framestart;
			keyend=nfo.frameend;
			frame_rate=nfo.frame_rate;
		}

		if (strcmp(chunk,"TEX_")==0)
		{
			
			tex.texID=BASIC3DREAD.readInt();
			tex.filename=BASIC3DREAD.readString();

			tex.TileU=CLAMP;
			tex.TileV=CLAMP;
			tex.OffsetU=0;
			tex.OffsetV=0;
			tex.ScaleU=1.0f;
			tex.ScaleV=1.0f;

			tex.Transform.Id();
			Textures.Add(tex);

		}

		if (strcmp(chunk,"TE2_")==0)
		{
			tex.texID=BASIC3DREAD.readInt();
			
			tex.filename=BASIC3DREAD.readString();

			tex.TileU=BASIC3DREAD.readInt();
			tex.TileV=BASIC3DREAD.readInt();

			tex.OffsetU=BASIC3DREAD.readFloat();
			tex.OffsetV=BASIC3DREAD.readFloat();

			tex.ScaleU=BASIC3DREAD.readFloat();
			tex.ScaleV=BASIC3DREAD.readFloat();

			tex.Transform.a[0][0]=BASIC3DREAD.readFloat();
			tex.Transform.a[0][1]=BASIC3DREAD.readFloat();
			tex.Transform.a[0][2]=BASIC3DREAD.readFloat();
			tex.Transform.a[0][3]=BASIC3DREAD.readFloat();

			tex.Transform.a[1][0]=BASIC3DREAD.readFloat();
			tex.Transform.a[1][1]=BASIC3DREAD.readFloat();
			tex.Transform.a[1][2]=BASIC3DREAD.readFloat();
			tex.Transform.a[1][3]=BASIC3DREAD.readFloat();

			tex.Transform.a[2][0]=BASIC3DREAD.readFloat();
			tex.Transform.a[2][1]=BASIC3DREAD.readFloat();
			tex.Transform.a[2][2]=BASIC3DREAD.readFloat();
			tex.Transform.a[2][3]=BASIC3DREAD.readFloat();

			tex.Transform.a[3][0]=BASIC3DREAD.readFloat();
			tex.Transform.a[3][1]=BASIC3DREAD.readFloat();
			tex.Transform.a[3][2]=BASIC3DREAD.readFloat();
			tex.Transform.a[3][3]=BASIC3DREAD.readFloat();

			Textures.Add(tex);
		}

		if (strcmp(chunk,"ANI2")==0)
		{
		
			unsigned int val;
			unsigned int ux,uy,uz;
			int nkeys;
			int id;
			unsigned int size,type;
			CVector min,max;

			id=BASIC3DREAD.readInt();
			nkeys=BASIC3DREAD.readInt();
			size=BASIC3DREAD.readInt();
			type=BASIC3DREAD.readInt();

			actual=Meshes[id];

			if (type&MORPH)
			{
				sampling_rate=BASIC3DREAD.readInt();
				nv=BASIC3DREAD.readInt();

				if (sampling_rate>0)
				{
					actual->obj->anim_strings=(char*) malloc(sampling_rate+1);
					memset(actual->obj->anim_strings,0,sampling_rate+1);
					BASIC3DREAD.readData(actual->obj->anim_strings,sampling_rate);
				}

				min=BASIC3DREAD.readVector();
				max=BASIC3DREAD.readVector();

				for (n1=0;n1<nkeys;n1++)
				{
					actual->obj->VerticesKeys[n1]=new CShortVertex[nv];
					for (n2=0;n2<nv;n2++)
					{
						val=BASIC3DREAD.readUInt();
						ux=(val>>20)&((1<<10)-1);
						uy=(val>>10)&((1<<10)-1);
						uz= (val>>0)&((1<<10)-1);

						x=min.x + (((float)ux)*(max.x - min.x)/((float)((1<<10)-1)));
						y=min.y + (((float)uy)*(max.y - min.y)/((float)((1<<10)-1)));
						z=min.z + (((float)uz)*(max.z - min.z)/((float)((1<<10)-1)));

						actual->obj->VerticesKeys[n1][n2].InitStok(x,y,z);
					}
				}

				actual->obj->nKeys=nkeys;
				actual->obj->actual_key=0;

				actual->sampling_rate=sampling_rate;
				actual->nsamplekeys=nkeys;
			}
		}


		if (strcmp(chunk,"ANI3")==0)
		{
			int nkeys;
			int id;
			unsigned int size,type;
			CVector min,max;

			id=BASIC3DREAD.readInt();
			nkeys=BASIC3DREAD.readInt();
			size=BASIC3DREAD.readInt();
			type=BASIC3DREAD.readInt();

			actual=Meshes[id];

			if (type&MORPH)
			{
				sampling_rate=BASIC3DREAD.readInt();
				nv=BASIC3DREAD.readInt();

				if (sampling_rate>0)
				{
					actual->obj->anim_strings=(char*) malloc(sampling_rate+1);
					memset(actual->obj->anim_strings,0,sampling_rate+1);
					BASIC3DREAD.readData(actual->obj->anim_strings,sampling_rate);
				}


				for (n1=0;n1<nkeys;n1++)
				{
					actual->obj->VerticesKeys[n1]=new CShortVertex[nv];
					for (n2=0;n2<nv;n2++)
					{
						x=BASIC3DREAD.readFloat();
						y=BASIC3DREAD.readFloat();
						z=BASIC3DREAD.readFloat();
						actual->obj->VerticesKeys[n1][n2].InitStok(x,y,z);
					}
				}

				actual->obj->nKeys=nkeys;
				actual->obj->actual_key=0;

				actual->sampling_rate=sampling_rate;
				actual->nsamplekeys=nkeys;
			}
		}


		if (strcmp(chunk,"ANIM")==0)
		{
			int nkeys;
			int id,nkey;
			unsigned int size,type;
			BASIC3D_Keyframe * key;

			id=BASIC3DREAD.readInt();
			nkeys=BASIC3DREAD.readInt();
			size=BASIC3DREAD.readInt();
			type=BASIC3DREAD.readInt();

			actual=Meshes[id];

			if (type&MORPH)
			{
				sampling_rate=BASIC3DREAD.readInt();
				nv=BASIC3DREAD.readInt();
				for (n1=0;n1<nkeys;n1++)
				{
					actual->obj->VerticesKeys[n1]=new CShortVertex[nv];
					for (n2=0;n2<nv;n2++)
					{
						x=BASIC3DREAD.readFloat();	
						y=BASIC3DREAD.readFloat();
						z=BASIC3DREAD.readFloat();
						actual->obj->VerticesKeys[n1][n2].InitStok(x,y,z);
					}
				}

				actual->obj->nKeys=nkeys;

				actual->sampling_rate=sampling_rate;
				actual->nsamplekeys=nkeys;
			}
			else
			{
				actual->sampling_rate=frame_rate;

				if (type&ROTATION)
				{
					for (n=0;n<nkeys;n++)
					{
						nkey=BASIC3DREAD.readInt();
						qx=BASIC3DREAD.readFloat();
						qy=BASIC3DREAD.readFloat();
						qz=BASIC3DREAD.readFloat();
						qw=BASIC3DREAD.readFloat();


						key=actual->keys[nkey];
						if (!key->q) key->q=new CQuaternion;
						key->q->Init(qw,qx,qy,qz);
						key->q->Normalise();

						if (!key->p)
						{
							key->p=new CVector;
							key->p->Init(0,0,0);
						}
					}
				}
				else
				{
					if (type&POSITION)
					{
						for (n=0;n<nkeys;n++)
						{
							nkey=BASIC3DREAD.readInt();
							x=BASIC3DREAD.readFloat();
							y=BASIC3DREAD.readFloat();
							z=BASIC3DREAD.readFloat();

							key=actual->keys[nkey];
							if (!key->p) key->p=new CVector;
							key->p->Init(x,y,z);

							if (!key->q)
							{
								key->q=new CQuaternion;
								key->q->Id();
							}

						}

					}
					else
					{
						for (n=0;n<nkeys;n++)
						{
							nkey=BASIC3DREAD.readInt();
							sx=BASIC3DREAD.readFloat();
							sy=BASIC3DREAD.readFloat();
							sz=BASIC3DREAD.readFloat();

							qx=BASIC3DREAD.readFloat();
							qy=BASIC3DREAD.readFloat();
							qz=BASIC3DREAD.readFloat();
							qw=BASIC3DREAD.readFloat();

							key=actual->keys[nkey];
							if (!key->q) key->q=new CQuaternion;
							key->q->Init(qw,qx,qy,qz);
							key->q->Normalise();

							if (!key->p) key->p=new CVector;
							key->p->Init(x,y,z);

						}

					}
				}

			}
		}


		if (strcmp(chunk,"CAM_")==0)
		{
			attr=BASIC3DREAD.readInt();
			fovtype=BASIC3DREAD.readInt();
			fov=BASIC3DREAD.readFloat();
			id=BASIC3DREAD.readInt();

			actual=Meshes[id];
			actual->attr=attr;
			actual->Affect();

			target=BASIC3DREAD.readInt();
			cnear=BASIC3DREAD.readFloat();
			cfar=BASIC3DREAD.readFloat();
		}

		if (strcmp(chunk,"LIGH")==0)
		{
			attr=BASIC3DREAD.readInt();
			id=BASIC3DREAD.readInt();
			actual=Meshes[id];
			actual->attr=attr;
			actual->Affect();

			if (attr&TARGET)
			{
				target=BASIC3DREAD.readInt();
			}

			id_proj_map=BASIC3DREAD.readInt();
			id_proj_shadow=BASIC3DREAD.readInt();

			r=BASIC3DREAD.readFloat();
			g=BASIC3DREAD.readFloat();
			b=BASIC3DREAD.readFloat();
		}

		if (strcmp(chunk,"MESH")==0)
		{
			attr=BASIC3DREAD.readInt();
			id=BASIC3DREAD.readInt();

			
			actual=Meshes[id];

			actual->attr=attr;
			actual->fstart=keystart;
			actual->fend=keyend;

			actual->Affect();

			if (!(attr&HELPER))
			{
				np=BASIC3DREAD.readInt();
				
				if (np>=0)
				{
					nf=BASIC3DREAD.readInt();

					obj=new CObject3D;
					obj->Coo=actual->p;
					obj->Q=actual->q;

					obj->Init(np,nf);
					for (n=0;n<np;n++)
					{
						v[0]=BASIC3DREAD.readFloat();
						v[1]=BASIC3DREAD.readFloat();
						v[2]=BASIC3DREAD.readFloat();
						obj->Vertices[n].Stok.Init(v[0],v[1],v[2]);
						v[0]=BASIC3DREAD.readFloat();
						v[1]=BASIC3DREAD.readFloat();
						obj->Vertices[n].Map.Init(v[0],v[1]);
						obj->Vertices[n].Diffuse.Init(1,1,1);
					}

					for (n=0;n<nf;n++)
					{
						i[0]=BASIC3DREAD.readInt();
						i[1]=BASIC3DREAD.readInt();
						i[2]=BASIC3DREAD.readInt();
						smg=BASIC3DREAD.readInt();
						nt=BASIC3DREAD.readInt();

						obj->Faces[n].Init(i[0],i[1],i[2]);
						obj->Faces[n].nT=nt;
						obj->Faces[n].tag=smg;
						
						obj->Faces[n].ref=0;
					}

					if (attr&SKIN)
					{
						n=BASIC3DREAD.readInt();
						
						if (n>=0)
						{
							obj->VerticesAdd=new CVertexAdd[np];
							for (n=0;n<np;n++)
							{
								obj->VerticesAdd[n].nbones=BASIC3DREAD.readInt();
								
								for (nn=0;nn<obj->VerticesAdd[n].nbones;nn++)
								{
									obj->VerticesAdd[n].offsets[nn].x=BASIC3DREAD.readFloat();
									obj->VerticesAdd[n].offsets[nn].y=BASIC3DREAD.readFloat();
									obj->VerticesAdd[n].offsets[nn].z=BASIC3DREAD.readFloat();
								}

								for (nn=0;nn<obj->VerticesAdd[n].nbones;nn++)
								{
									obj->VerticesAdd[n].bonesID[nn]=BASIC3DREAD.readInt();
									obj->VerticesAdd[n].bones[nn]=BASIC3DREAD.readInt();
									obj->VerticesAdd[n].weigths[nn]=BASIC3DREAD.readFloat();
								}
							}
							// bone links
							n=BASIC3DREAD.readInt();
							if (n>=0)
							{
								buf=BASIC3DREAD.readData(n*8);
								free(buf);
							}

						}
					}
			
					obj->SetFaces();
					if (SmgTyp)
						obj->CalculateNormalsSmoothingGroups2();
					else
						obj->CalculateNormalsSmoothingGroups();

					for (n=0;n<obj->nFaces;n++) obj->Faces[n].tag=0;
					for (n=0;n<obj->nFaces;n++) obj->Faces[n].ref=0;


					obj->nKeys=0;


					actual->obj=obj;
				}
			}
		}
	}

	BASIC3DREAD.clean();
}


void BASIC3D::ToggleSmoothing()
{
	if (SmgTyp) SmgTyp=false; else SmgTyp=true;
}

void BASIC3D::ToggleSmoothing(bool b)
{
	SmgTyp=b;
}


BASIC3D_Mesh * BASIC3D::GetMesh(char *name)
{
	BASIC3D_Mesh *m=Meshes.GetFirst();
	while (m)
	{
		if (strcmp(m->name,name)==0) return m;
		m=Meshes.GetNext();
	}
	return NULL;
}

BASIC3D_MATERIAL * BASIC3D::GetMaterial(int id)
{
	BASIC3D_MATERIAL *res=NULL;
	BASIC3D_MATERIAL *m=Materials.GetFirst();
	while (m)
	{
		if (m->matID==id) res=m;
		m=Materials.GetNext();
	}
	return res;
}

BASIC3D_TEXTURE * BASIC3D::GetTexture(int id)
{
	BASIC3D_TEXTURE *res=NULL;
	BASIC3D_TEXTURE *t=Textures.GetFirst();
	while (t)
	{
		if (t->texID==id) res=t;
		t=Textures.GetNext();
	}
	return res;
}

BASIC3D_Limb * BASIC3D::GetLimb(char *name)
{
    BASIC3D_Limb *l=Limbs.GetFirst();
    while (l)
    {
        if (strcmp(l->name,name)==0) return l;
        l=Limbs.GetNext();
    }
    return NULL;
}

BASIC3D_Cluster * BASIC3D_Mesh::GetCluster(char *name)
{
	BASIC3D_Cluster *cl=clusters.GetFirst();
	while (cl)
	{
		if (strcmp(cl->name,name)==0) return cl;
		cl=clusters.GetNext();
	}
	return NULL;
}

BASIC3D_Limb * BASIC3D::GetLimbById(long long id)
{
    BASIC3D_Limb *l=Limbs.GetFirst();
    while (l)
    {
        if (l->id==id) return l;
        l=Limbs.GetNext();
    }
    return NULL;
}

BASIC3D_Cluster * BASIC3D_Mesh::GetClusterById(long long id)
{
    BASIC3D_Cluster *cl=clusters.GetFirst();
    while (cl)
    {
        if (cl->id==id) return cl;
        cl=clusters.GetNext();
    }
    return NULL;
}

BASIC3D_Limb * BASIC3D_Mesh::GetLimb(char *name)
{
    BASIC3D_Limb **pl=limbs.GetFirst();
    while (pl)
    {
        if (strcmp((*pl)->name,name)==0) return *pl;
        pl=limbs.GetNext();
    }
    return NULL;
}

BASIC3D_Limb * BASIC3D_Mesh::GetLimbById(long long id)
{
    BASIC3D_Limb **pl=limbs.GetFirst();
    while (pl)
    {
        if ((*pl)->id==id) return *pl;
        pl=limbs.GetNext();
    }
    return NULL;
}


BASIC3D_Mesh * BASIC3D::GetMeshFromLimb(char *name)
{
	BASIC3D_Mesh *m=Meshes.GetFirst();
	while (m)
	{
		if (m->GetLimb(name)) return m;
		m=Meshes.GetNext();
	}
	return NULL;
}


