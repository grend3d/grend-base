#include <grend/gameMainDevWindow.hpp>
#include <grend/scancodes.hpp>
#include <grend/loadScene.hpp>
#include <grend/renderUtils.hpp>
#include <grend/interpolation.hpp>

#include <grend/ecs/sceneComponent.hpp>
#include <grend/ecs/collision.hpp>
#include <grend/ecs/rigidBody.hpp>
#include <grend/ecs/shader.hpp>

#include <iostream>

using namespace grendx;
using namespace grendx::ecs;
using namespace grendx::interp;

renderPostChain::ptr createPost(camera::ptr cam, gameMain *game) {
	auto rend = game->services.resolve<renderContext>();

	return std::make_shared<renderPostChain>(
		std::vector {
			loadPostShader(GR_PREFIX "shaders/baked/tonemap.frag",
			               rend->globalShaderOptions)
		},
		game->settings.targetResX, game->settings.targetResY
	);
}

class gamething : public gameView {
	renderPostChain::ptr post;

	public:
		gamething(gameMain *game) {
			glm::vec3 dir = glm::normalize(glm::vec3(0.71, -0.71, 0.71));
			cam->setDirection(dir);
			cam->setFar(1000);

			post = createPost(cam, game);
		};

		virtual void handleEvent(gameMain *game, const SDL_Event& ev) {
		}

		virtual void update(gameMain *game, float delta) {
			static smoothed<float> rads(0);
			static smoothed<float> speed(0, 128);
			static smoothed<glm::vec3> pos;

			static auto left  = keyButton(SDL_SCANCODE_LEFT);
			static auto right = keyButton(SDL_SCANCODE_RIGHT);
			static auto up    = keyButton(SDL_SCANCODE_UP);
			static auto down  = keyButton(SDL_SCANCODE_DOWN);
			static auto space = keyButton(SDL_SCANCODE_SPACE);
			static auto meh   = mouseButton(SDL_BUTTON_LEFT);

			auto phys     = game->services.resolve<physics>();
			auto entities = game->services.resolve<ecs::entityManager>();

			rads.update(delta);
			speed.update(delta);
			pos.update(delta);

			phys->stepSimulation(delta);
			phys->filterCollisions();;

			entities->update(delta);

			auto temp = entities->search<rigidBody, syncRigidBodyXZVelocity>();
			for (auto [ent, body, _] : temp) {
				if (entities->condemned.count(ent)) {
					std::cout << "[deleted] " << std::endl;
					continue;
				}

				if (!ent->active)
					continue;

				glm::vec3 pos = ent->node->getTransformTRS().position;
				glm::vec3 dir = cam->direction();
				static smoothed<float> x;

				x = 8.f * keyIsPressed(meh);
				x.update(delta);
				cam->setPosition((glm::vec3)pos - dir*(10.f + (*x)*(*x)));

				glm::vec3 vel;

				if (keyIsPressed(left))  rads += 0.1; 
				if (keyIsPressed(right)) rads -= 0.1; 

				cam->setDirection(glm::vec3 {
					cos(rads),
					-0.71,
					sin(rads)
				});

				speed = glm::length(body->phys->getVelocity());

				if (keyIsPressed(up)) {
					glm::vec3 dir = glm::cross(cam->right(), glm::vec3(0, 1, 0));
					glm::vec3 foo = glm::normalize(body->phys->getVelocity());
					float asdf = 2.f - (glm::dot(foo, dir) + 1.f);
					vel = dir * 10.f*(1.f + 10*asdf);
					body->phys->setAcceleration(vel);
				}

				if (keyIsPressed(down)) {
					glm::vec3 dir = glm::cross(cam->right(), glm::vec3(0, 1, 0));
					vel = dir * -10.f;
					body->phys->setVelocity(vel);
				}

				cam->setFovx(80 + 4 * min(*speed, 40.f));
				break;
			}
		};

		virtual void render(gameMain *game, renderFramebuffer::ptr fb) {
			auto rend  = game->services.resolve<renderContext>();
			auto state = game->services.resolve<gameState>();

			auto que = buildDrawableQueue(game);
			que.add(rend->getLightingFlags(), state->rootnode);
			drawMultiQueue(game, que, rend->framebuffer, cam);

			rend->defaultSkybox.draw(cam, rend->framebuffer);
			setPostUniforms(post, game, cam);
			post->draw(rend->framebuffer);
		};
};

int main(int argc, char **argv) {
	//unsigned x = 1280;
	//unsigned y = 720;
	unsigned x = 1920;
	unsigned y = 1080;

	//gameMain *game = new gameMainDevWindow({ .msaaLevel = 0, .UIScale = 2.0 });
	gameMainDevWindow *game = new gameMainDevWindow({
		.scaleX     = 1.0,
		.scaleY     = 1.0,
		//.scaleX     = 1.0,
		//.scaleY     = 1.0,
		//.targetResX = 2560,
		//.targetResY = 1440,
		.targetResX = x,
		.targetResY = y,
		.msaaLevel  = 0,
		//.windowResX = 2560,
		//.windowResY = 1440,
		.windowResX = x,
		.windowResY = y,
		.vsync      = 1,
		.UIScale    = 2.0,
		//.UIScale    = 1.0,
	});

	auto state     = game->services.resolve<gameState>();
	auto rend      = game->services.resolve<renderContext>();
	auto phys      = game->services.resolve<physics>();
	auto entities  = game->services.resolve<ecs::entityManager>();
	auto factories = game->services.resolve<ecs::serializer>();

	auto view = std::make_shared<gamething>(game);
	game->setView(view);

	//rend->setDefaultLightModel("unshaded");

	const char *mapfile = (argc > 1)? argv[1] : "save.map";

	// XXX
	game->editor->selectedNode = state->rootnode;
	game->editor->setMode(gameEditor::View);

	static std::vector<physicsObject::ptr> mapPhysics;
	if (auto p = loadMapCompiled(mapfile)) {
		state->rootnode = *p;
		phys->addStaticModels(nullptr, *p, TRS(), mapPhysics);
	}

	factories->add<entity>();
	factories->add<sceneComponent>();
	factories->add<rigidBody>();
	factories->add<rigidBodySphere>();
	factories->add<rigidBodyStaticMesh>();
	factories->add<syncRigidBodyXZVelocity>();
	factories->add<syncRigidBodyTransform>();
	factories->add<PBRShader>();
	factories->add<UnlitShader>();

	//entity *temp = new entity(entities.get());
	entity *temp = entities->construct<entity>();
	entities->add(temp);
	temp->attach<rigidBodySphere>(glm::vec3 {0, 10, 0}, 1, 1);
	temp->attach<syncRigidBodyXZVelocity>();
	temp->attach<sceneComponent>(DEMO_PREFIX "assets/obj/BoomBox.glb");
	temp->attach<PBRShader>();
	temp->node->setScale(glm::vec3(100));

	rend->defaultSkybox = skybox("share/proj/assets/tex/cubes/HeroesSquare/", ".jpg");

	entities->systems["collision"] = std::make_shared<entitySystemCollision>();
	entities->systems["syncPhysics"] = std::make_shared<syncRigidBodySystem>();
	game->run();

	std::cout << "It's alive!" << std::endl;
	return 0;
}
