/* Start Header ************************************************************************/
/*!
\file       ParticleEmitterComponent.h
\author     Guan Shao Jun [100%]
\par        shaojun.g@digipen.edu
\date       November 07 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/


#pragma once
#include "Component.h"
#include "MathLib.h"
#include <vector>
#include "GL/glew.h"
#include "transform.h"
#include "gameObject.h"
//#include "GraphicsSystem.h"

struct Particle {
	vec2 position;
	vec2 velocity;
	vec4 color;

	float size;
	float rotation;
	float angularVelocity;
	float lifetime;
	float maxLifeTime;
	bool active;

	Particle() : position(0,0), velocity(0,0), color(1,1,1,1),
		size(1.0f), rotation(0.0f), angularVelocity(0.0f),
		lifetime(0.f), maxLifeTime(1.0f), active(false) {}
};

struct RenderBatch;
struct BatchVertex;

class ParticleEmitterComponent : public IComponent {
	public:
		ParticleEmitterComponent(GameObject* owner);

		ParticleEmitterComponent(GameObject* owner, int maxParticles);

		virtual ~ParticleEmitterComponent() = default;

		virtual void Update(float) override;

		virtual std::string GetName() const override { return "Particle Emitter Component"; }

		virtual std::unique_ptr<IComponent> Clone(GameObject* newOwner) const override;

		void StartEmission();

		void StopEmission();

		void Burst(int count);

		void Clear();

		void SetEmissionRate(float particlesPerSecond) { emissionRate = particlesPerSecond; };

		void SetMaxParticles(int max);

		void SetLoop(bool shouldLoop) { loop = shouldLoop; };

		void SetDuration(float seconds) { duration = seconds; };

		void SetLifetime(float seconds, float variation = 0.f) {
			particleLifetime = seconds;
			lifetimeVariation = variation;
		}

		void SetSpawnOffset(const vec2& offset) { spawnOffset = offset; };

		void SetSpawnRadius(float radius) { spawnRadius = radius; };

		void SetSpawnBox(const vec2& halfExtents) { spawnBox = halfExtents; };

		void SetVelocity(const vec2& direction, float magnitude, float variation = 0.0f) {
			velocityDirection = direction.normalized();
			velocityMagnitude = magnitude;
			velocityVariation = variation;
		}

		void SetAngleSpread(float degrees) { angleSpread = degrees; };

		void SetTexture(GLuint texID) { textureID = texID; };

		void SetStartColor(const vec4& color) { startColor = color; };

		void SetEndColor(const vec4& color) { endColor = color; };

		void SetStartSize(float size) { startSize = size; };

		void SetEndSize(float size) { endSize = size; };

		void SetLayer(int renderLayer) { layer = renderLayer; };

		void SetStartRotation(float degrees, float variation = 0.f) {
			startRotation = degrees;
			startRotationVariation = variation;
		}

		void SetAngularVelocity(float degreesPerSecond, float variation = 0.f) {
			angularVelocity = degreesPerSecond;
			angularVelocityVariation = variation;
		}

		void SetGravity(const vec2& gravityAccel) { gravity = gravityAccel; };

		void SetDrag(float dragCoefficient) { drag = dragCoefficient; };

		void SetSizeOverLifetimeCurve(bool useStartEnd) { useSizeCurve = useStartEnd; };

		void SetFadeIn(float fadeInTime) { fadeInDuration = fadeInTime; };

		void SetFadeOut(float fadeOutTime) { fadeOutDuration = fadeOutTime; };

		bool IsEmitting() const { return isEmitting; };

		int GetActiveParticleCount() const;

		int GetMaxParticles() const { return maxParticles; };

		GLuint GetTextureID() const { return textureID; };

		int GetLayer() const { return layer; };

		void AddParticlesToBatch(std::vector<RenderBatch>& batches, const mat4& viewProj);

		void ConfigureDashTrail();

		void ConfigureJumpDust();

		void ConfigureWallSlideParticles();

		void ConfigureSpeedLines();

		void ConfigureDeathExplosion();

	private:
		void SpawnParticle(const vec2& emitterPosition);
		Particle* GetInactiveParticle();
		float RandomRange(float min, float max) const;
		vec2 RandomInCircle(float radius) const;
		vec2 RandomInBox(const vec2& halfExtents) const;

		std::vector<Particle> particles;
		int maxParticles;

		bool isEmitting;
		bool loop;
		float duration;
		float emissionTimer;
		float durationTimer;

		float emissionRate;

		float particleLifetime;
		float lifetimeVariation;

		vec2 spawnOffset;
		float spawnRadius;
		vec2 spawnBox;
		float useSpawnBox;

		vec2 velocityDirection;
		float velocityMagnitude;
		float velocityVariation;
		float angleSpread;

		GLuint textureID;
		vec4 startColor;
		vec4 endColor;
		float startSize;
		float endSize;
		int layer;

		float startRotation;
		float startRotationVariation;
		float angularVelocity;
		float angularVelocityVariation;

		vec2 gravity;
		float drag;

		bool useSizeCurve;
		float fadeInDuration;
		float fadeOutDuration;

};