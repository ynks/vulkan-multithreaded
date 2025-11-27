/// @file window.ixx
/// @author Xein
/// @date 27-Nov-2025

module;

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_raii.hpp>

export module window;

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;
constexpr const char* WINDOW_NAME = "CS180 final";

export class Window {
public:
	Window();
	~Window();
	static Window* window();
	static Window* operator()();
	
	void CreateSurface();
	bool shouldClose() const;
	void pollEvents();
	
	[[nodiscard]] /// @brief Get surface
	static vk::raii::SurfaceKHR* surface() { return &window()->m_surface; }

	std::pair<uint32_t, uint32_t> framebufferSize();

private:
	static Window* m_instance;
	GLFWwindow* m_rawWindow;

	// Vulkan specific
	vk::raii::SurfaceKHR m_surface = nullptr;
};

export Window* window() { return Window::window(); }

Window* Window::m_instance = nullptr;

