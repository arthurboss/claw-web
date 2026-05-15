#ifndef __ACTORCONTROLLER_H__
#define __ACTORCONTROLLER_H__

#include "Engine/Interfaces.h"
#include "Engine/SharedDefines.h"
#include "Engine/Platform/Events/AppEvent.h"

class SceneNode;
class ActorController : public IKeyboardHandler, public IPointerHandler,
                        public ITouchHandler, public IGamepadHandler
{
public:
    ActorController(shared_ptr<SceneNode> controlledObject, float speed = 0.36f);

    void SetControlledObject(shared_ptr<SceneNode> controlledObject) { m_pControlledObject = controlledObject; }
    void OnUpdate(uint32 msDiff);
    void HandleAction(ActionType actionType);

    // IKeyboardHandler
    bool VOnKeyDown(SDL_Keycode key) override;
    bool VOnKeyUp(SDL_Keycode key) override;

    // IPointerHandler
    bool VOnPointerMove(SDL_MouseMotionEvent& mouseEvent) override;
    bool VOnPointerButtonDown(SDL_MouseButtonEvent& mouseEvent) override;
    bool VOnPointerButtonUp(SDL_MouseButtonEvent& mouseEvent) override;

    // ITouchHandler
    std::vector<std::shared_ptr<AbstractRecognizer>> VRegisterRecognizers() override;
    bool VOnTouch(const Touch_Event &evt) override;

    // IGamepadHandler
    bool VOnGamepadButtonDown(GamepadButton button, float value) override;
    bool VOnGamepadButtonUp(GamepadButton button) override;
    bool VOnGamepadAxis(GamepadAxis axis, float value) override;

private:
    bool OnTap(int id, const Touch_TapEvent &evt);
    bool OnJoystick(int id, const Touch_JoystickEvent &evt);
    bool OnSwipe(int id, const Touch_SwipeEvent &evt, bool start);
    bool OnPress(int id, const Touch_PressEvent &evt, bool start);

    shared_ptr<SceneNode> m_pControlledObject;
    float m_Speed;

    std::map<int, bool> m_InputKeys;

    // SDL_Scancode array
    const uint8* m_pKeyStates;

    bool m_MouseLeftButtonDown;
    bool m_MouseRightButtonDown;

    // Gamepad analog stick state
    float m_GamepadMoveX = 0.0f;
    float m_GamepadMoveY = 0.0f;
};

#endif //__ACTORCONTROLLER_H__
