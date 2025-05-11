/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//#define BIGRESOURCE
/////////////////////////////////////////////////////////////////////////////////////////////////////////
DescriptorHeapHandleContainer::DescriptorHeapHandleContainer()
{
	CurrentDescriptorIndex = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void DescriptorHeapHandleContainer::Reset()
{
	CurrentDescriptorIndex = 0;
	recycle.Free();
	if (DescriptorHeap) DescriptorHeap->Release();
	DescriptorHeap = NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void DescriptorHeapHandleContainer::Init(ID3D12Device *device, void *api, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors)
{
	CurrentDescriptorIndex = 0;

	d3ddevice=device;
	render=api;

	HeapType = heapType;
	MaxDescriptors = numDescriptors;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	heapDesc.NumDescriptors = MaxDescriptors;
	heapDesc.Type = HeapType;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;
 
	device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&DescriptorHeap));
 
	DescriptorHeapCPUStart = DescriptorHeap->GetCPUDescriptorHandleForHeapStart(); 
	DescriptorHeapGPUStart = DescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	DescriptorSize = device->GetDescriptorHandleIncrementSize(HeapType);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
DescriptorHeapHandleContainer::~DescriptorHeapHandleContainer()
{
	recycle.Free();
	if (DescriptorHeap) DescriptorHeap->Release();
	DescriptorHeap = NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
DescriptorHeapHandle DescriptorHeapHandleContainer::GetNewHeapHandle()
{
	UINT newHandleID = 0;
 
	if (CurrentDescriptorIndex < MaxDescriptors)
	{
		newHandleID = CurrentDescriptorIndex;
		CurrentDescriptorIndex++;
	}
	else
	if (recycle.Length() > 0)
	{
		newHandleID = *recycle.Last();
		recycle.DeleteLast();
	}
 
	DescriptorHeapHandle newHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = DescriptorHeapCPUStart;
	cpuHandle.ptr += newHandleID * DescriptorSize;
	newHandle.SetCPUHandle(cpuHandle);

	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = DescriptorHeapGPUStart;
	gpuHandle.ptr += newHandleID * DescriptorSize;
	newHandle.SetGPUHandle(gpuHandle);

	newHandle.SetHeapIndex(newHandleID);

	return newHandle;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
DescriptorHeapHandle DescriptorHeapHandleContainer::GetNewHeapHandleNotRecycle()
{
	UINT newHandleID = 0;
 
	if (CurrentDescriptorIndex < MaxDescriptors)
	{
		newHandleID = CurrentDescriptorIndex;
		CurrentDescriptorIndex++;
	}
	else
	if (recycle.Length() > 0)
	{
		newHandleID = *recycle.Last();
		recycle.DeleteLast();
	}
 
	DescriptorHeapHandle newHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = DescriptorHeapCPUStart;
	cpuHandle.ptr += newHandleID * DescriptorSize;
	newHandle.SetCPUHandle(cpuHandle);

	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = DescriptorHeapGPUStart;
	gpuHandle.ptr += newHandleID * DescriptorSize;
	newHandle.SetGPUHandle(gpuHandle);

	newHandle.SetHeapIndex(newHandleID);

	return newHandle;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void DescriptorHeapHandleContainer::FreeHeapHandle(DescriptorHeapHandle handle)
{
	if (handle.Active) recycle.InsertFirst(handle.GetHeapIndex());
	handle.Active=false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RESOURCES
/////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CResource::CreateConstantBuffer(void *data,int size)
{
	C3DAPIBASE *api=((C3DAPIBASE*)heapref->render);

	device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),D3D12_HEAP_FLAG_NONE,&CD3DX12_RESOURCE_DESC::Buffer(size),D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr,IID_PPV_ARGS(&res));

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = res->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = size;

	api->DynamicResourcesId[api->frameIndex].Add(id);
	id=heapref->GetNewHeapHandle();

	D3D12_CPU_DESCRIPTOR_HANDLE h=id.GetCPUHandle();
    device->CreateConstantBufferView(&cbvDesc, h);

	api->DynamicResources[((C3DAPIBASE*)heapref->render)->frameIndex].Add(res);

	if (data)
	{
	    CD3DX12_RANGE readRange(0, 0);
		res->Map(0, &readRange, reinterpret_cast<void**>(&pdata));
		memcpy(pdata, data, size);
		res->Unmap(0, nullptr);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CResource::CreateConstantBufferPadding(void *data,int basesize)
{
	int shr=(basesize+255)/256;
	int size=256*shr;

	C3DAPIBASE *api=((C3DAPIBASE*)heapref->render);
	int frame=api->frameIndex;
#ifndef BIGRESOURCE

	ID3D12Resource **pcst=(ID3D12Resource **)api->ConstantBuffers[frame][shr].Current;
	DescriptorHeapHandle *pid=(DescriptorHeapHandle *)api->ConstantBuffersId[frame][shr].Current;
	UINT8 **pptr=(UINT8 **)api->ConstantBuffersPtr[frame][shr].Current;
	if (pcst)
	{
		res=*pcst;		
		id=*pid;
		pdata=*pptr;
		api->ConstantBuffers[frame][shr].GetNext();
		api->ConstantBuffersId[frame][shr].GetNext();
		api->ConstantBuffersPtr[frame][shr].GetNext();
	}
	else
	{
		device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),D3D12_HEAP_FLAG_NONE,&CD3DX12_RESOURCE_DESC::Buffer(size),D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr,IID_PPV_ARGS(&res));		
		api->ConstantBuffers[frame][shr].Add(res);

		id=heapref->GetNewHeapHandle();
		api->ConstantBuffersId[frame][shr].Add(id);

		CD3DX12_RANGE readRange(0, 0);
		res->Map(0, &readRange, reinterpret_cast<void**>(&pdata));

		api->ConstantBuffersPtr[frame][shr].Add(pdata);
	}

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	pointer = cbvDesc.BufferLocation = res->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = size;
	D3D12_CPU_DESCRIPTOR_HANDLE h=id.GetCPUHandle();
	device->CreateConstantBufferView(&cbvDesc, h);

	memcpy(pdata, data, basesize);

#endif

#ifdef BIGRESOURCE

	if (api->ofsConstantBuffers[frame]+size>HEAP_DYNAMIC_BUFFERS_SIZE)
	{
		api->CloseOpenedCommandListFence();
		api->ofsConstantBuffers[frame]=0;
	}

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	res=api->heapConstantBuffers[frame];
    pointer = cbvDesc.BufferLocation = res->GetGPUVirtualAddress()+api->ofsConstantBuffers[frame];
    cbvDesc.SizeInBytes = size;
	
	api->DynamicResourcesId[api->frameIndex].Add(id);
	id=heapref->GetNewHeapHandle();

	D3D12_CPU_DESCRIPTOR_HANDLE h=id.GetCPUHandle();
    device->CreateConstantBufferView(&cbvDesc, h);
#endif

	if (data)
	{
#ifdef BIGRESOURCE
		pdata=api->pdataConstantBuffer[frame]+api->ofsConstantBuffers[frame];
		memcpy(pdata, data, basesize);
		api->ofsConstantBuffers[frame]+=size;
#else
//		CD3DX12_RANGE readRange(0, 0);
//		res->Map(0, &readRange, reinterpret_cast<void**>(&pdata));
//		memcpy(pdata, data, basesize);
//		res->Unmap(0, nullptr);
#endif
	}

	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CResource::Release()
{
	//heapref->FreeHeapHandle(id);
	//if (res) res->Release();
    res=NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CResource::Update(void *data,int size)
{
	if (data)
	{
		CD3DX12_RANGE readRange(0, 0);
		res->Map(0, &readRange, reinterpret_cast<void**>(&pdata));
 		memcpy(pdata, data, size);
		res->Unmap(0, nullptr);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
