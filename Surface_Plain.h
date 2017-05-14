#pragma once

#include "Surface.h"

class Renderer;
class Window;
class Texture;

struct Surface_Plain_DescriptorSetInfo
{
	VkDescriptorSet				descriptor_set			= VK_NULL_HANDLE;
};

class Surface_Plain :
	public Surface
{
public:
	Surface_Plain( Renderer * renderer, GraphicsPipeline * pipeline, Texture * texture );
	~Surface_Plain();

	void								UpdateDescriptorSets();
	void								CmdBindDescriptorSets( VkCommandBuffer command_buffer );

private:
	Texture							*	_ref_texture					= nullptr;

	Surface_Plain_DescriptorSetInfo		_shader_data_info;
};
