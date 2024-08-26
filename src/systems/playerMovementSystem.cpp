#include <systems/playerMovementSystem.hpp>
#include <systems/playerCameraSystem.hpp>

#include <grend/interpolation.hpp>
#include <grend/scancodes.hpp>
#include <grend/ecs/rigidBody.hpp>
#include <grend/ecs/animationController.hpp>

using namespace grendx;
using namespace grendx::ecs;
using namespace grendx::engine;
using namespace grendx::interp;

playerMovementSystem::~playerMovementSystem() {};

void playerMovementSystem::update(entityManager *entities, float delta) {
	static smoothed<glm::vec3> pos;

	pos.update(delta);

	static auto up   = keyButton(SDL_SCANCODE_UP);
	static auto down = keyButton(SDL_SCANCODE_DOWN);

	auto temp = entities->search<rigidBody, animationController, sceneComponent>();
	for (auto [ent, body, animations, _] : temp) {
		// TODO: don't like this
		const auto playerCam = std::static_pointer_cast<playerCameraSystem>(entities->systems["playerCamera"])->cam;

		glm::vec3 vel;
		float curSpeed = glm::length(body->phys->getVelocity());

		if (curSpeed > 2) {
			animations->setAnimation("run");

		} else {
			animations->setAnimation("idle");
		}

		if (keyIsPressed(up)) {
			glm::vec3 dir = glm::cross(playerCam->right(), glm::vec3(0, 1, 0));
			glm::vec3 foo = glm::normalize(body->phys->getVelocity());
			//glm::vec3 foo = body->phys->getVelocity();
			//LogFmt("Speed: {}", curSpeed);
			//LogFmt("Speed: ({}, {}, {}), {}", foo.x, foo.y, foo.z, glm::length(foo));

			float asdf = 2.f - (glm::dot(foo, dir) + 1.f);
			vel = dir * 10.f*(1.f + 10*asdf);
			body->phys->setAcceleration(vel);
		}

		if (keyIsPressed(down)) {
			glm::vec3 dir = glm::cross(playerCam->right(), glm::vec3(0, 1, 0));
			vel = dir * -10.f;
			body->phys->setVelocity(vel);
		}

		break;
	}
}
