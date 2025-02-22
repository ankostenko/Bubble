#include "window/window.hpp"
#include "log/log.hpp"

namespace bubble
{

void Window::ErrorCallback( int error, const char* description )
{
    LogError( "Error: {} \nDescription: {}", error, description );
}

void Window::KeyCallback( GLFWwindow* window, int key, int scancode, int action, int mods )
{
    if( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS )
        glfwSetWindowShouldClose( window, GL_TRUE );

    Window* win = reinterpret_cast<Window*>( glfwGetWindowUserPointer( window ) );
    Event event;
    event.mType = EventType::KeyboardKey;
    event.mKeyboard.Key = static_cast<KeyboardKey>( key );
    event.mKeyboard.Action = static_cast<KeyAction>( action );
    event.mKeyboard.Mods.SHIFT = mods & GLFW_MOD_SHIFT;
    event.mKeyboard.Mods.CONTROL = mods & GLFW_MOD_CONTROL;
    event.mKeyboard.Mods.ALT = mods & GLFW_MOD_ALT;
    event.mKeyboard.Mods.SUPER = mods & GLFW_MOD_SUPER;
    event.mKeyboard.Mods.CAPS_LOCK = mods & GLFW_MOD_CAPS_LOCK;
    event.mKeyboard.Mods.NUM_LOCK = mods & GLFW_MOD_NUM_LOCK;
    win->mEvents.push_back( event );
}

void Window::MouseButtonCallback( GLFWwindow* window, int key, int action, int mods )
{
    Window* win = reinterpret_cast<Window*>( glfwGetWindowUserPointer( window ) );
    Event event;
    event.mType = EventType::MouseKey; 
    event.mMouse.Key = static_cast<MouseKey>( key );
    event.mMouse.Action = static_cast<KeyAction>( action );
    event.mMouse.Mods.SHIFT = mods & GLFW_MOD_SHIFT;
    event.mMouse.Mods.CONTROL = mods & GLFW_MOD_CONTROL;
    event.mMouse.Mods.ALT = mods & GLFW_MOD_ALT;
    event.mMouse.Mods.SUPER = mods & GLFW_MOD_SUPER;
    event.mMouse.Mods.CAPS_LOCK = mods & GLFW_MOD_CAPS_LOCK;
    event.mMouse.Mods.NUM_LOCK = mods & GLFW_MOD_NUM_LOCK;
    win->mEvents.push_back( event );
}

void Window::MouseCallback( GLFWwindow* window, double xpos, double ypos )
{
    Window* win = reinterpret_cast<Window*>( glfwGetWindowUserPointer( window ) );
    auto mouse_pos = glm::vec2( xpos, ypos );
    win->mMouseInput.mMouseOffset = mouse_pos - win->mMouseInput.mMousePos;
    win->mMouseInput.mMousePos = mouse_pos;
    Event event;
    event.mType = EventType::MouseMove;
    event.mMouse.Pos = win->mMouseInput.mMousePos;
    event.mMouse.Offset = win->mMouseInput.mMouseOffset;
    win->mEvents.push_back( event );
}

void Window::ScrollCallback( GLFWwindow* window, double xoffset, double yoffset )
{
    Window* win = reinterpret_cast<Window*>( glfwGetWindowUserPointer( window ) );
    Event event;
    event.mType = EventType::MouseZoom;
    event.mMouse.ZoomOffset -= static_cast<float>( yoffset );
    win->mEvents.push_back( event );
}

void Window::WindowSizeCallback( GLFWwindow* window, int width, int height )
{
    Window* win = reinterpret_cast<Window*>( glfwGetWindowUserPointer( window ) );
    win->mWindowSize = WindowSize{ width, height };
    glfwSetWindowSize( win->mWindow, width, height );
}

void Window::FramebufferSizeCallback( GLFWwindow* window, int width, int height )
{
    Window* win = reinterpret_cast<Window*>( glfwGetWindowUserPointer( window ) );
    win->mWindowSize = WindowSize{ width, height };
    glViewport( 0, 0, width, height );
}



Window::Window( const std::string& name, WindowSize size )
    : mWindowSize( size )
{
    glfwSetErrorCallback( ErrorCallback );
    if( !glfwInit() )
        throw std::runtime_error( "GLFW init error" );

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    mGLSLVersion = "#version 100";
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 2 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 0 );
    glfwWindowHint( GLFW_CLIENT_API, GLFW_OPENGL_ES_API );
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    mGLSLVersion = "#version 150";
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 2 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );  // 3.2+ only
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    mGLSLVersion = "#version 130";
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 0 );
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    mWindow = glfwCreateWindow( size.mWidth, size.mHeight, name.c_str(), NULL, NULL );
    if( !mWindow )
    {
        glfwTerminate();
        throw std::runtime_error( "GLFW window creation error" );
    }
    glfwMakeContextCurrent( mWindow );

    glfwSetWindowUserPointer( mWindow, this );
    glfwSetInputMode( mWindow, GLFW_LOCK_KEY_MODS, GLFW_TRUE );
    // set callback functions
    glfwSetKeyCallback( mWindow, KeyCallback );
    glfwSetMouseButtonCallback( mWindow, MouseButtonCallback );
    glfwSetCursorPosCallback( mWindow, MouseCallback );
    glfwSetScrollCallback( mWindow, ScrollCallback );
    glfwSetWindowSizeCallback( mWindow, WindowSizeCallback );
    glfwSetFramebufferSizeCallback( mWindow, FramebufferSizeCallback );

#if !defined(__EMSCRIPTEN__)
    GLenum err = glewInit();
    if ( GLEW_OK != err )
    {
        std::cerr << "Error: " << glewGetErrorString( err ) << std::endl;
        glfwTerminate();
        throw std::runtime_error( "GLEW init error" );
    }
#endif

    IMGUI_CHECKVERSION();
    mImGuiContext = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    //LoadIniSettingsFromMemory()
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    if ( io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    ImGui_ImplGlfw_InitForOpenGL( mWindow, true );
    ImGui_ImplOpenGL3_Init( mGLSLVersion );
}

Window::~Window()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow( mWindow );
    glfwTerminate();
}

WindowSize Window::GetSize() const 
{
    return mWindowSize;
}

bool Window::ShouldClose() const
{
    return mShouldClose;
}

const std::vector<Event>& Window::PollEvents()
{
    mEvents.clear();
    glfwPollEvents();
    mShouldClose = glfwWindowShouldClose( mWindow );
    return mEvents;
}

void Window::OnUpdate()
{
    glfwSwapBuffers( mWindow );
}

void Window::SetVSync( bool vsync )
{
    glfwSwapInterval( vsync );
}

GLFWwindow* Window::GetHandle() const
{
    return mWindow;
}

const char* Window::GetGLSLVersion() const
{
    return mGLSLVersion;
}


ImGuiContext* Window::GetImGuiContext()
{
    return mImGuiContext;
}

void Window::ImGuiBegin()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport( ImGui::GetMainViewport() );
}

void Window::ImGuiEnd()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

    // Multi viewports
    ImGuiIO& io = ImGui::GetIO();
    if ( io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent( mWindow );
    }
}


}