/* Start Header ************************************************************************/
/*!
\file       ParticleEmitterComponent.cpp
\author     Guan Shao Jun [100%]
\par        shaojun.g@digipen.edu
\date       November 07 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "ParticleEmitterComponent.h"
#include "GraphicsSystem.h"
#include <algorithm>
#include <cmath>
#include <random>

static std::random_device rd;
static std::mt19937 gen(rd());

ParticleEmitterComponent::ParticleEmitterComponent(GameObject* owner) :
	IComponent(owner),
	maxParticles(100),
	isEmitting(false),
	loop(true),
	duration(-1.f),
	emissionTimer(0.0f),
	durationTimer(0.0f),
	emissionRate(10.0f),
	particleLifetime(1.0f),
	lifetimeVariation(0.0f),
	spawnOffset(0, 0),
	spawnRadius(0.0f),
	spawnBox(0, 0),
	useSpawnBox(false),
	velocityDirection(0, 1),
	velocityMagnitude(50.0f),
	velocityVariation(0.0f),
	angleSpread(0.0f),
	textureID(0),
	startColor(1, 1, 1, 1),
	endColor(1, 1, 1, 0),
	startSize(10.0f),
	endSize(5.0f),
	layer(0),
	startRotation(0.0f),
	startRotationVariation(0.0f),
	angularVelocity(0.0f),
	angularVelocityVariation(0.0f),
	gravity(0, 0),
	drag(0.0f),
	useSizeCurve(true),
	fadeInDuration(0.0f),
	fadeOutDuration(0.3f)
{
	particles.resize(maxParticles);
}

ParticleEmitterComponent::ParticleEmitterComponent(GameObject* owner, int maxParts)
	: ParticleEmitterComponent(owner)
{
	SetMaxParticles(maxParts);
}

void ParticleEmitterComponent::Update(float dt) {
	if (!IsEnabled()) {
		return;
	}

	auto* transform = owner->GetComponent<TransformComponent>();
	if (!transform) {
		return;
	}

	float x, y;
	transform->GetPosition(x, y);
	vec2 emitterPos(x, y);

	if (isEmitting && duration > 0.0f) {
		durationTimer += dt;
		if (durationTimer >= duration) {
			if (loop) {
				durationTimer = 0.0f;
			}
			else {
				isEmitting = false;
			}
		}
	}

	if (isEmitting) {
		emissionTimer += dt;
		float timePerParticle = 1.0f / emissionRate;

		while (emissionTimer >= timePerParticle) {
			SpawnParticle(emitterPos);
			emissionTimer -= timePerParticle;
		}
	}

	for (auto& p : particles) {
		p.lifetime += dt;

		if (p.lifetime >= p.maxLifeTime) {
			p.active = false;
			continue;
		}

		p.velocity += gravity * dt;
		p.velocity *= (1.0f - drag * dt);
		p.position += p.velocity * dt;
		p.rotation += p.angularVelocity * dt;

		float t = p.lifetime / p.maxLifeTime;

		p.color.x = startColor.x + (endColor.x - startColor.x) * t;
		p.color.y = startColor.y + (endColor.y - startColor.y) * t;
		p.color.z = startColor.z + (endColor.z - startColor.z) * t;
		p.color.w = startColor.w + (endColor.w - startColor.w) * t;

		if (fadeInDuration > 0.0f && p.lifetime < fadeInDuration) {
			float fadeT = p.lifetime / fadeInDuration;
			p.color.w *= fadeT;
		}

		if (fadeOutDuration > 0.0f && p.lifetime > (p.maxLifeTime - fadeOutDuration)) {
			float fadeT = (p.maxLifeTime - p.lifetime) / fadeOutDuration;
			p.color.w *= fadeT;
		}

		if (useSizeCurve) {
			p.size = startSize + (endSize - startSize) * t;
		}
	}
}

void ParticleEmitterComponent::StartEmission() {
	isEmitting = true;
	durationTimer = 0.0f;
	emissionTimer = 0.0f;
}

void ParticleEmitterComponent::StopEmission() {
	isEmitting = false;
}

void ParticleEmitterComponent::Burst(int count) {
	auto* transform = owner->GetComponent<TransformComponent>();
	if (!transform) {
		return;
	}

	float x, y;
	transform->GetPosition(x, y);
	vec2 emitterPos(x, y);

	for (int i = 0;i < count;i++) {
		SpawnParticle(emitterPos);
	}
}

void ParticleEmitterComponent::Clear() {
	for (auto& p : particles) {
		p.active = false;
	}
}

void ParticleEmitterComponent::SetMaxParticles(int max) {
	maxParticles = max;
	particles.clear();
	particles.resize(maxParticles);
}

int ParticleEmitterComponent::GetActiveParticleCount() const {
	int count = 0;
	for (const auto& p : particles) {
		if (p.active) {
			++count;
		}
	}

	return count;
}

void ParticleEmitterComponent::AddParticlesToBatch(std::vector<RenderBatch>& batches, const mat4& viewProj) {
	if (!IsEnabled()) {
		return;
	}

	//int activeCount = GetActiveParticleCount();
	//if (activeCount > 0) {
	//	std::cout << "Attempting to render " << activeCount << " particles." << std::endl;
	//}

	RenderBatch* targetBatch = nullptr;
	for (auto& batch : batches) {
		if (batch.textureID == textureID && batch.isCircle == false && batch.layer == layer) {
			targetBatch = &batch;
			break;
		}
	}

	if (!targetBatch) {
		batches.emplace_back(textureID, false, layer);
		targetBatch = &batches.back();
		targetBatch->vertices.reserve(maxParticles * 4);
		targetBatch->indices.reserve(maxParticles * 6);
	}

	for (const auto& p : particles) {
		if (!p.active) {
			continue;
		}

		vec3 localVerts[4] = {
			{-0.5f, -0.5f, 0.f},
			{ 0.5f, -0.5f, 0.f},
			{-0.5f,  0.5f, 0.f},
			{ 0.5f,  0.5f, 0.f}
		};

		vec2 texCoords[4]{
			{0.f, 0.f},
			{1.f, 0.f},
			{0.f, 1.f},
			{1.f, 1.f}
		};

		GLuint baseIndex = (GLuint)targetBatch->vertices.size();

		for (int i = 0;i < 4;i++) {
			BatchVertex vert;
			vert.pos = localVerts[i];
			vert.tex = texCoords[i];
			vert.color = p.color;
			vert.uvScale = vec2(1.0f, 1.0f);
			vert.uvOffset = vec2(0.0f, 0.0f);
			vert.transform = vec3(p.position.x, p.position.y, p.rotation);
			vert.scale = vec2(p.size, p.size);

			targetBatch->vertices.push_back(vert);
		}

		targetBatch->indices.push_back(baseIndex + 0);
		targetBatch->indices.push_back(baseIndex + 1);
		targetBatch->indices.push_back(baseIndex + 2);
		targetBatch->indices.push_back(baseIndex + 2);
		targetBatch->indices.push_back(baseIndex + 1);
		targetBatch->indices.push_back(baseIndex + 3);
	}
}

void ParticleEmitterComponent::SpawnParticle(const vec2& emitterPosition) {
	Particle* p = GetInactiveParticle();
	if (!p) {
		return;
	}

	p->active = true;
	p->lifetime = 0.0f;
	p->maxLifeTime = particleLifetime + RandomRange(-lifetimeVariation, lifetimeVariation);

	vec2 spawnPos = emitterPosition + spawnOffset;
	if (useSpawnBox) {
		spawnPos += RandomInBox(spawnBox);
	}
	else if (spawnRadius > 0.0f) {
		spawnPos += RandomInCircle(spawnRadius);
	}

	p->position = spawnPos;

	float angle = RandomRange(-angleSpread / 2.0f, angleSpread / 2.0f);
	float radians = angle * pi / 180.0f;
	float cosA = cos(radians);
	float sinA = sin(radians);

	vec2 rotatedDir(velocityDirection.x * cosA - velocityDirection.y * sinA,
		velocityDirection.x * sinA + velocityDirection.y * cosA);

	float speed = velocityMagnitude + RandomRange(-velocityVariation, velocityVariation);
	p->velocity = rotatedDir * speed;

	p->color = startColor;
	p->size = startSize;

	p->rotation = startRotation + RandomRange(-startRotationVariation, startRotationVariation);
	p->angularVelocity = angularVelocity + RandomRange(-angularVelocityVariation, angularVelocityVariation);
}

Particle* ParticleEmitterComponent::GetInactiveParticle() {
	for (auto& p : particles) {
		if (!p.active) {
			return &p;
		}
	}
	return nullptr;
}

float ParticleEmitterComponent::RandomRange(float min, float max) const {
	std::uniform_real_distribution<float> dist(min, max);
	return dist(gen);
}

vec2 ParticleEmitterComponent::RandomInCircle(float radius) const {
	std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * pi);
	std::uniform_real_distribution<float> radiusDist(0.0f, radius);

	float angle = angleDist(gen);
	float r = sqrt(radiusDist(gen));

	return vec2(r * cos(angle), r * sin(angle));
}

vec2 ParticleEmitterComponent::RandomInBox(const vec2& halfExtents) const {
	std::uniform_real_distribution<float> xDist(-halfExtents.x, halfExtents.x);
	std::uniform_real_distribution<float> yDist(-halfExtents.y, halfExtents.y);

	return vec2(xDist(gen), yDist(gen));
}

std::unique_ptr<IComponent> ParticleEmitterComponent::Clone(GameObject* newOwner) const {
	auto clone = std::make_unique<ParticleEmitterComponent>(newOwner, maxParticles);

	// Copy all properties
	clone->emissionRate = emissionRate;
	clone->loop = loop;
	clone->duration = duration;
	clone->particleLifetime = particleLifetime;
	clone->lifetimeVariation = lifetimeVariation;
	clone->spawnOffset = spawnOffset;
	clone->spawnRadius = spawnRadius;
	clone->spawnBox = spawnBox;
	clone->useSpawnBox = useSpawnBox;
	clone->velocityDirection = velocityDirection;
	clone->velocityMagnitude = velocityMagnitude;
	clone->velocityVariation = velocityVariation;
	clone->angleSpread = angleSpread;
	clone->textureID = textureID;
	clone->startColor = startColor;
	clone->endColor = endColor;
	clone->startSize = startSize;
	clone->endSize = endSize;
	clone->layer = layer;
	clone->startRotation = startRotation;
	clone->startRotationVariation = startRotationVariation;
	clone->angularVelocity = angularVelocity;
	clone->angularVelocityVariation = angularVelocityVariation;
	clone->gravity = gravity;
	clone->drag = drag;
	clone->useSizeCurve = useSizeCurve;
	clone->fadeInDuration = fadeInDuration;
	clone->fadeOutDuration = fadeOutDuration;

	clone->SetEnabled(IsEnabled());

	return clone;
}

// =====================================================================
// PRESET CONFIGURATIONS FOR GAME
// =====================================================================

void ParticleEmitterComponent::ConfigureDashTrail() {
	SetEmissionRate(80.0f);
	SetMaxParticles(200);
	SetLifetime(1.0f, 0.05f);

	SetSpawnRadius(5.0f);
	SetSpawnOffset(vec2(0.0f, -60.0f));
	SetVelocity(vec2(0, 0), 0.0f);  // stationary particles

	SetStartColor(vec4(0.3f, 0.6f, 1.0f, 0.8f));  // bright blue
	SetEndColor(vec4(0.1f, 0.3f, 0.8f, 0.0f));    // fade to transparent

	SetStartSize(20.0f);
	SetEndSize(5.0f);

	SetGravity(vec2(0, 0));
	SetDrag(0.0f);

	SetFadeOut(0.2f);
	SetLayer(-2);  // behind player
}

void ParticleEmitterComponent::ConfigureJumpDust() {
	SetEmissionRate(0.0f);  // Use Burst() instead
	SetMaxParticles(20);
	SetLifetime(0.5f, 0.1f);

	SetSpawnBox(vec2(15.0f, 2.0f));  // spread horizontally
	SetVelocity(vec2(0, 1), 40.0f, 20.0f);
	SetAngleSpread(60.0f);

	SetStartColor(vec4(0.8f, 0.8f, 0.7f, 0.8f));  // dusty color
	SetEndColor(vec4(0.6f, 0.6f, 0.5f, 0.0f));

	SetStartSize(8.0f);
	SetEndSize(15.0f);  // expand

	SetGravity(vec2(0, -100.0f));
	SetDrag(0.5f);

	SetStartRotation(0.0f, 360.0f);
	SetAngularVelocity(90.0f, 180.0f);

	SetLayer(1);
}

void ParticleEmitterComponent::ConfigureWallSlideParticles() {
	SetEmissionRate(50.0f);
	SetMaxParticles(100);
	SetLifetime(0.4f, 0.1f);

	SetSpawnBox(vec2(2.0f, 10.0f));  // thin vertical spawn area
	SetVelocity(vec2(1, 0), 80.0f, 30.0f);  // shoot away from wall
	SetAngleSpread(45.0f);

	SetStartColor(vec4(1.0f, 0.8f, 0.3f, 1.0f));  // orange sparks
	SetEndColor(vec4(1.0f, 0.3f, 0.1f, 0.0f));    // red fade

	SetStartSize(6.0f);
	SetEndSize(2.0f);

	SetGravity(vec2(0, -150.0f));
	SetDrag(0.3f);

	SetStartRotation(0.0f, 360.0f);
	SetAngularVelocity(360.0f, 720.0f);

	SetLayer(0);
}

void ParticleEmitterComponent::ConfigureSpeedLines() {
	SetEmissionRate(40.0f);
	SetMaxParticles(80);
	SetLifetime(0.2f, 0.05f);

	SetSpawnBox(vec2(100.0f, 50.0f));  // wide spawn area
	SetVelocity(vec2(-1, 0), 300.0f, 50.0f);  // fast horizontal

	SetStartColor(vec4(1.0f, 1.0f, 1.0f, 0.5f));
	SetEndColor(vec4(1.0f, 1.0f, 1.0f, 0.0f));

	SetStartSize(30.0f);  // long lines
	SetEndSize(20.0f);

	SetGravity(vec2(0, 0));
	SetDrag(0.0f);

	SetStartRotation(0.0f);  // horizontal lines

	SetLayer(0);  // far behind
}

void ParticleEmitterComponent::ConfigureDeathExplosion() {
	SetEmissionRate(0.0f);  // Use Burst() for this
	SetMaxParticles(50);
	SetLifetime(1.0f, 0.3f);

	SetSpawnRadius(5.0f);
	SetVelocity(vec2(0, 1), 150.0f, 80.0f);
	SetAngleSpread(360.0f);  // explode in all directions

	SetStartColor(vec4(1.0f, 0.3f, 0.2f, 1.0f));  // red
	SetEndColor(vec4(0.8f, 0.1f, 0.0f, 0.0f));

	SetStartSize(15.0f);
	SetEndSize(5.0f);

	SetGravity(vec2(0, -200.0f));
	SetDrag(0.4f);

	SetStartRotation(0.0f, 360.0f);
	SetAngularVelocity(180.0f, 360.0f);

	SetFadeOut(0.5f);
	SetLayer(2);  // on top
}