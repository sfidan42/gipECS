# gipObject2DPlugin

## Description
This is a plugin for GlistEngine that provides a 2D object that can be defined dynamically.

## How to use?
1. Include the plugin in GlistApp's `CMakeLists.txt` file:
    ```cmake
    ########## USED PLUGINS LIST ##########
    set(PLUGINS
        gipObject2DPlugin
    )
    ```
2. Use the `Object2D` class to create and manipulate 2D objects.
    ```c++
    #include "datatypes/Object2D.h"
    ```
3. Create an instance of `Object2D` and use its methods to add textures, set positions, and draw the object.
    ```c++
   // === IMAGE + FIXED + VECTOR ===
    auto* imageVectorFixed = new Object2D<Type2D::VECTOR, Pos2D::FIXED, Tex2D::IMAGE>();
    Frame imgTex = loadFrame("platform.png");
    imageVectorFixed->addTexture(std::move(imgTex));
    imageVectorFixed->addObject2D(0, glm::vec2(10, 10), 0.0f, glm::vec2(64, 64));
    imageVectorFixed->draw();

    // === SPRITE + MOVING + VECTOR ===
    auto* spriteVectorMoving = new Object2D<Type2D::VECTOR, Pos2D::MOVING, Tex2D::SPRITE>();
    AnimationFrames animFrames = loadAnimationFrames("gemiler/ship_1/ship_1_%d.png", 1, 25);
    spriteVectorMoving->addTexture(std::move(animFrames));
    spriteVectorMoving->addObject2D(0, glm::vec2(50, 50), glm::vec2(1.0f, 0.0f), 0.0f, glm::vec2(32, 32));
    spriteVectorMoving->update(0.016f); // simulate frame update
    spriteVectorMoving->draw();

    // === NODE + MOVING + SPRITE ===
    AnimationFrames moveAnim = loadAnimation("gemiler/ship_2/ship_2_%d.png", 1, 16));
    auto* spriteNodeMoving = new Object2D<Type2D::NODE, Pos2D::MOVING, Tex2D::SPRITE>(&moveAnim,
        glm::vec2(100, 100), glm::vec2(0.5f, 0.2f), 0.0f, glm::vec2(32, 32));
    spriteNodeMoving->update(0.016f, glm::vec2(0, 0), glm::vec2(800, 600));
    spriteNodeMoving->draw();

    // === NODE + FIXED + IMAGE ===
    Frame iconFrame = loadFrame("icon.png");
    auto* imageNodeFixed = new Object2D<Type2D::NODE, Pos2D::FIXED, Tex2D::IMAGE>(&iconFrame,
        glm::vec2(200, 150), 0.0f, glm::vec2(48, 48));
    // no update is defined for fixed position
    imageNodeFixed->draw();

    // === INTERFACE example (abstract: here we just assign pointer)
    Object2D<Type2D::INTERFACE, Pos2D::MOVING, Tex2D::SPRITE>* abstractInterface = spriteNodeMoving;
    std::cout << "Abstract mid position: " << abstractInterface->getMidPosition().x << ", "
              << abstractInterface->getMidPosition().y << "\n";

    // Clean up
    delete imageVectorFixed;
    delete spriteVectorMoving;
    delete spriteNodeMoving;
    delete imageNodeFixed;
    ```

