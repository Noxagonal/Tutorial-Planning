#include "SceneObject.h"

#include "Platform.h"
#include "Renderer.h"
#include "Shared.h"

SceneObject::SceneObject( Renderer * renderer )
{
	assert( nullptr != renderer );
	_ref_renderer		= renderer;
	_ref_vk_device		= _ref_renderer->GetVulkanDevice();
}


SceneObject::~SceneObject()
{
}

glm::mat4 SceneObject::CalculateTransformationMatrix()
{
	glm::mat4 ret = glm::mat4( 1 );
	ret = glm::translate( ret, position );
	ret *= glm::mat4_cast( rotation );
	ret = glm::scale( ret, size );
	return ret;
}
