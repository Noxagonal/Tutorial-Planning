#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct ME3D_Header
{
	int32_t		file_id;
	int32_t		head_size;
	int32_t		file_size;

	int32_t		vert_count;
	int32_t		vert_size;
	int32_t		vert_location;

	int32_t		vert_copy_count;
	int32_t		vert_copy_size;
	int32_t		vert_copy_location;

	int32_t		polygon_count;
	int32_t		polygon_size;
	int32_t		polygon_location;

private:
	int32_t		padding[ 4 ];
};

struct ME3D_MetaData
{
	std::string			identifier;
	std::string			data;
};

struct ME3D_Vertex
{
	float		position[ 3 ];
	int16_t		normals[ 3 ];
	int16_t		uvs[ 2 ];
	int16_t		material_index;
};

struct ME3D_VertexCopy
{
	int32_t		copy_from_index;
	int16_t		uvs[ 2 ];
	int16_t		material_index;
};

struct ME3D_Polygon
{
	int32_t		indices[ 3 ];
};

uint16_t ME3D_FloatToShort( float value );
float ME3D_ShortToFloat( uint16_t value );

class ME3D_File
{
public:
	ME3D_File();
	ME3D_File( std::string path );
	~ME3D_File();

	bool										Load( std::string path );		// returns true if successfully loaded a file
	const std::vector<ME3D_Vertex>			&	GetVertices() const;
	const std::vector<ME3D_VertexCopy>		&	GetCopyVertices() const;
	const std::vector<ME3D_Polygon>			&	GetPolygons() const;

	bool										IsLoaded() const;

private:
	ME3D_Header									head;

	std::vector<ME3D_Vertex>					vertices;
	std::vector<ME3D_VertexCopy>				vertex_copies;
	std::vector<ME3D_Polygon>					polygons;

	bool										is_loaded					= false;
};
