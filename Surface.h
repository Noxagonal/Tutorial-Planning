#pragma once

#include "Platform.h"

#include <memory>

class Renderer;
class GraphicsPipeline;

class Surface
{
public:
	Surface( Renderer * renderer, GraphicsPipeline * pipeline );
	virtual ~Surface();

	GraphicsPipeline						*	GetPipeline();

	virtual void						UpdateDescriptorSets()									= 0;
	virtual void						CmdBindDescriptorSets( VkCommandBuffer command_buffer ) = 0;

protected:
	Renderer						*	_ref_renderer					= nullptr;
	GraphicsPipeline						*	_ref_pipeline					= nullptr;
	VkDevice							_ref_vk_device					= VK_NULL_HANDLE;
};
