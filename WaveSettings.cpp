#include "WaveSettings.hpp"
#include <fstream>

#define write(x) << x << " "

// to avoid confusion the order of these are written in order of properties in the struct
std::ostream &operator<<(std::ostream &os, const WaveSettings ws)
{
	// clang-format off
	os
	write(ws.plane.size)
	write(ws.plane.lod)
	write(ws.plane.sqrt_num_instances)
	write(ws.debug.image_select)
	write(ws.debug.render_wireframe)
	write(ws.debug.tess_follow_cam)
	write(ws.debug.simulate_ocean)
	write(ws.debug.render_ocean)
	write(ws.debug.render_skybox)
	write(ws.tess.min_distance)
	write(ws.tess.max_distance)
	write(ws.tess.min_level)
	write(ws.tess.max_level)
	write(ws.cam.speed)
	write(ws.cam.sprint_factor)
	write(ws.render.normal_strength)
	write(ws.render.tex_coord_scale)
	write(ws.casc1.depth)
	write(ws.casc1.fetch)
	write(ws.casc1.wind_speed)
	write(ws.casc1.vertical_displacement_scale)
	write(ws.casc1.horizontal_displacement_scale)
	write(ws.casc1.cutoffLow)
	write(ws.casc1.cutoffHigh)
	write(ws.casc1.resolution)
	write(ws.casc1.patch_size)
	write(ws.casc1.select_tex_res)
	write(ws.casc1.spread_blend)
	write(ws.casc1.swell)
	write(ws.casc1.angle)
	write(ws.casc2.cutoffLow)
	write(ws.casc2.cutoffHigh)
	write(ws.casc2.patch_size)
	write(ws.casc2.depth)
	write(ws.casc3.cutoffLow)
	write(ws.casc3.cutoffHigh)
	write(ws.casc3.patch_size)
	write(ws.casc3.depth);
	// clang-format on

	return os;
}

std::istream &operator>>(std::istream &is, WaveSettings &ws)
{
	// clang-format off
	is
	>> ws.plane.size
	>> ws.plane.lod
	>> ws.plane.sqrt_num_instances
	>> ws.debug.image_select
	>> ws.debug.render_wireframe
	>> ws.debug.tess_follow_cam
	>> ws.debug.simulate_ocean
	>> ws.debug.render_ocean
	>> ws.debug.render_skybox
	>> ws.tess.min_distance
	>> ws.tess.max_distance
	>> ws.tess.min_level
	>> ws.tess.max_level
	>> ws.cam.speed
	>> ws.cam.sprint_factor
	>> ws.render.normal_strength
	>> ws.render.tex_coord_scale
	>> ws.casc1.depth
	>> ws.casc1.fetch
	>> ws.casc1.wind_speed
	>> ws.casc1.vertical_displacement_scale
	>> ws.casc1.horizontal_displacement_scale
	>> ws.casc1.cutoffLow
	>> ws.casc1.cutoffHigh
	>> ws.casc1.resolution
	>> ws.casc1.patch_size
	>> ws.casc1.select_tex_res
	>> ws.casc1.spread_blend
	>> ws.casc1.swell
	>> ws.casc1.angle
	>> ws.casc2.cutoffLow
	>> ws.casc2.cutoffHigh
	>> ws.casc2.patch_size
	>> ws.casc2.depth
	>> ws.casc3.cutoffLow
	>> ws.casc3.cutoffHigh
	>> ws.casc3.patch_size
	>> ws.casc3.depth;
	// clang-format on

	return is;
}

void WaveSettings::save()
{
	std::fstream file{fileName, std::ios::out | std::ios::trunc};
	if (!file.is_open())
	{
		std::cerr << "COULD NOT SAVE: " << fileName << std::endl;
		return;
	}
	file << *this;
	file.close();
}

void WaveSettings::load()
{
	std::fstream file{fileName, std::ios::in};
	if (!file.is_open())
	{
		std::cout << "Could not find settings file\nLoading defaults" << std::endl;
		return;
	}
	file >> *this;
	file.close();
}
