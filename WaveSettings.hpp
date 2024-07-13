#pragma once

#include <iostream>

struct WaveSettings
{
	struct
	{
		float size = 512.f;
		int lod = 55;
		int sqrt_num_instances = 1;
	} plane;
	struct
	{
		int image_select = 0;
		int render_wireframe = 0;
		int tess_follow_cam = 1;
		int simulate_ocean = 1;
		int render_ocean = 1;
		int render_skybox = 1;
	} debug;
	struct
	{
		float min_distance = 75.f;
		float max_distance = 250.f;
		int min_level = 1;
		int max_level = 25;
	} tess;
	struct
	{
		float speed = 50.f;
		float sprint_factor = 7.f;
	} cam;
	struct
	{
		float normal_strength = 1.f;
		float tex_coord_scale = 250.f;
	} render;
	struct
	{
		float depth = 1.f;
		float fetch = 1250.f;
		float wind_speed = 25.f;
		float vertical_displacement_scale = 1.f;
		float horizontal_displacement_scale = 1.f;
		float cutoffLow = 0.f;
		float cutoffHigh = 1000.f;
		int resolution = 512;
		int patch_size = 250;
		// I doubt this will change in the future but this setting should correspond to the initial resolution value in
		// WaveSettings::casc::resolution these two work together in our ImGui::Combo to select the desired resolution
		int select_tex_res = 1;
	} casc;

	const char *const fileName = "wave_settings.ini";

	void load();
	void save();
};

std::ostream &operator<<(std::ostream &os, const WaveSettings ws);
std::istream &operator>>(std::istream &is, WaveSettings &ws);
