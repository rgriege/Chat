#include "Gui.h"

#include "SDL.h"
#include "SDL_keysym.h"
#include "RendererModules/OpenGL/CEGUIOpenGLRenderer.h"
#include "RendererModules/OpenGL/CEGUIOpenGL.h"
#include "NetworkManager.h"
#include "constants.h"
#include "glut.h"

#include <iostream>

#define SDL_SCANCODE_Q 24
#define SDL_SCANCODE_RETURN 36

Gui::Gui()
{
  using namespace CEGUI;

  /* Initialize SDL and OpenGL */
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf ("SDL_Init: %s\n", SDL_GetError());
    exit (EXIT_FAILURE);
  }

  if (SDL_SetVideoMode( 800, 600, 0, SDL_OPENGL) == NULL) {
    printf ("SDL_SetVideoMode: %s\n", SDL_GetError());
    SDL_Quit();
    exit (EXIT_FAILURE);
  }

  SDL_EnableUNICODE(1);
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  glEnable(GL_CULL_FACE);
  glDisable(GL_FOG);
  glClearColor(0.0f,0.0f,0.0f,1.0f);
  glViewport(0,0, 800,600);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0, 800.0/600.0, 0.1,100.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  OpenGLRenderer& myRenderer = OpenGLRenderer::bootstrapSystem();

  // initialise the required dirs for the DefaultResourceProvider
  DefaultResourceProvider* rp = static_cast<DefaultResourceProvider*>(System::getSingleton().getResourceProvider());
  rp->setResourceGroupDirectory("schemes", "datafiles/schemes/");
  rp->setResourceGroupDirectory("imagesets", "datafiles/imagesets/");
  rp->setResourceGroupDirectory("fonts", "datafiles/fonts/");
  rp->setResourceGroupDirectory("layouts", "datafiles/layouts/");
  rp->setResourceGroupDirectory("looknfeels", "datafiles/looknfeel/");
  rp->setResourceGroupDirectory("lua_scripts", "datafiles/lua_scripts/");\

  // set the default resource groups to be used
  Imageset::setDefaultResourceGroup("imagesets");
  Font::setDefaultResourceGroup("fonts");
  Scheme::setDefaultResourceGroup("schemes");
  WidgetLookManager::setDefaultResourceGroup("looknfeels");
  WindowManager::setDefaultResourceGroup("layouts");
  ScriptModule::setDefaultResourceGroup("lua_scripts");
  // setup default group for validation schemas
  CEGUI::XMLParser* parser = CEGUI::System::getSingleton().getXMLParser();
  if (parser->isPropertyPresent("SchemaDefaultResourceGroup"))
    parser->setProperty("SchemaDefaultResourceGroup", "schemas");

  SchemeManager::getSingleton().create("TaharezLook.scheme");

  loggedIn = false;
  SetupLogin();
}

void Gui::SetupLogin()
{
  using namespace CEGUI;

  Window* myRoot = WindowManager::getSingleton().loadWindowLayout("Login.layout");
  System::getSingleton().setGUISheet(myRoot);

  subscribeEvent("Login/Start", PushButton::EventClicked, Event::Subscriber(&Gui::SetupChat, this));
  subscribeEvent("Login/Quit", PushButton::EventClicked, Event::Subscriber(&Gui::Quit, this));
}

bool Gui::SetupChat(const CEGUI::EventArgs &args)
{
  using namespace std;

  string name = static_cast<CEGUI::Editbox*>(CEGUI::WindowManager::getSingleton().getWindow("Login/Namebox"))->getText().c_str();
  string host = static_cast<CEGUI::Editbox*>(CEGUI::WindowManager::getSingleton().getWindow("Login/Hostbox"))->getText().c_str();

  if (host.length() == 0 || host.compare("Hostname") == 0) {
    host = NetworkManager::GetHostname();
    cout << "netmgr host: '" << host << "'" << endl;
  }

  /* Start the server and connect to it */
  Server::GetInstance();
  NetworkManager::GetInstance()->ConnectToServer(host);
  loggedIn = true;

  if (name.length() == 0 || name.compare("Name") == 0)
    name = "Stranger";
  char *msg = new char[name.length()+2];
  msg[0] = NAME;
  strcpy(&msg[1], name.c_str());
  NetworkManager::GetInstance()->SendMessage(msg);
  delete[] msg;

  CEGUI::Window* myRoot = CEGUI::WindowManager::getSingleton().loadWindowLayout("Chat.layout");
  CEGUI::System::getSingleton().setGUISheet(myRoot);

  inputBox = static_cast<CEGUI::MultiLineEditbox*>(CEGUI::WindowManager::getSingleton().getWindow("ChatBox/Input"));
  messageBox = static_cast<CEGUI::Listbox*>(CEGUI::WindowManager::getSingleton().getWindow("ChatBox/List"));
  subscribeEvent("ChatBox/Quit", CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&Gui::Quit, this));
  return true;
}

