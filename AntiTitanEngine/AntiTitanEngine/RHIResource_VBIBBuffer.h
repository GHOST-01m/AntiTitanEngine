#pragma once
class RHIResource_VBIBBuffer {
public:
	virtual ~RHIResource_VBIBBuffer();
public:
	std::string MeshName;
	std::vector<Vertex> vertices;
	std::vector<int32_t> indices;
};