#include "ChilliEngine.h"

void ChilliEngine::Init(WindowData& window_data)
{
	//Create Systems
	m_resolver = std::make_shared<DependencyResolver<EngineSystem>>();
	m_eventSystem = std::make_shared<EventSystem>(m_resolver);
	m_timer = std::make_shared<Timer>(m_resolver);
	m_renderer = std::make_shared<Renderer>(window_data.handle, window_data.width, window_data.height, m_resolver);
	//Register Dependencies	
	m_resolver->Add(m_eventSystem);
	m_resolver->Add(m_timer);
	m_resolver->Add(m_renderer);
	ENGINE_INFO("Chilli Engine Initialized Successfully");
	
	//Subscribe To Events
	//m_eventSystem->Subscribe({ EventType::MouseMove }, std::bind(MouseMovedCallback, std::ref(m_eventSystem->m_data.mouse_x), std::ref(m_eventSystem->m_data.mouse_y)));
	
	
}

void ChilliEngine::Update()
{
	m_timer->ProcessFrame();
	m_eventSystem->ProcessFrame();	
	
}

//Bridge Between Window And Engine
void ChilliEngine::RaiseEvent(EventData& data)
{
	Event* e = new Event(data);
	m_eventSystem->Push(e);
}

