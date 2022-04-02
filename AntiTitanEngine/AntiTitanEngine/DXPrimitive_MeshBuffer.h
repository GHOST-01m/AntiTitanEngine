#pragma once
#include "MyStruct.h"

class DXPrimitive_MeshBuffer :public Primitive_MeshBuffer {

public:
	//~DXRHIResource_VBIBBuffer();

public:
	std::string MeshName;
	std::vector<Vertex> vertices;
	std::vector<int32_t> indices;

public:
	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

public:
	UINT VertexByteStride = 0;
	UINT VertexBufferByteSize = 0;
	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
	UINT IndexBufferByteSize = 0;

public:
	std::unordered_map<std::string, SubmeshGeometry> DrawArgs;

public:
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView()const {

		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
		vbv.StrideInBytes = VertexByteStride;
		vbv.SizeInBytes = VertexBufferByteSize;

		return vbv;
	};
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView()const {
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
		ibv.Format = IndexFormat;
		ibv.SizeInBytes = IndexBufferByteSize;

		return ibv;
	};
};

