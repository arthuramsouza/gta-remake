#include "InputManager.h"

namespace cgf

{

InputManager InputManager::m_InputManager;

void InputManager::addKeyInput(sf::String name, sf::Keyboard::Key keyCode)
{
    MyKeys key;
    key.myInputType = KeyboardInput;
    key.myKeyCode   = keyCode;
    keys[name] = key;
}

void InputManager::addMouseInput(sf::String name, sf::Mouse::Button mouseButton)
{
    MyKeys key;
    key.myInputType = MouseInput;
    key.myMouseButton = mouseButton;
    keys[name] = key;
}

bool InputManager::testEvent(sf::String name)
{
    mapT::iterator it = keys.find(name);
    if(it == keys.end())
        return false; // no such binding

    // Get iterator contents
    MyKeys k = it->second;

    // Mouse event?
    if (k.myInputType == MouseInput &&
        sf::Mouse::isButtonPressed(k.myMouseButton))
            return true;
    // Keyboard event
    if (k.myInputType == KeyboardInput &&
        sf::Keyboard::isKeyPressed(k.myKeyCode))
            return true;

    // Binding type doesn't match
    return false;
}

} // namespace
