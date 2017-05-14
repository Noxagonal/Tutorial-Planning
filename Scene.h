#pragma once

#include "Platform.h"

#include <list>
#include <memory>

class SceneObject;
class SceneObject_Camera;

class Scene
{
public:
	Scene();
	~Scene();

	void								UpdateLogic();
	void								CmdRender( VkCommandBuffer command_buffer );

	void								AddObject( SceneObject * object );
	void								RemoveObject( SceneObject * object );

private:
	std::list<SceneObject*>				objects;
};
