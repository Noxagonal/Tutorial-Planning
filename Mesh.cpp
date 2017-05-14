#include "Mesh.h"

#include "Platform.h"

#include "ME3DFile.h"

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

void Mesh::GenerateShape( MESH_OBJECT_SHAPE shape )
{
	vertices.clear();
	triangles.clear();

	switch( shape ) {
	case MESH_OBJECT_SHAPE::NONE:
		break;
	case MESH_OBJECT_SHAPE::TRIANGLE:
	{
		vertices.reserve( 3 );
		triangles.reserve( 1 );

		vertices.push_back( {
			{ 0.0f, -0.5f, 0.0f },
			{ 1.0f, 0.0f, 0.0f },
			{ 0.0f, 0.5f }
		} );
		vertices.push_back( {
			{ -0.5f, 0.5f, 0.0f },
			{ 0.0f, 1.0f, 0.0f },
			{ 1.0f, 0.0f }
		} );
		vertices.push_back( {
			{ 0.5f, 0.5f, 0.0f },
			{ 0.0f, 0.0f, 1.0f },
			{ 1.0f, 1.0f }
		} );

		triangles.push_back( {
			{ 0, 1, 2 }
		} );
	}
		break;
	case MESH_OBJECT_SHAPE::PLANE:
	{
		vertices.reserve( 4 );
		triangles.reserve( 2 );

		vertices.push_back( {
			{ -0.5f, -0.5f, 0.0f },
			{ 1.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f }
		} );
		vertices.push_back( {
			{ 0.5f, -0.5f, 0.0f },
			{ 0.0f, 1.0f, 0.0f },
			{ 1.0f, 0.0f }
		} );
		vertices.push_back( {
			{ -0.5f, 0.5f, 0.0f },
			{ 0.0f, 0.0f, 1.0f },
			{ 0.0f, 1.0f }
		} );
		vertices.push_back( {
			{ 0.5f, 0.5f, 0.0f },
			{ 1.0f, 1.0f, 1.0f },
			{ 1.0f, 1.0f }
		} );

		triangles.push_back( {
			{ 0, 2, 1 }
		} );
		triangles.push_back( {
			{ 2, 3, 1 }
		} );
	}
		break;
	case MESH_OBJECT_SHAPE::CUBE:
	{
		vertices.reserve( 24 );
		triangles.reserve( 12 );

		// front
		vertices.push_back( {
			{ -0.5f, -0.5f, -0.5f },
			{ 1.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f }
		} );
		vertices.push_back( {
			{ 0.5f, -0.5f, -0.5f },
			{ 0.0f, 1.0f, 0.0f },
			{ 1.0f, 0.0f }
		} );
		vertices.push_back( {
			{ -0.5f, 0.5f, -0.5f },
			{ 0.0f, 0.0f, 1.0f },
			{ 0.0f, 1.0f }
		} );
		vertices.push_back( {
			{ 0.5f, 0.5f, -0.5f },
			{ 1.0f, 1.0f, 1.0f },
			{ 1.0f, 1.0f }
		} );

		// back
		vertices.push_back( {
			{ 0.5f, -0.5f, 0.5f },
			{ 1.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f }
		} );
		vertices.push_back( {
			{ -0.5f, -0.5f, 0.5f },
			{ 0.0f, 1.0f, 0.0f },
			{ 1.0f, 0.0f }
		} );
		vertices.push_back( {
			{ 0.5f, 0.5f, 0.5f },
			{ 0.0f, 0.0f, 1.0f },
			{ 0.0f, 1.0f }
		} );
		vertices.push_back( {
			{ -0.5f, 0.5f, 0.5f },
			{ 1.0f, 1.0f, 1.0f },
			{ 1.0f, 1.0f }
		} );

		// right
		vertices.push_back( {
			{ 0.5f, -0.5f, -0.5f },
			{ 1.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f }
		} );
		vertices.push_back( {
			{ 0.5f, -0.5f, 0.5f },
			{ 0.0f, 1.0f, 0.0f },
			{ 1.0f, 0.0f }
		} );
		vertices.push_back( {
			{ 0.5f, 0.5f, -0.5f },
			{ 0.0f, 0.0f, 1.0f },
			{ 0.0f, 1.0f }
		} );
		vertices.push_back( {
			{ 0.5f, 0.5f, 0.5f },
			{ 1.0f, 1.0f, 1.0f },
			{ 1.0f, 1.0f }
		} );

		// right
		vertices.push_back( {
			{ -0.5f, -0.5f, 0.5f },
			{ 1.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f }
		} );
		vertices.push_back( {
			{ -0.5f, -0.5f, -0.5f },
			{ 0.0f, 1.0f, 0.0f },
			{ 1.0f, 0.0f }
		} );
		vertices.push_back( {
			{ -0.5f, 0.5f, 0.5f },
			{ 0.0f, 0.0f, 1.0f },
			{ 0.0f, 1.0f }
		} );
		vertices.push_back( {
			{ -0.5f, 0.5f, -0.5f },
			{ 1.0f, 1.0f, 1.0f },
			{ 1.0f, 1.0f }
		} );

		// top
		vertices.push_back( {
			{ -0.5f, -0.5f, -0.5f },
			{ 1.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f }
		} );
		vertices.push_back( {
			{ 0.5f, -0.5f, -0.5f },
			{ 0.0f, 1.0f, 0.0f },
			{ 1.0f, 0.0f }
		} );
		vertices.push_back( {
			{ -0.5f, -0.5f, 0.5f },
			{ 0.0f, 0.0f, 1.0f },
			{ 0.0f, 1.0f }
		} );
		vertices.push_back( {
			{ 0.5f, -0.5f, 0.5f },
			{ 1.0f, 1.0f, 1.0f },
			{ 1.0f, 1.0f }
		} );

		// bottom
		vertices.push_back( {
			{ 0.5f, 0.5f, 0.5f },
			{ 1.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f }
		} );
		vertices.push_back( {
			{ -0.5f, 0.5f, 0.5f },
			{ 0.0f, 1.0f, 0.0f },
			{ 1.0f, 0.0f }
		} );
		vertices.push_back( {
			{ 0.5f, 0.5f, -0.5f },
			{ 0.0f, 0.0f, 1.0f },
			{ 0.0f, 1.0f }
		} );
		vertices.push_back( {
			{ -0.5f, 0.5f, -0.5f },
			{ 1.0f, 1.0f, 1.0f },
			{ 1.0f, 1.0f }
		} );

		// front
		uint32_t offset = 0;
		triangles.push_back( {
			{ 0 + offset, 2 + offset, 1 + offset }
		} );
		triangles.push_back( {
			{ 1 + offset, 2 + offset, 3 + offset }
		} );

		// back
		offset = 4 * 1;
		triangles.push_back( {
			{ 0 + offset, 2 + offset, 1 + offset }
		} );
		triangles.push_back( {
			{ 1 + offset, 2 + offset, 3 + offset }
		} );

		// right
		offset = 4 * 2;
		triangles.push_back( {
			{ 0 + offset, 2 + offset, 1 + offset }
		} );
		triangles.push_back( {
			{ 1 + offset, 2 + offset, 3 + offset }
		} );

		// left
		offset = 4 * 3;
		triangles.push_back( {
			{ 0 + offset, 2 + offset, 1 + offset }
		} );
		triangles.push_back( {
			{ 1 + offset, 2 + offset, 3 + offset }
		} );

		// top
		offset = 4 * 4;
		triangles.push_back( {
			{ 0 + offset, 2 + offset, 1 + offset }
		} );
		triangles.push_back( {
			{ 1 + offset, 2 + offset, 3 + offset }
		} );

		// bottom
		offset = 4 * 5;
		triangles.push_back( {
			{ 0 + offset, 2 + offset, 1 + offset }
		} );
		triangles.push_back( {
			{ 1 + offset, 2 + offset, 3 + offset }
		} );
	}
	break;
	default:
		break;
	}
}

