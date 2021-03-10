#include "Renderer.h"
#include "../ImGui/imgui_impl_dx11.h"
#include "../ImGui/imgui_impl_win32.h"
#include <assert.h>
#include "../Core/Logger.h"
#include "../Core/EventSystem.h"

Engine::Renderer::Renderer::Renderer(HWND& handle, int64_t window_width, int64_t window_height, std::shared_ptr<DependencyResolver<EngineSystem>> m_resolver)
	: EngineSystem(m_resolver), m_handle(handle), m_width(window_width), m_height(window_height)
{
	assert(handle != nullptr);

	DXGI_MODE_DESC buffer_desc = {};

	buffer_desc.Width = static_cast<unsigned int>(m_width);
	buffer_desc.Height = static_cast<unsigned int>(m_height);
	buffer_desc.RefreshRate.Numerator = 60;
	buffer_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	buffer_desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	buffer_desc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	DXGI_SWAP_CHAIN_DESC swap_chain = {};
	swap_chain.BufferDesc = buffer_desc;
	swap_chain.BufferCount = 1;
	swap_chain.SampleDesc.Count = 1;
	swap_chain.SampleDesc.Quality = 0;
	swap_chain.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain.OutputWindow = m_handle;
	swap_chain.Windowed = TRUE;
	swap_chain.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	if (FAILED(m_hresult = D3D11CreateDeviceAndSwapChain(
		NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG,
		NULL, NULL, D3D11_SDK_VERSION, &swap_chain, m_swapChain.GetAddressOf(),
		m_device.GetAddressOf(), NULL, m_context.GetAddressOf())))
	{
		ENGINE_ERROR("Failed To Create Device/Swapchain: {} -- Line {}", m_hresult, __LINE__);
	}

	if (FAILED(m_hresult = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &m_bufferTexture)))
	{
		ENGINE_ERROR("Failed To Get Handle To Back Buffer: {} -- Line {}", m_hresult, __LINE__);
	}

	if (FAILED(m_hresult = m_device->CreateRenderTargetView(m_bufferTexture.Get(), NULL, &m_backBuffer)))
	{
		ENGINE_ERROR("Failed To Create Render Target View From Back Buffer: {} -- Line {}", m_hresult, __LINE__);
	}

	m_device->QueryInterface(__uuidof(ID3D11Debug), &m_debug);

	D3D11_DEPTH_STENCIL_DESC depth_stencil = {};

	depth_stencil.DepthEnable = TRUE;
	depth_stencil.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depth_stencil.DepthFunc = D3D11_COMPARISON_LESS;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;

	if (FAILED(m_hresult = m_device->CreateDepthStencilState(&depth_stencil, depthStencilState.GetAddressOf()))) {
		ENGINE_ERROR("Failed To Create Depth/Stencil State: {} -- Line {}", m_hresult, __LINE__);
	}

	m_context->OMSetDepthStencilState(depthStencilState.Get(), 1u);

	Microsoft::WRL::ComPtr<ID3D11Texture2D> depth_texture;
	D3D11_TEXTURE2D_DESC depth_desc = {};
	depth_desc.Usage = D3D11_USAGE_DEFAULT;
	depth_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depth_desc.Width = static_cast<unsigned int>(m_width);
	depth_desc.Height = static_cast<unsigned int>(m_height);
	depth_desc.MipLevels = 1u;
	depth_desc.ArraySize = 1u;
	depth_desc.Format = DXGI_FORMAT_D32_FLOAT;
	depth_desc.SampleDesc.Count = 1u;
	depth_desc.SampleDesc.Quality = 0u;

	if (FAILED(m_hresult = m_device->CreateTexture2D(&depth_desc, nullptr, depth_texture.GetAddressOf())))
	{
		ENGINE_ERROR("Failed To Create Depth/Stencil Texture: {} -- Line {}", m_hresult, __LINE__);
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC depth_view = {};
	depth_view.Format = DXGI_FORMAT_D32_FLOAT;
	depth_view.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depth_view.Texture2D.MipSlice = 0u;

	if (FAILED(m_hresult = m_device->CreateDepthStencilView(depth_texture.Get(), &depth_view, m_depthStencil.GetAddressOf()))) {
		ENGINE_ERROR("Failed To Bind Depth/Stencil View: {} -- Line {}", m_hresult, __LINE__);
	}

	//Set the render target view to the back buffer that we created above
	m_context->OMSetRenderTargets(1, m_backBuffer.GetAddressOf(), m_depthStencil.Get());

	//Create the Viewport
	D3D11_VIEWPORT view_port = {};
	
	view_port.TopLeftX = 0;
	view_port.TopLeftY = 0;
	view_port.MinDepth = 0;
	view_port.MaxDepth = 1;
	view_port.Width = static_cast<float>(m_width);
	view_port.Height = static_cast<float>(m_height);

	//Set the Viewport
	m_context->RSSetViewports(1, &view_port);

//	ImGui_ImplDX11_Init(m_device.Get(), m_context.Get());
}

void Engine::Renderer::Renderer::ProcessFrame()
{
	BeginFrame();
	EndFrame();
}

int Engine::Renderer::Renderer::GetHash()
{
	return static_cast<int>(SystemTypes::Renderer);
}

void Engine::Renderer::Renderer::BeginFrame()
{
	m_context->ClearRenderTargetView(m_backBuffer.Get(), DirectX::XMVECTORF32{ 0.0f, 0.0f, 0.0f, 1.0f });
	m_context->ClearDepthStencilView(m_depthStencil.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u);
}

void Engine::Renderer::Renderer::EndFrame()
{
	m_swapChain->Present(0u, 0u);
}

void Engine::Renderer::Renderer::HandleWindowResize(int64_t width, int64_t height)
{
	if (m_width != width || m_height != height)
	{
		m_width = width;
		m_height = height;

		ENGINE_INFO("Window Resize Handled By Renderer - Width: {} Height: {}", m_width, m_height);
	}
	
}
