#ifndef CONF
#define CONF

# include <string>

// Some constants: screen size, near and far plane
namespace conf {

	// Sensor configuration
	constexpr unsigned int SCR_WIDTH{ 600 };	// horizontal pixels of the sensor (so, it will be the horizontal pixels of the screen)
	constexpr unsigned int SCR_HEIGHT{ 600 };
	constexpr float near {.1f};
	constexpr float far {15.0f};
	constexpr float focal{ near };	// focal length of camera (distance from sensor to camera origin)
	constexpr float fx{ focal / .0001f };	// usual fx = focal/size of a pixel, in camera units
	constexpr float fy{ focal / .0001f };	// usual fx = focal/size of a pixel, in camera units
	constexpr float cx{ SCR_WIDTH / 2 };	// x coordinate in pixels of the camera center, on the sensor
	constexpr float cy{ SCR_HEIGHT / 2 };	// y coordinate in pixels of the camera center, on the sensor
	constexpr float r{ focal / fx * (SCR_WIDTH - cx) };
	constexpr float l{ - focal / fx * cx };
	constexpr float t{ focal / fy * (SCR_HEIGHT - cy) };
	constexpr float b{ - focal / fy * cy };

	// Rendering configurations
	const std::string render_type = "color";	// normals, HDR, depth_map, otherwise it's the normal thing
	const std::string depth_mode = "standard";

	// Shadow configuration
	//constexpr bool shadows{ false };
	//constexpr float scene_size{ 15.0f };  // i.e. we assume that the size is in ||x||<=scene_size
	//constexpr float light_nearPlane{ .1f };	// to render shadows from the perspective of light, we need a newar and far plane. This is the near. 
	//constexpr float light_farPlane{light_nearPlane + 2 * scene_size};	//The far is near + 2 scene_size

	// Output folder
	const std::string out_folder = "C:/Code/University/TUM/learnOpenGL/data/models/backpack/synthetic/run_0/";
	
}

#endif