
/////////////////////////////////////////////////////////////////////////////////////////////////////////
// thanx to alex tardif
/////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DESCHEAP_H_
#define _DESCHEAP_H_

#include "../base/list.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////
class DescriptorHeapHandle
{
public:
    D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE GPUHandle;
    UINT Index;
	bool Active;

	DescriptorHeapHandle()
    {
        CPUHandle.ptr = NULL;
        GPUHandle.ptr = NULL;
        Index = 0;
		Active = false;
    }
 
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() { return CPUHandle; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() { return GPUHandle; }
    UINT GetHeapIndex() { return Index; }
 
    void SetCPUHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle) { CPUHandle = cpuHandle; }
    void SetGPUHandle(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) { GPUHandle = gpuHandle; }
    void SetHeapIndex(UINT heapIndex) { Index = heapIndex; Active=true; }
 };

/////////////////////////////////////////////////////////////////////////////////////////////////////////
class DescriptorHeapHandleContainer
{
public:
    ID3D12DescriptorHeap *DescriptorHeap;
    D3D12_DESCRIPTOR_HEAP_TYPE HeapType;
    D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeapCPUStart;
    D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeapGPUStart;
    UINT MaxDescriptors;
    UINT DescriptorSize;
    CList<UINT> recycle;
    UINT CurrentDescriptorIndex;
	ID3D12Device* d3ddevice;
	void *render;

	DescriptorHeapHandleContainer();
	~DescriptorHeapHandleContainer();

    void Init(ID3D12Device *device, void *api, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors);
 
	ID3D12DescriptorHeap *GetHeap() { return DescriptorHeap; }
    D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType() { return HeapType; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetHeapCPUStart() { return DescriptorHeapCPUStart; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetHeapGPUStart() { return DescriptorHeapGPUStart; }
    UINT GetMaxDescriptors() { return MaxDescriptors; }
    UINT GetDescriptorSize() { return DescriptorSize; }
 
    DescriptorHeapHandle GetNewHeapHandle();
	DescriptorHeapHandle GetNewHeapHandleNotRecycle();
    void FreeHeapHandle(DescriptorHeapHandle handle);
	void Reset();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////
class CResource
{
public:
	DescriptorHeapHandle id;
	ID3D12Device* device;
	DescriptorHeapHandleContainer *heapref;
	ID3D12Resource * res;
	UINT64 pointer;
	UINT8* pdata;

	CResource(DescriptorHeapHandleContainer *heap)
	{
		heapref=heap;
		device=heap->d3ddevice;
		id=heap->GetNewHeapHandle();
        res=NULL;
	}

	~CResource()
	{
        Release();
	}

	void CreateConstantBuffer(void *data,int size);
	void CreateConstantBufferPadding(void *data,int size);
	void Release();
	void Update(void *data,int size);
};

extern DescriptorHeapHandle zeroid;

#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
