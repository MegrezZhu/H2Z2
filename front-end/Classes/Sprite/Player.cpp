#pragma once

#include "Player.h"
#include <chrono>

USING_NS_CC;
using namespace std::chrono;

const float Player::maxHp = 200.0f;

Player::Player() {}

Player* Player::create(Vec2 pos) {
	auto player = new (std::nothrow) Player();
	if (player && player->initWithFile("player.png")) {
		// init
		player->setScale(0.2f);
		player->setPosition(pos);
		auto playerBody = PhysicsBody::createBox(player->getContentSize(), PhysicsMaterial(10.0f, 0.0f, 0.0f));
		playerBody->setCategoryBitmask(0x00000001);
		playerBody->setCollisionBitmask(0xFFFFFFFE); // disable collision between players
		playerBody->setContactTestBitmask(0xFFFFFFFE);
		player->setPhysicsBody(playerBody);

		player->autorelease();
		return player;
	}
	CC_SAFE_DELETE(player);
	return nullptr;
}

Document Player::createSyncData() const {
	Document dom;
	auto body = this->getPhysicsBody();
	dom.SetObject();

	// position & speed & angle
	rapidjson::Value speedX, speedY, posX, posY;
	auto speed = body->getVelocity();

	// note: must use position of 'player' instead of 'body', to have a correct position
	//	 while working with the Follow Action
	auto pos = this->getPosition();

	dom.AddMember("speedX", speed.x, dom.GetAllocator());
	dom.AddMember("speedY", speed.y, dom.GetAllocator());
	dom.AddMember("posX", pos.x, dom.GetAllocator());
	dom.AddMember("posY", pos.y, dom.GetAllocator());
	dom.AddMember("angle", this->getRotation(), dom.GetAllocator());

	milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	dom.AddMember("timestamp", ms.count(), dom.GetAllocator());	// used to check ping
	return dom;
}

void Player::sync(GenericValue<UTF8<>> &data) {
	auto body = this->getPhysicsBody();
	body->setVelocity(Vec2(data["speedX"].GetDouble(), data["speedY"].GetDouble()));
	this->setPosition(data["posX"].GetDouble(), data["posY"].GetDouble());
	this->setRotation(data["angle"].GetDouble());
}

void Player::addVelocity(Vec2 v) {
	auto body = this->getPhysicsBody();
	if (body) {
		body->setVelocity(body->getVelocity() + v);
	}
}

void Player::setVelocityX(float vx) {
	auto body = this->getPhysicsBody();
	if (body) {
		body->setVelocity(Vec2(vx, body->getVelocity().y));
	}
}

void Player::setVelocityY(float vy) {
	auto body = this->getPhysicsBody();
	if (body) {
		body->setVelocity(Vec2(body->getVelocity().x, vy));
	}
}

void Player::broadcastHit(float dmg) const {
	Document dom;
	dom.SetObject();
	dom.AddMember("type", "hit", dom.GetAllocator());
	dom.AddMember("damage", 20.0f, dom.GetAllocator());	// TODO: damage determined by bullet type
	GSocket->sendEvent("broadcast", dom);
}

float Player::getHp() const { return hp; }

void Player::setHp(float _hp) {
	hp = std::min(_hp, maxHp);
}

void Player::heal(float _hp) {
	hp = std::min(hp + _hp, maxHp);
}

bool Player::damage(float _hp) {
	hp = std::max(0.0f, hp - _hp);
	return hp > 0.0f;
}

void Player::broadcastDead() const {
	Document dom;
	dom.SetObject();
	dom.AddMember("type", "dead", dom.GetAllocator());
	GSocket->sendEvent("broadcast", dom);
}

void Player::takeWeapon(Weapon* w) {
	w->token();
	this->addChild(w);
	auto size = this->getContentSize();
	w->setRotation(0);
	w->setPosition(Vec2(size.width / 2, size.height + 39));
	this->weapon = w;
}

Weapon* Player::dropWeapon() {
	auto w = this->weapon;
	if (w != nullptr) {
		w->dropped();
		auto pos = this->getPosition();
		auto angle = this->getRotation();
		w->setRotation(angle);
		if (angle > 270) angle = angle - 360.0;
		angle = angle / 180.0 * M_PI;
		float x = 100.0 * sin(angle);
		float y = 100.0 * cos(angle);
		w->setPosition(Vec2(pos.x + x, pos.y + y));
		this->removeChild(w, false);
		this->weapon = nullptr;
	}
	return w;
}

Weapon* Player::getWeapon() {
	return this->weapon;
}
