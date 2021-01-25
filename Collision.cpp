#include "screen.h"
#include "vec2.h"

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <thread>

using namespace std::chrono_literals;

constexpr float G = 3.0f;

struct Body
{
	float r;
	float m;
	vec2 pos;
	vec2 vel;
	vec2 acc;

	Body()
	{
		m = 1.0f;
		r = 0.2f;
		pos = 0;
		vel = 0;
		acc = 0;
	}

	Body(float m) : m(m)
	{
		r = 0.2f * std::cbrt(m);
		pos = 0;
		vel = 0;
		acc = 0;
	}

	Body(float m, float r) : r(r), m(m)
	{
		pos = 0;
		vel = 0;
		acc = 0;
	}

	void setPos(float x, float y)
	{
		pos.x = x;
		pos.y = y;
	}

	void PulledBy(const Body& other)
	{
		float dist = std::sqrt((pos - other.pos) * (pos - other.pos));
		acc += G * other.m * (other.pos - pos) / dist / dist /
			   dist;	// vector form of the formula for gravity
	}

	void Update(float dt)
	{
		vel += dt * acc;
		pos += dt * vel;
		acc = 0;
	}
};

void Plot(const Body& body, Screen& scr)	// render the body
{
	scr.PlotCircle(body.pos.x, body.pos.y, body.r);
}

float random(float low, float high)
{
	return low + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (high - low)));
}

int main()
{
	srand(static_cast<unsigned>(time(nullptr)));

	Screen scr(0, 0, 5);

	constexpr std::size_t n = 20000;
	constexpr float dt = 1.0f / 25.0f;

	// Initializing first galaxy
	Body Centre1(2000.0f, 2.5f);
	Centre1.pos = {150.0f, 20.0f};
	Centre1.vel = {-5.0f, 0.0f};
	Body Bodies1[n];
	for(auto& body: Bodies1)
	{
		float maxRadius = 30.0f;	// max radius of the galaxy

		float theta = random(0.0f, 2 * M_PIf32);	// angle the particle makes with the centre

		float r = random(1.0f, maxRadius);
		r = r * r / maxRadius;	  // change distribution of particles (optional)
		r += 0.2f * Centre1.r;
		body.pos = {r * std::cos(theta), r * std::sin(theta)};	  // polar to cartezian coordinates
		body.pos += Centre1.pos;	// move the particle relative to the centre

		float v = std::sqrt(G * Centre1.m / r);	   // calculate velocity based on radius
		body.vel = {v * std::sin(theta),
					-v *
						std::cos(theta)};	 // polar to cartezian coordinates, rotated by 90 degrees
		float offset = 0.6f;
		body.vel +=
			{random(-offset, offset), random(-offset, offset)};	   // random offset to velocity
		body.vel += Centre1.vel;

		body.r = 0.2f;
	}
	/* Initializing second galaxy */
	/* Similar to first */
	Body Centre2(2000.0f, 2.5f);
	Centre2.pos = -Centre1.pos;
	Centre2.vel = -Centre1.vel;
	Body Bodies2[n];
	for(auto& body: Bodies2)
	{
		float maxRadius = 30.0f;

		float theta = random(0.0f, 2 * M_PIf32);

		float r = random(1.0f, maxRadius);
		r = r * r / maxRadius;
		r += 0.2f * Centre2.r;
		body.pos = {r * std::cos(theta), r * std::sin(theta)};
		body.pos += Centre2.pos;

		float v = std::sqrt(G * Centre2.m / r);
		body.vel = {v * std::sin(theta), -v * std::cos(theta)};
		// uncomment for opposite direction of rotation
		// Bodies2[i].vel=-Bodies2[i].vel;
		float offset = 0.6f;
		body.vel += {random(-offset, offset), random(-offset, offset)};
		body.vel += Centre2.vel;

		body.r = 0.2f;
	}

	while(true)
	{
		scr.Clear();

		// centres attract each other
		Centre1.PulledBy(Centre2);
		Centre2.PulledBy(Centre1);

		// particles are attracted to centres
		for(std::size_t i = 0; i < n; i++)
		{
			Bodies1[i].PulledBy(Centre1);
			Bodies2[i].PulledBy(Centre1);
			Bodies1[i].PulledBy(Centre2);
			Bodies2[i].PulledBy(Centre2);
		}

		// update bodies
		Centre1.Update(dt);
		Centre2.Update(dt);
		for(auto& body: Bodies1)
			body.Update(dt);
		for(auto& body: Bodies2)
			body.Update(dt);

		// rendering
		Plot(Centre1, scr);
		Plot(Centre2, scr);

		for(auto&& body: Bodies1)
			Plot(body, scr);
		for(auto&& body: Bodies2)
			Plot(body, scr);

		// drawing
		if((Centre1.pos - Centre2.pos) * (Centre1.pos - Centre2.pos) < 90.0f * 90.0f)
			scr.Zoom(9);
		if((Centre1.pos - Centre2.pos) * (Centre1.pos - Centre2.pos) > 110.0f * 110.0f)
			scr.Zoom(5);
		scr.Draw();

		std::this_thread::sleep_for(15ms);
	}

	return 0;
}
