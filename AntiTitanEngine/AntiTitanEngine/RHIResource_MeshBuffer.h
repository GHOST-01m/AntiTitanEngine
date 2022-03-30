#pragma once
class RHIResource_MeshBuffer {
public:
	virtual ~RHIResource_MeshBuffer();
public:
	std::string MeshName;
	std::vector<Vertex> vertices;
	std::vector<int32_t> indices;
};