# gipObject2DPlugin

## Description
This is a plugin for GlistEngine that provides a 2D object that can be defined dynamically.

## How to use?
1. Include the plugin in GlistApp's `CMakeLists.txt` file:
    ```cmake
    ########## USED PLUGINS LIST ##########
    set(PLUGINS
        gipECS
    )
    ```
2. Example Code
```

void gCanvas::setup() {

	using namespace ecs;

	animationDuration = 1.0f;

	auto* model = new gModel();
	model->loadModel("alien_walk/scene.gltf");
	for(int i = -3; i <= 3; i++) {
		entity modelEntity = reg.create();
		reg.emplace<gModel*>(modelEntity, model);
		reg.emplace<Position<3>>(modelEntity, Position<3>{(float)i, 0.0f, 0.0f});
		reg.emplace<Velocity<3>>(modelEntity, Velocity<3>{0.0f, 0.0f, 1.0f});
		reg.emplace<Rotation<3>>(modelEntity, Rotation<3>{ -60.0f, 0.0f, 1.0f, 0.0f});
	}

	camera.setPosition(0, 1, 10);

	entity lightEntity = reg.create();
	auto* light1 = new gLight(gLight::LIGHTTYPE_DIRECTIONAL);
	light1->setPosition(camera.getPosition());
	light1->setDiffuseColor(255, 255, 0);
	light1->setSpecularColor(66, 66, 66);
	light1->setOrientation(camera.getLookOrientation());
	light1->setSpotCutOffAngle(1.0f);

	reg.emplace<gLight*>(lightEntity, light1);

	font.load(gGetFontsDir() + "FreeSans.ttf", 12);
}

void gCanvas::update() {

	using namespace ecs;

	float deltaTime = static_cast<float>(appmanager->getElapsedTime());
	animationTime += deltaTime;
	if (animationTime >= animationDuration) {
		animationTime = 0.0f;
	}
	auto view = reg.view_all<gModel*, Position<3>, Velocity<3>>();
	for(auto e : view) {
		auto& pos = reg.get<Position<3>>(e);
		auto& vel = reg.get<Velocity<3>>(e);
		auto* model = reg.get<gModel*>(e);
		pos.x += vel.vx * deltaTime;
		pos.y += vel.vy * deltaTime;
		pos.z += vel.vz * deltaTime;
		model->animate(animationTime);
	}
}

void gCanvas::draw() {
	setColor(255, 255, 255);

	camera.begin();
	enableDepthTest();

	const auto& lights = reg.view_all<gLight*>();
	const auto& models = reg.view_all<gModel*>();

	for(auto e : lights) {
		gLight* light = reg.get<gLight*>(e);
		light->enable();
	}

	for(auto e : models) {
		gModel* model = reg.get<gModel*>(e);
		ecs::Position<3> pos = reg.get<ecs::Position<3>>(e);
		model->setPosition(pos.x, pos.y, pos.z);
		model->draw();
	}

	for(auto e : lights) {
		gLight* light = reg.get<gLight*>(e);
		light->disable();
	}

	disableDepthTest();
	camera.end();

	font.drawText("FPS: " + gToStr(root->getFramerate()), 10, 22);
}

```
