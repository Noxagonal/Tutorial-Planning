#pragma once

#include "Platform.h"

class Renderer;

struct UBOData_Camera
{
	glm::mat4 View_Matrix;
	glm::mat4 Projection_Matrix;
};

struct UBOData_Object
{
	glm::mat4 Model_Matrix;
};

class SceneObject
{
public:
	SceneObject( Renderer * renderer );
	virtual ~SceneObject();

	glm::vec3					position				= glm::vec3( 0, 0, 0 );
	glm::vec3					size					= glm::vec3( 1, 1, 1 );
	glm::quat					rotation				= glm::quat( 1, 0, 0, 0 );

	glm::mat4					CalculateTransformationMatrix();

	virtual void				UpdateLogic()								= 0;
	virtual void				CmdRender( VkCommandBuffer command_buffer )	= 0;

protected:
	Renderer				*	_ref_renderer							= nullptr;
	VkDevice					_ref_vk_device							= VK_NULL_HANDLE;
};