void Gui::Run()
{
  quit = false;
  while (!quit) {
    CheckInput();
    if (loggedIn)
      CheckServer();
    Display();
  }
  
  CEGUI::OpenGLRenderer::destroySystem();
}

void Gui::Display()
{
  glClear(GL_COLOR_BUFFER_BIT);
  CEGUI::System::getSingleton().renderGUI();
  SDL_GL_SwapBuffers();
}

void Gui::CheckInput()
{
  SDL_Event(e);

  while (SDL_PollEvent(&e)) {
    switch (e.type) {
      case SDL_MOUSEMOTION:
        CEGUI::System::getSingleton().injectMousePosition(
          static_cast<float>(e.motion.x), static_cast<float>(e.motion.y));
        break;
      case SDL_MOUSEBUTTONDOWN:
        handle_mouse_down(e.button.button);
        break;
      case SDL_MOUSEBUTTONUP:
        handle_mouse_up(e.button.button);
        break;
      case SDL_KEYDOWN:
        // printf("key: %i\n", e.key.keysym.sym);
        if (e.key.keysym.sym == SDLK_BACKSPACE) {
          CEGUI::System::getSingleton().injectKeyDown(CEGUI::Key::Backspace);
        } else if (e.key.keysym.sym == SDLK_RETURN  && inputBox && inputBox->hasInputFocus() && inputBox->getCaratIndex() > 0) {
          char msg[512];
          strcpy(&msg[1], inputBox->getText().c_str());
          msg[0] = MESSAGE;
          NetworkManager::GetInstance()->SendMessage(msg);
          inputBox->setText("");
        } else {
          // CEGUI::System::getSingleton().injectKeyDown(e.key.keysym.scancode);
          if (e.key.keysym.unicode != 0) {
            // printf("odd key: %i\n", e.key.keysym.unicode);
            CEGUI::System::getSingleton().injectChar(e.key.keysym.unicode);
          }
        }
        break;
      case SDL_KEYUP:
        CEGUI::System::getSingleton().injectKeyUp(e.key.keysym.scancode);
        break;
      case SDL_QUIT:
        break;
    }
  }
}

void Gui::handle_mouse_down(char button) {
  switch (button) {
    // handle real mouse buttons
    case SDL_BUTTON_LEFT:
      CEGUI::System::getSingleton().injectMouseButtonDown(CEGUI::LeftButton);
      break;
    case SDL_BUTTON_MIDDLE:
      CEGUI::System::getSingleton().injectMouseButtonDown(CEGUI::MiddleButton);
      break;
    case SDL_BUTTON_RIGHT:
      CEGUI::System::getSingleton().injectMouseButtonDown(CEGUI::RightButton);
      break;
 
    // handle the mouse wheel
    case SDL_BUTTON_WHEELDOWN:
      CEGUI::System::getSingleton().injectMouseWheelChange( -1 );
      break;
    case SDL_BUTTON_WHEELUP:
      CEGUI::System::getSingleton().injectMouseWheelChange( +1 );
      break;
  }
}

void Gui::handle_mouse_up(char button) {
  switch (button) {
    case SDL_BUTTON_LEFT:
      CEGUI::System::getSingleton().injectMouseButtonUp(CEGUI::LeftButton);
      break;
    case SDL_BUTTON_MIDDLE:
      CEGUI::System::getSingleton().injectMouseButtonUp(CEGUI::MiddleButton);
      break;
    case SDL_BUTTON_RIGHT:
      CEGUI::System::getSingleton().injectMouseButtonUp(CEGUI::RightButton);
      break;
  }
}

void Gui::CheckServer()
{
  if (NetworkManager::GetInstance()->CheckSocket()) {
    printf("message!\n");
    ShowMessage(NetworkManager::GetInstance()->ReceiveMessage());
  }
}

void Gui::ShowMessage(std::string msg)
{
  CEGUI::ListboxTextItem* chatItem = new CEGUI::ListboxTextItem(CEGUI::String(msg));
  messageBox->addItem(chatItem);
  messageBox->ensureItemIsVisible(messageBox->getItemCount());
}

bool Gui::ClearEditbox(const CEGUI::EventArgs &args)
{
 // TODO - make initial text placeholder
  return true;
}

bool Gui::Quit(const CEGUI::EventArgs &args)
{
  quit = true;
  return true;
}

void Gui::subscribeEvent(const CEGUI::String& widget, const CEGUI::String& event, const CEGUI::Event::Subscriber& method)
{
  CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();
  if (winMgr.isWindowPresent(widget)) {
    CEGUI::Window* window = winMgr.getWindow(widget);
    window->subscribeEvent(event, method);
  }
}
