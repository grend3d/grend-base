#pragma once

#include <grend/ecs/ecs.hpp>
#include <grend/camera.hpp>

using namespace grendx;
using namespace grendx::ecs;

class playerCameraSystem : public entitySystem {
	public:
		typedef std::shared_ptr<playerCameraSystem> ptr;
		typedef std::weak_ptr<playerCameraSystem>   weakptr;

		virtual ~playerCameraSystem();
		virtual void update(entityManager *manager, float delta);

		camera::ptr cam = std::make_shared<camera>();
};

