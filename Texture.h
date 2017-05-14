#pragma once

#include "Platform.h"

class Renderer;

class Texture
{
public:
	Texture( Renderer * renderer, std::wstring path );
	~Texture();

	VkImage						GetVulkanImage();
	VkImageView					GetVulkanImageView();
	VkSampler					GetVulkanSampler();
	VkFormat					GetFormat();

private:
	Renderer				*	_ref_renderer			= nullptr;
	VkDevice					_ref_vk_device			= VK_NULL_HANDLE;

	VkImage						_image					= VK_NULL_HANDLE;
	VkImageView					_image_view				= VK_NULL_HANDLE;
	VkDeviceMemory				_image_memory			= VK_NULL_HANDLE;
	VkSampler					_sampler				= VK_NULL_HANDLE;

	VkExtent2D					_size					= { 0, 0 };
	VkFormat					_image_format			= VK_FORMAT_UNDEFINED;
};
