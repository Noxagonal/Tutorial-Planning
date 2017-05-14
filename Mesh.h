#pragma once

#include <stdint.h>
#include <vector>
#include <string>

enum class MESH_OBJECT_SHAPE
{
	NONE,
	TRIANGLE,
	PLANE,
	CUBE,
};

struct Vertex
{
	float position[ 3 ];
	float color[ 3 ];
	float uv[ 2 ];
};

struct Triangle
{
	uint32_t indices[ 3 ];
};

class Mesh
{
public:
	Mesh();
	~Mesh();

	void					GenerateShape( MESH_OBJECT_SHAPE shape );
	void					Load( std::string path );

	uint32_t				GetVerticesByteSize();
	uint32_t				GetIndicesByteSize();

	std::vector<Vertex>		vertices;
	std::vector<Triangle>	triangles;
};
