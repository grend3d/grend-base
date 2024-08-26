#include <systems/playerCameraSystem.hpp>
#include <grend/interpolation.hpp>
#include <grend/scancodes.hpp>
#include <grend/ecs/rigidBody.hpp>
#include <grend/ecs/animationController.hpp>

using namespace grendx;
using namespace grendx::ecs;
using namespace grendx::engine;
using namespace grendx::interp;

playerCameraSystem::~playerCameraSystem() {};

void playerCameraSystem::update(entityManager *entities, float delta) {
	static smoothed<float> speed(0, 128);
	static smoothed<float> rads(0);

	static auto left  = keyButton(SDL_SCANCODE_LEFT);
	static auto right = keyButton(SDL_SCANCODE_RIGHT);

	speed.update(delta);
	rads.update(delta);

	auto temp = entities->search<rigidBody, sceneComponent, animationController>();
	for (auto [ent, body, _, __] : temp) {
		glm::vec3 pos = ent->transform.getTRS().position;
		glm::vec3 dir = cam->direction();

		cam->setPosition(pos - dir*10.f);

		if (keyIsPressed(left))  rads += 0.1;
		if (keyIsPressed(right)) rads -= 0.1;

		cam->setDirection(glm::vec3 {
			cos(rads),
			-0.71,
			sin(rads)
		}, glm::vec3(0, 1, 0));

		//speed = glm::length(body->phys->getVelocity());
		speed = 1.0;

		cam->setFovx(80 + 4 * min(*speed, 40.f));
		break;
	}
}

