#include "Surface.h"

#include "Platform.h"
#include "Renderer.h"

Surface::Surface( Renderer * renderer, GraphicsPipeline * pipeline )
{
	_ref_renderer		= renderer;
	_ref_vk_device		= _ref_renderer->GetVulkanDevice();
	_ref_pipeline		= pipeline;
}


Surface::~Surface()
{
}

GraphicsPipeline * Surface::GetPipeline()
{
	return _ref_pipeline;
}
