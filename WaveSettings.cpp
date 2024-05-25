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
	write(ws.casc.depth)
	write(ws.casc.fetch)
	write(ws.casc.wind_speed)
	write(ws.casc.vertical_displacement_scale)
	write(ws.casc.horizontal_displacement_scale)
	write(ws.casc.cutoffLow)
	write(ws.casc.cutoffHigh)
	write(ws.casc.resolution)
	write(ws.casc.patch_size)
	write(ws.casc.select_tex_res);
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
	>> ws.casc.depth
	>> ws.casc.fetch
	>> ws.casc.wind_speed
	>> ws.casc.vertical_displacement_scale
	>> ws.casc.horizontal_displacement_scale
	>> ws.casc.cutoffLow
	>> ws.casc.cutoffHigh
	>> ws.casc.resolution
	>> ws.casc.patch_size
	>> ws.casc.select_tex_res;
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
		return;
	file >> *this;
	file.close();
}
