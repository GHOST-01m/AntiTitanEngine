#pragma once
class Primitive_MeshBuffer {
public:
	virtual ~Primitive_MeshBuffer();
public:
	std::string MeshName;
	std::vector<Vertex> vertices;
	std::vector<int32_t> indices;
};