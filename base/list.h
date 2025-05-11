
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

#ifndef _LIST_H_
#define _LIST_H_

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef NULL
	#define NULL 0
#endif
#ifdef WIN32
#define CMAPLISTCONV __fastcall
#else
#define CMAPLISTCONV
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CSElement
{
public:
	int data;
	CSElement * next;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <class T>
class CElement
{
public:
	T data;
	CElement * next;
	CElement * prev;

	CElement()
	{
		next=NULL;
		prev=NULL;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <class T>
class CList 
{
public:
	CElement <T> * First;
	CElement <T> * Closing;
	CElement <T> * Current;
	CElement <T> * Current2;
	CElement <T> * Save;
	CElement <T> * Save2;
	int nb;
	int index;
	CElement <T> * Cro;

	CList()
	{
		Cro=First=Closing=NULL;
		//Save=NULL;
		//Save2=NULL;
		nb=0;
		index=-1;
	}
    
    void Zero()
    {
        First=NULL;
        Closing=NULL;
        Current=NULL;
        Current2=NULL;
        nb=0;
        index=-1;
        Cro=NULL;
    }

	int CMAPLISTCONV Length()
	{
		return nb;
	}

	T * CMAPLISTCONV GetFirst()
	{
		Current=First;
		if (Current!=NULL)
			return &(Current->data);
		else return NULL;
	}

	T * CMAPLISTCONV GetNext()
	{
		Current=Current->next;
		if (Current!=NULL)
			return &(Current->data);
		else return NULL;
	}

	T* CMAPLISTCONV DeleteAndGetNext()
	{
		CElement <T> * el,*prev;
		index=-1;

		if (nb>0)
		{
			if (Current==First)
			{
				DeleteFirst();
				Current=First;
			}
			else
				if (Current==Closing)
				{
					Current=NULL;
					DeleteLast();
				}
				else
				{
					el=Current;
					Current=Current->next;
					prev=el->prev;

					Current->prev=prev;
					prev->next=Current;
					delete el;
					nb--;
				}

			if (Current!=NULL)
				return &(Current->data);
			else
				return NULL;
		}
		else return NULL;
	}

	void CMAPLISTCONV InsertCurrent(T data)
	{
		CElement <T> * el;
		index=-1;

		if (Current)
		{
			if (Current==Closing)
			{
				Add(data);
				Current=Closing;
			}
			else
			{
				el=new CElement <T>;
				el->data=data;
				el->next=Current->next;
				el->prev=Current;
				(Current->next)->prev=el;
				Current->next=el;
				nb++;

				Current=Current->next;
			}
		}
		else
		{
			InsertFirst(data);
			Current=First;
		}
	}

	T * CMAPLISTCONV SetCurrent(int n)
	{
		int nn;
		CElement <T> * el;

		if (First)
		{			
			if (index<0)
			{
				el=First;
				if (n>0)
				for (nn=0;nn<n;nn++)
					if (el->next!=NULL) el=el->next;

				Cro=el;
				index=n;

				if (el) Current=el;
				else Current=NULL;
			}
			else
			{
				if (index==n) el=Cro;
				else
				{
					el=First;
					if (n>0)
					for (nn=0;nn<n;nn++)
						if (el->next!=NULL) el=el->next;
				}

				Cro=el;
				index=n;

				if (el) Current=el;
				else 
				{
					index=-1;
					Current=NULL;
				}
			}
		}
		else 
		{
			index=-1;
			Current=NULL;
		}

		if (Current!=NULL)
			return &(Current->data);
		else return NULL;
	}

	void CMAPLISTCONV DeleteLast()
	{
		CElement <T> * el;
		index=-1;

		if (nb>0)
		{
			el=Closing;
			Closing=Closing->prev;
			if (Closing) Closing->next=NULL;
			delete el;
			nb--;
			if (nb==0)
			{
				First=NULL;
				Closing=NULL;
			}
		}
	}

	T * CMAPLISTCONV GetFirst2()
	{
		Current2=First;
		if (Current2!=NULL)
			return &(Current2->data);
		else return NULL;
	}

	T * CMAPLISTCONV GetNext2()
	{
		Current2=Current2->next;
		if (Current2!=NULL)
			return &(Current2->data);
		else return NULL;
	}

	T * CMAPLISTCONV GetNext1()
	{
		Current2=Current->next;
		if (Current2!=NULL)
			return &(Current2->data);
		else return NULL;
	}

	T* CMAPLISTCONV Add(T item)
	{
		CElement <T> * el;
		index=-1;

		if (Closing!=NULL)
		{
			el=Closing;
			Closing->next=new CElement <T>;
			Closing=Closing->next;
			Closing->prev=el;
			Closing->next=NULL;
			Closing->data=item;
			el=Closing;
			nb++;
		}
		else
		{
			First=new CElement <T>;
			First->data=item;
			Closing=First;
			Closing->prev=NULL;
			Closing->next=NULL;
			el=First;
			nb++;
		}
		return &(el->data);
	}

	T* CMAPLISTCONV InsertLast()
	{
		CElement <T> * el;
		index=-1;

		if (Closing!=NULL)
		{
			el=Closing;
			Closing->next=new CElement <T>;
			Closing=Closing->next;
			Closing->prev=el;
			Closing->next=NULL;
			el=Closing;
			nb++;
		}
		else
		{
			First=new CElement <T>;
			Closing=First;
			Closing->prev=NULL;
			Closing->next=NULL;
			el=First;
			nb++;
		}
		return &(el->data);
	}

	inline T* operator [](int n)
	{
		int nn;
		CElement <T> * el;
		if (First)
		{			
			if (index<0)
			{
				el=First;
				if (n>0)
				for (nn=0;nn<n;nn++)
					if (el->next!=NULL) el=el->next;

				Cro=el;
				index=n;

				if (el) return &(el->data);
				else return NULL;
			}
			else
			{
				if (index==n) el=Cro;
				else
				if (n==0) el=First;
				else
				{
					el=First;
					if (n>0)
					for (nn=0;nn<n;nn++)
						if (el->next!=NULL) el=el->next;
				}

				Cro=el;
				index=n;

				if (el) return &(el->data);
				else 
				{
					index=-1;
					return NULL;
				}
			}
		}
		else 
		{
			index=-1;
			return NULL;
		}
	}

	inline CElement <T>* operator ()(int n)
	{
		int nn;
		CElement <T> * el;
		if (First)
		{			
			if (index<0)
			{
				el=First;
				if (n>0)
				for (nn=0;nn<n;nn++)
					if (el->next!=NULL) el=el->next;

				Cro=el;
				index=n;

				if (el) return el;
				else return NULL;
			}
			else
			{
				if (index==n) el=Cro;
				else
				if (n==0) el=First;
				else
				{
					el=First;
					if (n>0)
					for (nn=0;nn<n;nn++)
						if (el->next!=NULL) el=el->next;
				}

				Cro=el;
				index=n;

				if (el) return el;
				else 
				{
					index=-1;
					return NULL;
				}
			}
		}
		else 
		{
			index=-1;
			return NULL;
		}
	}

	T* CMAPLISTCONV Last()
	{
		return &(Closing->data);
	}

	void CMAPLISTCONV Del(int n)
	{
		int nn;
		CElement <T> *el;
		int res;

		index=-1;

		if (nb>0)
		{
			el=First;
			res=0;
			for (nn=0;nn<n;nn++)
			{
				if (el->next!=NULL) el=el->next;
				else res=1;
			}

			if (res==0)
			{
				if (el==First)
				{
					DeleteFirst();
				}
				else
				{
					if (el==Closing)
					{
						DeleteLast();
					}
					else
					{
						(el->prev)->next=el->next;
						(el->next)->prev=el->prev;
						delete el;
						nb--;
					}
				}
			}
		}
	}

	void CMAPLISTCONV DelEl(CElement <T> *el)
	{
		index=-1;

		if (nb>0)
		{
			if (el==First)
			{
				DeleteFirst();
			}
			else
			{
				if (el==Closing)
				{
					DeleteLast();
				}
				else
				{
					(el->prev)->next=el->next;
					(el->next)->prev=el->prev;
					delete el;
					nb--;
				}
			}
		}
	}

	void CMAPLISTCONV Delete(T* del)
	{
		T * tmp;

		index=-1;

		if (nb>0)
		{
			tmp=GetFirst();
			while (tmp)
			{
				if (tmp==del)
				{
					DeleteAndGetNext();
					tmp=NULL;
				}
				else tmp=GetNext();
			}
		}
	}

	void CMAPLISTCONV Delete(T del)
	{
		T * tmp;

		index=-1;

		if (nb>0)
		{
			tmp=GetFirst();
			while (tmp)
			{
				if (*tmp==del)
				{
					DeleteAndGetNext();
					tmp=NULL;
				}
				else tmp=GetNext();
			}
		}
	}

	void CMAPLISTCONV Free()
	{
		CElement <T> * el,* next;
		next=First;
		while (next!=NULL)
		{
			el=next;
			next=el->next;
			delete el;
		}

		First=NULL;
		Closing=NULL;

		nb=0;
		index=-1;	
	}

	void CMAPLISTCONV DeleteFirst()
	{
		CElement <T> * el;
		index=-1;

		if (nb>0)
		{
			el=First;
			First=First->next;
			delete el;
			nb--;
			if (nb==0) 
			{
				First=NULL;
				Closing=NULL;
			}
			else First->prev=NULL;				
		}
	}

	void CMAPLISTCONV InsertFirst(T data)
	{
		CElement <T> * el;
		index=-1;

		if (First!=NULL)
		{
			el=First;
			First=new CElement <T>;
			First->data=data;
			First->next=el;
			First->prev=NULL;
			el->prev=First;
		}
		else
		{
			Closing=First=new CElement <T>;
			First->data=data;
			First->next=NULL;
			First->prev=NULL;
		}
		nb++;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <class Tindex,class Tdata> 
class CMap
{
public:

	class Item 
	{
	public:
		Tdata	data;			// données
		Tindex  Id;				// index
		Item	*next;			
		Item    *prev;

		Item() {}
		Item(Tdata dat) { data=dat; }
	};

	Item * First;
	Item * Last;
	Item * Current;
	int nb;

	CMap() { First=Last=NULL;nb=0;}

	Tdata * CMAPLISTCONV GetFirst()
	{
		if (First)
		{
			Current=First;
			return &(First->data);
		}
		else return NULL;
	}

	Tdata * CMAPLISTCONV GetNext()
	{
		Current=Current->next;
		if (Current) return &(Current->data);
		else return NULL;
	}

	Tindex CMAPLISTCONV GetId()
	{
		if (Current) return Current->Id;
		else return 0;
	}

	Tdata * CMAPLISTCONV DeleteAndGetNext()
	{
		Item * el;

		if (Current)
		{
			if (Current==First)
			{
				DelFirst();
				Current=First;
			}
			else
			if (Current==Last)
			{
				DelLast();
				Current=NULL;
			}
			else
			{
				el=Current;
				(Current->prev)->next=Current->next;
				(Current->next)->prev=Current->prev;
				Current=Current->next;
				delete el;
				nb--;
			}
			
			if (Current) return &(Current->data);
			else return NULL;
		}
		else return NULL;
	}

	int CMAPLISTCONV Length()
	{
		return nb;
	}

	void CMAPLISTCONV Delete(Tindex n)
	{
		Item * el;
		bool res;

		if (nb>0)
		{
			el=First;
			res=false;
			while ((el->Id!=n)&&(el->next!=NULL))
			{
				el=el->next;
			}

			if (el->Id==n) res=true;
		}
		else res=false;


		if (res)
		{
			if (el==First)
			{
				DelFirst();
			}
			else
			if (el==Last)
			{
				DelLast();
			}
			else
			{
				(el->next)->prev=el->prev;
				(el->prev)->next=el->next;
				delete el;
				nb--;
			}
		}
	}

	void CMAPLISTCONV Add(Tindex n)
	{
		Item * actual;
		if (Last==NULL)
		{
			actual=NULL;
			First=Last=new Item;
			nb=1;
		}
		else
		{
			actual=Last;
			Last->next=new Item;
			Last=Last->next;
			nb++;
		}

		Last->next=NULL;
		Last->prev=actual;
		Last->Id=n;
	}

	void CMAPLISTCONV Add(Tindex n,Tdata dat)
	{
		Item * actual;
		if (Last==NULL)
		{
			actual=NULL;
			First=Last=new Item(dat);
			nb=1;
		}
		else
		{
			actual=Last;
			Last->next=new Item(dat);
			Last=Last->next;
			nb++;
		}

		Last->next=NULL;
		Last->prev=actual;
		Last->Id=n;
	}

	inline Tdata * operator ()(Tindex n)
	{
		Item * el=NULL;
		bool res;

		if (nb>0)
		{
			el=First;
			res=false;
			while ((el->Id!=n)&&(el->next!=NULL))
			{
				el=el->next;
			}

			if (el->Id==n) res=true;
		}
		else res=false;

		if (res)
		{
			return (&el->data);
		}
		else
		{
			return NULL;
		}
	}

	inline Tdata * operator [](Tindex n)
	{
		Item * el=NULL;
		bool res;

		if (nb>0)
		{
			el=First;
			res=false;
			while ((el->Id!=n)&&(el->next!=NULL))
			{
				el=el->next;
			}

			if (el->Id==n) res=true;
		}
		else res=false;

		if (res)
		{
			return (&el->data);
		}
		else
		{
			Add(n);
			return (&Last->data);
		}
	}

	void CMAPLISTCONV Free()
	{
		Item * el,* e;

		el=First;

		while (el!=NULL)
		{
			e=el;
			el=el->next;
			delete e;
		}
		First=NULL;
		Last=NULL;
		nb=0;

	}

	void CMAPLISTCONV DelLast()
	{
		Item * previous;
		
		if (Last!=NULL)
		{
			if (First==Last)
			{
				delete First;
				First=Last=NULL;
			}
			else
			{
				previous=Last;
				Last=Last->prev;
				Last->next=NULL;
				delete previous;
			}
			nb--;
		}
	}

	void CMAPLISTCONV DelFirst()
	{
		Item * previous;
		
		if (First!=NULL)
		{
			if (First==Last)
			{
				delete First;
				First=Last=NULL;
			}
			else
			{
				previous=First;
				First=First->next;
				First->prev=NULL;
				delete previous;
			}
			nb--;
		}
	}

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define TAILLE_BLOCKS 64
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <class T>
class CElement2
{
public:
	T data;
	unsigned int index;
	CElement2 **bloc;
	CElement2 *next;
	CElement2 *prev;

	CElement2() {}
};


template <class T>
class MemItem
{
public:
	CElement2<T>** bloc;
	unsigned int ptr;
	unsigned int nbre;

	MemItem() {}
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <class T>
class CList2 
{
public:
	CElement2 <T> * First;
	CElement2 <T> * Closing;
	CElement2 <T> * Current;
	CElement2 <T> * Current2;

	CList <CElement2<T>*> Blocs;

	typedef MemItem<T> Item;

	CList <Item> Mem;

	int nb;

	void NewBloc()
	{
		CElement2<T>* bloc;
		Item newbloc;

		bloc=new CElement2<T>[TAILLE_BLOCKS];
		newbloc.bloc=Blocs.Add(bloc);
		newbloc.nbre=TAILLE_BLOCKS;
		newbloc.ptr=0;
		Mem.Add(newbloc);
	}

	CElement2<T>* NewElement()
	{
		Item * m;
		CElement2<T>* el;

		m=Mem.GetFirst();
		if (!m) 
		{
			NewBloc();
			m=Mem.GetFirst();
		}

		el=&((*(m->bloc))[m->ptr]);
		el->index=m->ptr;

		m->ptr++;
		m->nbre--;

		if (m->nbre==0)
		{
			Mem.DeleteFirst();
		}
		return el;
	}

	CList2()
	{
		First=NULL;
		Closing=NULL;
		nb=0;
	}

	int Length()
	{
		return nb;
	}

	T * GetFirst()
	{
		Current=First;
		if (Current!=NULL)
			return &(Current->data);
		else return NULL;
	}

	T * GetNext()
	{
		Current=Current->next;
		if (Current!=NULL)
			return &(Current->data);
		else return NULL;
	}

	T * GetFirst2()
	{
		Current2=First;
		if (Current2!=NULL)
			return &(Current2->data);
		else return NULL;
	}

	T * GetNext2()
	{
		Current2=Current2->next;
		if (Current2!=NULL)
			return &(Current2->data);
		else return NULL;
	}

	T* Add(T item)
	{
		CElement2 <T> * el;

		if (Closing!=NULL)
		{
			el=Closing;
			Closing->next=NewElement();
			Closing=Closing->next;
			Closing->data=item;
			Closing->next=NULL;
			Closing->prev=el;
			el=Closing;
			nb++;
		}
		else
		{
			First=NewElement();
			First->data=item;
			Closing=First;
			Closing->next=NULL;
			Closing->prev=NULL;
			el=First;
			nb++;
		}
		return &(el->data);
	}

	T* InsertLast()
	{
		CElement2 <T> * el;

		if (Closing!=NULL)
		{
			el=Closing;
			Closing->next=NewElement();
			Closing=Closing->next;
			Closing->next=NULL;
			Closing->prev=el;
			el=Closing;
			nb++;
		}
		else
		{
			First=NewElement();
			Closing=First;
			Closing->next=NULL;
			Closing->prev=NULL;
			el=First;
			nb++;
		}
		return &(el->data);
	}

	void InsertCurrent(T data)
	{
		CElement2 <T> * el;
		if (Current!=NULL)
		{
			el=NewElement();
			el->data=data;
			el->next=Current->next;
			el->prev=Current;
			Current->next=el;
		}
		nb++;
	}

	void SetCurrent(int n)
	{
		int nn;
		Current=First;
		if (n>0)
		for (nn=0;nn<n;nn++)
			if (Current->next!=NULL) Current=Current->next;
	}

	inline T* operator [](int n)
	{
		int nn;
		CElement2 <T> * el;
		el=First;
		if (n>0)
		for (nn=0;nn<n;nn++)
			if (el->next!=NULL) el=el->next;

		return &(el->data);
	}

	T* Last()
	{
		return &(Closing->data);
	}

	void Del(int n)
	{
		Item newbloc;
		int nn;
		CElement2 <T> *el,*prev,*next;
		int res;
		el=First;
		res=0;
		prev=NULL;
		for (nn=0;nn<n;nn++)
		{
			prev=el;
			if (el->next!=NULL) el=el->next;
			else res=1;
		}

		if (res==0)
		{
			if (el==First)
			{
				DeleteFirst();
			}
			else
				if (el==Closing)
				{
					DeleteLast();
				}
				else
				{
					prev->next=el->next;
					(el->next)->prev=prev;
					newbloc.bloc=el->bloc;
					newbloc.nbre=1;
					newbloc.ptr=el->index;
					Mem.InsertFirst(newbloc);
					nb--;
				}
		}
	}

	void Free()
	{
		CElement2<T>** bloc;

		bloc=Blocs.GetFirst();
		while (bloc)
		{
			delete[] (*bloc);
			bloc=Blocs.GetNext();
		}

		Blocs.Free();
		Mem.Free();

		First=NULL;
		Closing=NULL;

		nb=0;
	}

	void InsertFirst(T data)
	{
		CElement2 <T> * el;

		if (First!=NULL)
		{
			el=First;
			First=NewElement();
			First->data=data;
			First->next=el;
			First->prev=NULL;
			el->prev=First;
		}
		else
		{
			First=Closing=NewElement();
			Closing->next=NULL;
			First->prev=NULL;
			First->data=data;
		}
		nb++;
	}

	void DeleteFirst()
	{
		CElement2 <T> * el;
		Item newbloc;

		if (nb>0)
		{
			el=First;
			First=First->next;

			newbloc.bloc=el->bloc;
			newbloc.nbre=1;
			newbloc.ptr=el->index;
			Mem.InsertFirst(newbloc);

			nb--;
			if (nb==0) 
			{
				First=NULL;
				Closing=NULL;
			}
			else
				First->prev=NULL;
		}
	}

	T* DeleteAndGetNext()
	{
		CElement2 <T> * el,*prev;
		Item newbloc;

		if (nb>0)
		{
			if (Current==First)
			{
				DeleteFirst();
				Current=First;
			}
			else
				if (Current==Closing)
				{
					Current=NULL;
					DeleteLast();
				}
				else
				{			
					el=Current;
					Current=Current->next;
					prev=el->prev;

					Current->prev=prev;
					prev->next=Current;

					newbloc.bloc=el->bloc;
					newbloc.nbre=1;
					newbloc.ptr=el->index;
					Mem.InsertFirst(newbloc);

					nb--;
				}

			if (Current!=NULL)
				return &(Current->data);
			else
				return NULL;
		}
		else return NULL;
	}

	void DeleteLast()
	{
		CElement2 <T> * el;
		Item newbloc;

		if (nb>0)
		{
			el=Closing;
			Closing=Closing->prev;
			Closing->next=NULL;

			
			newbloc.bloc=el->bloc;
			newbloc.nbre=1;
			newbloc.ptr=el->index;
			Mem.InsertFirst(newbloc);
			
			
			nb--;
			if (nb==0)
			{
				First=Closing=NULL;
			}
		}
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <class Tindex,class Tdata> 
class CMapSort
{
public:

	class Item 
	{
	public:
		Tdata	data;			// données
		Tindex  Id;				// index
		Item	*next;			
		Item    *prev;
		Item() {}
		Item(Tdata dat) { data=dat; }
	};

	Item * First;
	Item * Last;
	Item * Current;
	Item * Cro;
	int nb;

	CMapSort() { Cro=First=Last=NULL;nb=0;}

	Tdata * GetFirst()
	{
		if (First)
		{
			Current=First;
			return &(First->data);
		}
		else
			return NULL;
	}

	Tdata * GetNext()
	{
		Current=Current->next;
		if (Current) return &(Current->data);
		else return NULL;
	}

	Tindex GetId()
	{
		if (Current) return Current->Id;
		else return 0;
	}

	Tdata * DeleteAndGetNext()
	{
		Item * el;

		if (Current)
		{
			if (Current==First)
			{
				DelFirst();
				Current=First;
			}
			else
			if (Current==Last)
			{
				DelLast();
				Current=NULL;
			}
			else
			{
				el=Current;
				(Current->prev)->next=Current->next;
				(Current->next)->prev=Current->prev;
				Current=Current->next;
				delete el;
				nb--;
			}
			
			if (Current) return &(Current->data);
			else return NULL;
		}
		else return NULL;
	}

	int Length()
	{
		return nb;
	}

	void Delete(Tindex n)
	{
		Item * el;
		bool res;

		if (nb>0)
		{
			el=First;
			res=false;
			while ((el->Id!=n)&&(el->next!=NULL))
			{
				el=el->next;
			}

			if (el->Id==n) res=true;
		}
		else res=false;


		if (res)
		{
			if (el==First) DelFirst();
			else
			if (el==Last) DelLast();
			else
			{
				(el->next)->prev=el->prev;
				(el->prev)->next=el->next;
				delete el;
				nb--;
			}
		}
	}

	void Add(Tindex n)
	{
		Item * actual;
		if (Last==NULL)
		{
			actual=NULL;
			First=Last=new Item;
			nb=1;
		}
		else
		{
			actual=Last;
			Last->next=new Item;
			Last=Last->next;
			nb++;
		}

		Last->next=NULL;
		Last->prev=actual;
		Last->Id=n;
	}

	void Add(Tindex n,Tdata dat)
	{
		Item * actual;
		if (Last==NULL)
		{
			actual=NULL;
			First=Last=new Item(dat);
			nb=1;
		}
		else
		{
			actual=Last;
			Last->next=new Item(dat);
			Last=Last->next;
			nb++;
		}

		Last->next=NULL;
		Last->prev=actual;
		Last->Id=n;
	}

	inline Tdata * operator ()(Tindex n)
	{
		Item * el=NULL;
		bool res;

		if (nb>0)
		{
			if (Cro)
			{
				el=Cro;
				res=false;
				if (el->Id>n) { while ((el->Id!=n)&&(el->prev)) el=el->prev; }
				else { while ((el->Id!=n)&&(el->next)) el=el->next; }
				if (el->Id==n) res=true;
				Cro=el;
			}
			else
			{
				el=First;
				res=false;
				while ((el->Id!=n)&&(el->next)) el=el->next;
				if (el->Id==n) res=true;
				Cro=el;
			}
		}
		else res=false;

		if (res) return (&el->data);
		else return NULL;
	}

	inline Tdata * operator [](Tindex n)
	{
		Item * actual;
		Item * el=NULL;
		bool res;

		if (nb>0)
		{
			if (Cro)
			{
				el=Cro;
				res=false;
				if (el->Id>n) { while ((el->Id!=n)&&(el->prev)) el=el->prev; }
				else { while ((el->Id!=n)&&(el->next)) el=el->next; }
				if (el->Id==n) res=true;
				Cro=el;
			}
			else
			{
				el=First;
				res=false;
				while ((el->Id!=n)&&(el->next)) el=el->next;
				if (el->Id==n) res=true;
				Cro=el;
			}
		}
		else res=false;

		if (res) return (&el->data);
		else
		{
			if (el)
			{
				if (el->next)
				{
					if (el==First)
					{
						First=actual=new Item;
						actual->Id=n;
						actual->next=el;
						actual->prev=NULL;
						el->prev=actual;
						nb++;
						return (&actual->data);
					}
					else
					{
						actual=new Item;
						actual->Id=n;
						actual->next=el;
						actual->prev=el->prev;
						el->prev->next=actual;
						el->prev=actual;
						nb++;
						return (&actual->data);
					}
				}
				else
				{
					Add(n);
					return (&Last->data);
				}
			}
			else
			{
				Add(n);
				return (&Last->data);
			}
		}
	}

	void Free()
	{
		Item * el,* e;

		el=First;

		while (el!=NULL)
		{
			e=el;
			el=el->next;
			delete e;
		}
		First=NULL;
		Last=NULL;
		nb=0;

	}

	void DelLast()
	{
		Item * previous;
		
		if (Last!=NULL)
		{
			if (First==Last)
			{
				delete First;
				First=Last=NULL;
			}
			else
			{
				previous=Last;
				Last=Last->prev;
				Last->next=NULL;
				delete previous;
			}
			nb--;
		}
	}

	void DelFirst()
	{
		Item * previous;
		
		if (First!=NULL)
		{
			if (First==Last)
			{
				delete First;
				First=Last=NULL;
			}
			else
			{
				previous=First;
				First=First->next;
				First->prev=NULL;
				delete previous;
			}
			nb--;
		}
	}
};

#endif