## Example
 ```c++
 /*
 * ShipController.h
 *
 *  Created on: Jul 3, 2025
 *      Author: sadettin
 */

#ifndef SRC_CONTROLLERS_SHIPCONTROLLER_H_
#define SRC_CONTROLLERS_SHIPCONTROLLER_H_
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include "gImage.h"
#include "gFile.h"
#include "gFont.h"
#include "animation/SpriteAnimator.h"
#include "datatypes/Object2D.h"


class ShipController {
public:
   ShipController();
   ~ShipController();
   void WPressed() { speed.y -= 1000.0f; if(selectedship) selectedship->setSpeed(speed); }
   void APressed() { speed.x -= 1000.0f; if(selectedship) selectedship->setSpeed(speed); }
   void SPressed() { speed.y += 1000.0f; if(selectedship) selectedship->setSpeed(speed); }
   void DPressed() { speed.x += 1000.0f; if(selectedship) selectedship->setSpeed(speed); }
   void FPressed();
   void GPressed();
   void WReleased() { speed.y += 1000.0f; if(selectedship) selectedship->setSpeed(speed); }
   void AReleased() { speed.x += 1000.0f; if(selectedship) selectedship->setSpeed(speed); }
   void SReleased() { speed.y -= 1000.0f; if(selectedship) selectedship->setSpeed(speed); }
   void DReleased() { speed.x -= 1000.0f; if(selectedship) selectedship->setSpeed(speed); }
   void mouseLeftRelease(const glm::vec2& clickedPos);
   void mouseRightRelease(const glm::vec2& clickedPos);
   
    void setup(const glm::vec2& minBoundary, const glm::vec2& maxBoundary);
    void update(float deltaTime);
    void draw() const;
   
    Object2D<Type2D::NODE, Pos2D::MOVING, Tex2D::SPRITE>* getSelectedShip() const {
        return dynamic_cast<Object2D<Type2D::NODE, Pos2D::MOVING, Tex2D::SPRITE>*>(selectedship);
    }
   
private:
   glm::vec2 speed = glm::vec2(0.0f);
   
   Object2D<Type2D::VECTOR, Pos2D::FIXED, Tex2D::SPRITE>* fixedships = nullptr;
   Object2D<Type2D::VECTOR, Pos2D::MOVING, Tex2D::SPRITE>* movableships = nullptr;
   Object2D<Type2D::INTERFACE, Pos2D::MOVING, Tex2D::SPRITE>* selectedship = nullptr;
   
   Object2D<Type2D::VECTOR, Pos2D::MOVING, Tex2D::IMAGE>* bullets = nullptr;
   Frame bulletframe;
   
   AnimationFrames selectionframes;
   SpriteAnimator selectionanimator;
   
   gFont* font = nullptr;
};

#endif /* SRC_CONTROLLERS_SHIPCONTROLLER_H_ */
```
```c++
/*
 * ShipController.cpp
 *
 *  Created on: Jul 3, 2025
 *      Author: sadettin
 */

#include "controllers/ShipController.h"

#include "gAppManager.h"

ShipController::ShipController() {
	fixedships = new Object2D<Type2D::VECTOR, Pos2D::FIXED, Tex2D::SPRITE>;
	fixedships->addTexture("gemiler/ship_1/ship_1_%d.png", 1, 25);
	fixedships->addTexture("gemiler/ship_2/ship_2_%d.png", 1, 16);
	fixedships->addTexture("gemiler/ship_3/ship_3_%d.png", 1, 25);
	fixedships->addTexture("gemiler/ship_4/ship_4_%d.png", 1, 25);
	fixedships->addObject2D(0, {60, 20}, 0.0f, {242, 239});
	fixedships->addObject2D(1, {90, 310}, 0.0f, {170, 193});
	fixedships->addObject2D(2, {47, 557},0.0f, {255, 250});
	fixedships->addObject2D(3, {110, 900}, 0.0f, {130, 97});
	movableships = new Object2D<Type2D::VECTOR, Pos2D::MOVING, Tex2D::SPRITE>();
	bullets = new Object2D<Type2D::VECTOR, Pos2D::MOVING, Tex2D::IMAGE>;
	const std::string bulletPath = "human_bullet.png";
	bulletframe = loadFrame(bulletPath);
	if(bulletframe.frame == nullptr) {
		gLoge("ShipController::ShipController") << "Failed to load bullet frame from: " << bulletPath;
		return;
	}
	selectionframes = loadAnimationFrames("platform_anim/platform_%d.png", 0, 24);
	auto* selectionAnimation = new SpriteAnimation(&selectionframes, 25);
	selectionanimator.addAnimation(selectionAnimation);
	selectionanimator.changeAnimation(selectionAnimation->getId());
	font = new gFont();
}

ShipController::~ShipController() {
	delete fixedships;
	delete movableships;
	delete bullets;
	delete font;
}

void ShipController::FPressed() {
	if (selectedship) {
		const float bulletDrawAngle = selectedship->getAngle();
		const float bulletSpeedAngle = bulletDrawAngle + 90.0f;

		const glm::vec2 direction = glm::rotate(glm::vec2(1.0f, 0.0f), glm::radians(bulletSpeedAngle));

		const glm::vec2 shipSize = selectedship->getSize();
		const float shipRadius = glm::length(shipSize) * 0.5f;

		const glm::vec2 shipSpeed = selectedship->getSpeed();
		const glm::vec2 normShipSpeed = glm::length(shipSpeed) <= 0.01f ? glm::vec2(0.0f) : glm::normalize(shipSpeed);
		const glm::vec2 bulletSpeed = direction * 1000.0f + normShipSpeed * 333.333f;
		const glm::vec2 bulletFrameSize = glm::vec2(bulletframe.frame->getWidth(), bulletframe.frame->getHeight());

		const glm::vec2 spawnPos = selectedship->getMidPosition() + direction * shipRadius - bulletFrameSize * 0.5f;

		const glm::vec2 bulletSize = bulletFrameSize;

		auto* newBullet = new Object2D<Type2D::NODE, Pos2D::MOVING, Tex2D::IMAGE>(&bulletframe, spawnPos, bulletSpeed, bulletDrawAngle, bulletSize);
		bullets->push_back(newBullet);
	} else {
		gLogw("ShipController::FPressed") << "No ship selected to fire!";
	}
}

void ShipController::GPressed() {
	if (selectedship) {
		const float bulletDrawAngle = selectedship->getAngle();
		const float bulletSpeedAngle = bulletDrawAngle + 90.0f;

		const glm::vec2 direction = glm::rotate(glm::vec2(1.0f, 0.0f), glm::radians(bulletSpeedAngle));

		const glm::vec2 shipSize = selectedship->getSize();
		const float shipRadius = glm::length(shipSize) * 0.5f;

		const glm::vec2 shipSpeed = selectedship->getSpeed();
		const glm::vec2 normShipSpeed = glm::length(shipSpeed) <= 0.01f ? glm::vec2(0.0f) : glm::normalize(shipSpeed);
		const glm::vec2 bulletSpeed = direction * 100.0f + normShipSpeed * 33.333f;
		const glm::vec2 bulletFrameSize = glm::vec2(bulletframe.frame->getWidth(), bulletframe.frame->getHeight());

		glm::vec2 spawnPos = selectedship->getMidPosition() + direction * shipRadius - bulletFrameSize * 0.5f;

		const glm::vec2 bulletSize = bulletFrameSize;

		for (int i = 0; i < 10; ++i) {
			const glm::vec2 newSpawnPos = spawnPos + glm::gaussRand(glm::vec2(0.0f), glm::vec2(10.0f));
			auto* newBullet = new Object2D<Type2D::NODE, Pos2D::MOVING, Tex2D::IMAGE>(&bulletframe, newSpawnPos, bulletSpeed, bulletDrawAngle, bulletSize);
			bullets->push_back(newBullet);
		}
	} else {
		gLogw("ShipController::FPressed") << "No ship selected to fire!";
	}
}

void ShipController::mouseLeftRelease(const glm::vec2& clickedPos) {
	if (selectedship) {
		selectedship->setSpeed(glm::vec2(0.0f, 0.0f));
	}
	selectedship = nullptr;
	// Try selecting a MOVING ship first
	auto movableIt = movableships->selectObject2D(clickedPos);
	if (movableIt != movableships->end()) {
		selectedship = *movableIt;
		gLogi("ShipController::mouseLeftRelease")
			<< "Selected movable ship with id " << selectedship->getId()
			<< " at position: " << clickedPos.x << ", " << clickedPos.y;
		selectedship->setSpeed(speed);
		return;
	}
	// If not found, check template (FIXED) ships
	auto fixedIt = fixedships->selectObject2D(clickedPos);
	if (fixedIt != fixedships->end()) {
		auto* movingShipCopy = new Object2D<Type2D::NODE, Pos2D::MOVING, Tex2D::SPRITE>(**fixedIt);
		movableships->push_back(movingShipCopy);
		selectedship = movingShipCopy;
		gLogi("ShipController::mouseLeftRelease")
			<< "Selected fixed ship with id " << selectedship->getId()
			<< " at position: " << clickedPos.x << ", " << clickedPos.y;
		selectedship->setSpeed(speed);
	}
}

void ShipController::mouseRightRelease(const glm::vec2& clickedPos) {
	auto movableIt = movableships->selectObject2D(clickedPos);
	if(movableIt == movableships->end()) {
		return;
	}
	Object2D<Type2D::INTERFACE, Pos2D::MOVING, Tex2D::SPRITE>* selectedMovable = *movableIt;
	if(selectedMovable == selectedship) {
		selectedship = nullptr;
	}
	delete selectedMovable;
	*movableIt = movableships->back();
	movableships->pop_back();
}

void ShipController::setup(const glm::vec2& minBoundary, const glm::vec2& maxBoundary) {
	movableships->setup(minBoundary, maxBoundary); // Ships don't die in border, dieInBorder is left false
	bullets->setup(minBoundary, maxBoundary, true); // Bullets die in border
	font->loadFont("TESLA.ttf", 16);
}

void ShipController::update(float deltaTime) {
	movableships->update(deltaTime);
	selectionanimator.update(deltaTime);
	bullets->update(deltaTime);
}

void ShipController::draw() const {
	if(selectedship) {
		const gImage* curFrame = selectionanimator.getCurrentFrame();
		glm::vec2 curPos = selectedship->getMidPosition();
		glm::vec2 curFrameSize {
			static_cast<float>(curFrame->getWidth()),
			static_cast<float>(curFrame->getHeight())
		};
		curPos -= curFrameSize * 0.5f;
		selectionanimator.draw(curPos, curFrameSize, 30.0f);
	}
	bullets->draw();
	fixedships->draw();
	movableships->draw();
	std::string shipsinfo = "# of fixed ships: " + gToStr(fixedships->size()) +
		"\n# pf moving ships: " + gToStr(movableships->size()) +
		"\n# of bullets: " + gToStr(bullets->size());
	font->drawText(shipsinfo, 900.0f, 516.0f);
}
```
