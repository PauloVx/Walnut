#pragma once

#include "Walnut/Layer.h"
#include "Walnut/Image.h"

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <memory>
#include <functional>
#include <filesystem>

#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"

#include "vulkan/vulkan.h"

void check_vk_result(VkResult err);

struct GLFWwindow;

namespace Walnut {

	// A TrueType font the application adds to the atlas, found later with Application::GetFont(Name)
	struct FontSpecification
	{
		std::string Name;
		const void* Data = nullptr;   // TTF bytes; must outlive the application
		size_t DataSize = 0;
		float SizePixels = 20.0f;
		bool Default = false;         // replaces Roboto as ImGui's default font
	};

	struct ApplicationSpecification
	{
		std::string Name = "Walnut App";
		uint32_t Width = 1600;
		uint32_t Height = 900;

		std::filesystem::path IconPath;

		bool WindowResizeable = true;

		// Uses custom Walnut titlebar instead
		// of Windows default
		bool CustomTitlebar = false;

		// Fonts added after Walnut's own (Default, Bold, Italic)
		std::vector<FontSpecification> Fonts;

		// Custom titlebar look: its background, and whether it shows Name centred
		uint32_t TitlebarColor = IM_COL32(21, 21, 21, 255);
		bool TitlebarShowsName = true;
		bool TitlebarShowsIcon = true;

		// Uses a dockspace which takes up the
		// entire window
		bool UseDockspace = true;

		// Window will be created in the center
		// of primary monitor
		bool CenterWindow = false;

		// ImGui windows can be dragged out as OS windows (Windows only: GLFW cannot place windows on Wayland)
		bool UseViewports = true;

		// Where WALNUT.log and APP.log go; empty logs to the console only
		std::filesystem::path LogDirectory;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& applicationSpecification = ApplicationSpecification());
		~Application();

		static Application& Get();

		void Run();
		void SetMenubarCallback(const std::function<void()>& menubarCallback);

		template<typename T>
		void PushLayer()
		{
			static_assert(std::is_base_of<Layer, T>::value, "Pushed type is not subclass of Layer!");
			m_LayerStack.emplace_back(std::make_shared<T>())->OnAttach();
		}

		void PushLayer(const std::shared_ptr<Layer>& layer) { m_LayerStack.emplace_back(layer); layer->OnAttach(); }

		const std::vector<std::shared_ptr<Layer>>& GetLayerStack() const { return m_LayerStack; }

		void Close();

		bool IsMaximized() const;
		std::shared_ptr<Image> GetApplicationIcon() const { return m_AppHeaderIcon; }
		// Replaces the icon drawn in the custom titlebar (Walnut's own by default)
		void SetApplicationIcon(std::shared_ptr<Image> icon) { if (icon) m_AppHeaderIcon = std::move(icon); }

		float GetTime();
		GLFWwindow* GetWindowHandle() const { return m_WindowHandle; }
		bool IsTitleBarHovered() const { return m_TitleBarHovered; }
		// Height of the custom titlebar drawn at the top of the window; 0 with the system's own frame
		float GetTitlebarHeight() const { return m_Specification.CustomTitlebar ? 58.0f : 0.0f; }

		static VkInstance GetInstance();
		static VkPhysicalDevice GetPhysicalDevice();
		static VkDevice GetDevice();

		static VkCommandBuffer GetCommandBuffer(bool begin);
		static void FlushCommandBuffer(VkCommandBuffer commandBuffer);

		static void SubmitResourceFree(std::function<void()>&& func);

		static ImFont* GetFont(const std::string& name);

		template<typename Func>
		void QueueEvent(Func&& func)
		{
			m_EventQueue.push(func);
		}

		static ImGui_ImplVulkanH_Window* GetMainWindowData();
		static VkCommandBuffer GetActiveCommandBuffer();
	private:
		void Init();
		void Shutdown();

		// For custom titlebars
		void UI_DrawTitlebar(float& outTitlebarHeight);
		void UI_DrawMenubar();
	private:
		ApplicationSpecification m_Specification;
		GLFWwindow* m_WindowHandle = nullptr;
		bool m_Running = false;

		float m_TimeStep = 0.0f;
		float m_FrameTime = 0.0f;
		float m_LastFrameTime = 0.0f;

		bool m_TitleBarHovered = false;

		std::vector<std::shared_ptr<Layer>> m_LayerStack;
		std::function<void()> m_MenubarCallback;

		std::mutex m_EventQueueMutex;
		std::queue<std::function<void()>> m_EventQueue;

		// Resources
		// TODO(Yan): move out of application class since this can't be tied
		//            to application lifetime
		std::shared_ptr<Walnut::Image> m_AppHeaderIcon;
		std::shared_ptr<Walnut::Image> m_IconClose;
		std::shared_ptr<Walnut::Image> m_IconMinimize;
		std::shared_ptr<Walnut::Image> m_IconMaximize;
		std::shared_ptr<Walnut::Image> m_IconRestore;
	};

	// Implemented by CLIENT
	Application* CreateApplication(int argc, char** argv);
}