void Mesh::Load( std::string path )
{
	ME3D_File file( path );
	if( !file.IsLoaded() ) return;
	auto & me3d_vertices		= file.GetVertices();
	auto & me3d_polygons		= file.GetPolygons();

	vertices.empty();
	triangles.empty();
	vertices.resize( me3d_vertices.size() );
	triangles.resize( me3d_polygons.size() );

	for( size_t i=0; i < me3d_vertices.size(); ++i ) {
		auto & dst = vertices[ i ];
		auto & src = me3d_vertices[ i ];
		dst.position[ 0 ]			= src.position[ 0 ];
		dst.position[ 1 ]			= src.position[ 1 ];
		dst.position[ 2 ]			= src.position[ 2 ];
		dst.color[ 0 ]				= 0.5f;
		dst.color[ 1 ]				= 0.5f;
		dst.color[ 2 ]				= 0.5f;
		dst.uv[ 0 ]					= ME3D_ShortToFloat( src.uvs[ 0 ] );
		dst.uv[ 1 ]					= ME3D_ShortToFloat( src.uvs[ 1 ] );
	}
	for( size_t i=0; i < me3d_polygons.size(); ++i ) {
		auto & dst = triangles[ i ];
		auto & src = me3d_polygons[ i ];
		dst.indices[ 0 ]			= src.indices[ 0 ];
		dst.indices[ 1 ]			= src.indices[ 1 ];
		dst.indices[ 2 ]			= src.indices[ 2 ];
	}
}

uint32_t Mesh::GetVerticesByteSize()
{
	return vertices.size() * sizeof( Vertex );
}

uint32_t Mesh::GetIndicesByteSize()
{
	return triangles.size() * sizeof( Triangle );
}
