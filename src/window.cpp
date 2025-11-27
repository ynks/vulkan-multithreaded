module;
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <print>
#include <vulkan/vulkan_raii.hpp>

import vulkan.instance;

module window;

Window::Window() {
	if (m_instance) {
		throw std::runtime_error("One Window already exists");
	}

	m_instance = this;

	std::println("Initializing GLFW");
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_rawWindow = glfwCreateWindow(WIDTH, HEIGHT, WINDOW_NAME, nullptr, nullptr);
	std::println("Created window \"{}\"", WINDOW_NAME);
}

Window::~Window() {
	if (m_rawWindow) {
		glfwDestroyWindow(m_rawWindow);
	}
	glfwTerminate();
	m_instance = nullptr;
}

Window* Window::window() {
	if (!m_instance) {
		throw std::runtime_error("Trying to access Window but it doesn't exist yet");
	}

	return m_instance;
}

Window* Window::operator()(){ return Window::window(); }

void Window::CreateSurface() {
	auto* vk_instance = vulkan::instance()->get();

	// Retrieve correct OS window surface
	VkSurfaceKHR m_rawSurface;
	if (glfwCreateWindowSurface(**vk_instance, m_rawWindow, nullptr, &m_rawSurface) != 0) {
		throw std::runtime_error("Failed to create window surface");
	}

	// Promote surface to vkhpp
	m_surface = vk::raii::SurfaceKHR(*vk_instance, m_rawSurface);
}

bool Window::shouldClose() const {
	return glfwWindowShouldClose(m_rawWindow);
}

void Window::pollEvents() {
	glfwPollEvents();
}


std::pair<uint32_t, uint32_t> Window::framebufferSize() {
	int width, height;
	glfwGetFramebufferSize(m_rawWindow, &width, &height);

	return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
}

