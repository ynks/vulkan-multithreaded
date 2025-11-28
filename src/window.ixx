/// @file window.ixx
/// @author Xein
/// @date 27-Nov-2025

module;

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_raii.hpp>
#include <functional>

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
	void waitEvents();
	
	[[nodiscard]] /// @brief Get surface
	static vk::raii::SurfaceKHR* surface() { return &window()->m_surface; }

	std::pair<uint32_t, uint32_t> framebufferSize();
	
	template<typename T, typename Func>
	void setupResizeCallback(T* app, Func&& callback) {
		glfwSetWindowUserPointer(m_rawWindow, this);
		m_appPointer = app;
		m_resizeHandler = [callback, app](void*, int w, int h) {
			callback(app, w, h);
		};
		setFramebufferSizeCallback(internalResizeCallback);
	}

private:
	static Window* m_instance;
	GLFWwindow* m_rawWindow;

	// Vulkan specific
	vk::raii::SurfaceKHR m_surface = nullptr;
	
	void* m_appPointer = nullptr;
	void setUserPointer(void* pointer);
	void setFramebufferSizeCallback(void (*callback)(GLFWwindow*, int, int));
	
	static void internalResizeCallback(GLFWwindow* window, int width, int height);
	std::function<void(void*, int, int)> m_resizeHandler;
};

export Window* window() { return Window::window(); }

Window* Window::m_instance = nullptr;

