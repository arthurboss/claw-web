#include "ActorController.h"
#include "Engine/Scene/SceneNodes.h"
#include "Engine/Events/EventMgr.h"
#include "Engine/Events/Events.h"
#include "ClawEvents.h"
#include "Engine/GameApp/BaseGameApp.h"
#include "Engine/GameApp/HapticFeedback.h"
#include "Engine/UserInterface/Touch/TouchRecognizers/TapRecognizer.h"
#include "Engine/UserInterface/Touch/TouchRecognizers/JoystickRecognizer.h"
#include "Engine/UserInterface/Touch/TouchRecognizers/SwipeRecognizer.h"
#include "Engine/UserInterface/Touch/TouchRecognizers/PressRecognizer.h"

#define PAUSE_TAP_RECOGNIZER 1
#define JUMP_PRESS_RECOGNIZER 2
#define ATTACK_TAP_RECOGNIZER 3
#define WEAPON_TAP_RECOGNIZER 4
#define MOVEMENT_JOYSTICK_RECOGNIZER 5
#define PROJECTILE_SWIPE_RECOGNIZER 6

ActorController::ActorController(shared_ptr<SceneNode> controlledObject, float speed)
{
    m_pControlledObject = controlledObject;
    m_Speed = speed;
    m_pKeyStates = NULL;
    m_MouseLeftButtonDown = m_MouseRightButtonDown = false;
}

void ActorController::HandleAction(ActionType actionType)
{
    switch(actionType)
    {
        case ActionType_Fire:
        {
            shared_ptr<EventData_Actor_Fire> pClimbEvent(new EventData_Actor_Fire(m_pControlledObject->VGetProperties()->GetActorId()));
            IEventMgr::Get()->VTriggerEvent(pClimbEvent);
            break;
        }
        case ActionType_Attack:
        {
            shared_ptr<EventData_Actor_Attack> pClimbEvent(new EventData_Actor_Attack(m_pControlledObject->VGetProperties()->GetActorId()));
            IEventMgr::Get()->VTriggerEvent(pClimbEvent);
            break;
        }
        case ActionType_Change_Ammo_Type:
        {
            shared_ptr<EventData_Request_Change_Ammo_Type> pEvent(new EventData_Request_Change_Ammo_Type(m_pControlledObject->VGetProperties()->GetActorId(), AmmoType_Max, false));
            IEventMgr::Get()->VTriggerEvent(pEvent);
            break;
        }
        case ActionType_Change_Ammo_Type_Prev:
        {
            shared_ptr<EventData_Request_Change_Ammo_Type> pEvent(new EventData_Request_Change_Ammo_Type(m_pControlledObject->VGetProperties()->GetActorId(), AmmoType_Max, true));
            IEventMgr::Get()->VTriggerEvent(pEvent);
            break;
        }
    }
}

void ActorController::OnUpdate(uint32 msDiff)
{
    // Don't drive the player while a menu (e.g. the ingame quick menu) is open.
    // OnUpdate reads raw held-key state every frame, so even keys held down
    // before the menu opened would otherwise keep moving the character in the
    // background. Gating here is the single choke point for all held input.
    if (g_pApp->IsMenuActive())
    {
        // Zero the stored analog-stick value. Unlike keyboard state (polled fresh
        // each frame), the gamepad axis is only updated by events — which are
        // dropped while the menu is open. Without this, releasing/centering the
        // stick during the pause is never registered, so the stale value would
        // keep moving the character the moment the menu closes.
        m_GamepadMoveX = 0.0f;
        m_GamepadMoveY = 0.0f;
        return;
    }

    int count;
    SDL_PumpEvents();
    m_pKeyStates = SDL_GetKeyboardState(&count);

    float moveX = 0.0f;
    float moveY = 0.0f;

    float climbY = 0.0f;

    // We need two conditions because I want behaviour such as when both right and left
    // buttons are pressed, I dont want actor to move, e.g. to have the move effect nullyfied

    if (g_pApp->GetControlOptions()->useAlternateControls)
    {
        if (m_InputKeys[SDLK_d])
        {
            moveX += m_Speed * (float)msDiff;
        }
        if (m_InputKeys[SDLK_a])
        {
            moveX -= m_Speed * (float)msDiff;
        }

        // CLimbing
        if (m_InputKeys[SDLK_s])
        {
            climbY += 5.0;
        }
        if (m_InputKeys[SDLK_w])
        {
            climbY -= 5.0;
        }

        // Jumping
        if (m_InputKeys[SDLK_SPACE] || m_InputKeys[SDLK_w])
        {
            moveY -= m_Speed * (float)msDiff;
        }
    }
    else
    {
        if (m_InputKeys[SDLK_RIGHT])
        {
            moveX += m_Speed * (float)msDiff;
        }
        if (m_InputKeys[SDLK_LEFT])
        {
            moveX -= m_Speed * (float)msDiff;
        }

        // CLimbing
        if (m_InputKeys[SDLK_DOWN])
        {
            climbY += 5.0;
        }
        if (m_InputKeys[SDLK_UP])
        {
            climbY -= 5.0;
        }

        // Jumping
        if (m_InputKeys[SDLK_SPACE])
        {
            moveY -= m_Speed * (float)msDiff;
        }
    }

    // Gamepad analog stick movement (combines with keyboard)
    if (fabs(m_GamepadMoveX) > 0.01f)
    {
        moveX += m_GamepadMoveX * m_Speed * (float)msDiff;
    }
    // Left stick Y: used for climbing and looking up/down (not jumping - use A button)
    if (fabs(m_GamepadMoveY) > 0.3f)
    {
        // Analog climbing/looking (negative Y = up = climb up/look up, positive Y = down = climb down/duck)
        climbY += m_GamepadMoveY * 5.0f;
    }

    if (fabs(climbY) > FLT_EPSILON)
    {
        shared_ptr<EventData_Start_Climb> pClimbEvent(new EventData_Start_Climb(m_pControlledObject->VGetProperties()->GetActorId(), Point(0, climbY)));
        IEventMgr::Get()->VTriggerEvent(pClimbEvent);
    }
    if (fabs(moveX) > FLT_EPSILON || fabs(moveY) > FLT_EPSILON)
    {
        shared_ptr<EventData_Actor_Start_Move> pMoveEvent(new EventData_Actor_Start_Move(m_pControlledObject->VGetProperties()->GetActorId(), Point(moveX, moveY)));
        IEventMgr::Get()->VTriggerEvent(pMoveEvent);
    }
}

bool ActorController::VOnKeyDown(SDL_Keycode key)
{
    if (g_pApp->GetControlOptions()->useAlternateControls)
    {
        if (key == SDLK_e)
        {
            HandleAction(ActionType_Change_Ammo_Type);
            return true;
        }
        else if (SDL_GetScancodeFromKey(key) == SDL_SCANCODE_LSHIFT)
        {
            HandleAction(ActionType_Change_Ammo_Type);
            return true;
        }
    }
    else
    {
        if (SDL_GetScancodeFromKey(key) == SDL_SCANCODE_LALT)
        {
            HandleAction(ActionType_Fire);
            return true;
        }
        else if (SDL_GetScancodeFromKey(key) == SDL_SCANCODE_LCTRL)
        {
            HandleAction(ActionType_Attack);
            return true;
        }
        else if (SDL_GetScancodeFromKey(key) == SDL_SCANCODE_LSHIFT)
        {
            HandleAction(ActionType_Change_Ammo_Type);
            return true;
        }
    }

    m_InputKeys[key] = true;

    return false;
}

bool ActorController::VOnKeyUp(SDL_Keycode key)
{
    if (g_pApp->GetControlOptions()->useAlternateControls)
    {

    }
    else
    {
        if (SDL_GetScancodeFromKey(key) == SDL_SCANCODE_LALT)
        {
            shared_ptr<EventData_Actor_Fire_Ended> pFireEndedEvent(new EventData_Actor_Fire_Ended(m_pControlledObject->VGetProperties()->GetActorId()));
            IEventMgr::Get()->VTriggerEvent(pFireEndedEvent);

            return true;
        }
    }

    m_InputKeys[key] = false;
    return false;
}

bool ActorController::VOnPointerMove(SDL_MouseMotionEvent& mouseEvent)
{
    return false;
}

bool ActorController::VOnPointerButtonDown(SDL_MouseButtonEvent& mouseEvent)
{
    if (mouseEvent.button == SDL_BUTTON_LEFT)
    {
        m_MouseLeftButtonDown = true;

        if (g_pApp->GetControlOptions()->useAlternateControls)
        {
            HandleAction(ActionType_Fire);
        }
        
        return true;
    }
    else if (mouseEvent.button == SDL_BUTTON_RIGHT)
    {
        m_MouseRightButtonDown = true;

        if (g_pApp->GetControlOptions()->useAlternateControls)
        {
            HandleAction(ActionType_Attack);
        }

        return true;
    }

    return false;
}

bool ActorController::VOnPointerButtonUp(SDL_MouseButtonEvent& mouseEvent)
{
    if (mouseEvent.button == SDL_BUTTON_LEFT)
    {
        shared_ptr<EventData_Actor_Fire_Ended> pFireEndedEvent(new EventData_Actor_Fire_Ended(m_pControlledObject->VGetProperties()->GetActorId()));
        IEventMgr::Get()->VTriggerEvent(pFireEndedEvent);

        m_MouseLeftButtonDown = false;
        return true;
    }
    else if (mouseEvent.button == SDL_BUTTON_RIGHT)
    {
        m_MouseRightButtonDown = false;
        return true;
    }

    return false;
}

std::vector<std::shared_ptr<AbstractRecognizer>> ActorController::VRegisterRecognizers() {
    const float thresholdDistance = g_pApp->GetControlOptions()->touchScreen.distanceThreshold;
    const unsigned int thresholdMs = g_pApp->GetControlOptions()->touchScreen.timeThreshold;

    // TODO: Try to describe it in config?
    std::vector<std::shared_ptr<AbstractRecognizer>> recognizers{};
    recognizers.reserve(6);

    // Pause button. Left up screen corner. Maximum priority
    auto pauseRecognizer = std::make_shared<TapRecognizer>(PAUSE_TAP_RECOGNIZER, 100);
    pauseRecognizer->SetFrame(Rect{0.0f, 0.0f, 0.1f, 0.1f});
    recognizers.push_back(pauseRecognizer);

    // Change weapon button. Right up screen corner. Maximum priority
    auto weaponRecognizer = std::make_shared<TapRecognizer>(WEAPON_TAP_RECOGNIZER, 99);
    weaponRecognizer->SetFrame(Rect{0.9f, 0.0f, 0.1f, 0.2f});
    recognizers.push_back(weaponRecognizer);

    // Attack button. All screen area. Minimum priority
    auto attackRecognizer = std::make_shared<TapRecognizer>(ATTACK_TAP_RECOGNIZER, 1);
    recognizers.push_back(attackRecognizer);

    // Movement controller. Left part of screen. Medium priority
    auto joystick = std::make_shared<JoystickRecognizer>(MOVEMENT_JOYSTICK_RECOGNIZER, 50, thresholdDistance, thresholdMs + 100);
    joystick->SetFrame(Rect{0.0f, 0.0f, 0.5f, 1.0f});
    recognizers.push_back(joystick);

    // Jump button. Right part of screen. Minimum priority but greater that attack button
    auto jumpButton = std::make_shared<PressRecognizer>(JUMP_PRESS_RECOGNIZER, 20, thresholdDistance, thresholdMs);
    jumpButton->SetFrame(Rect{0.5f, 0.0f, 0.5f, 1.0f});
    recognizers.push_back(jumpButton);

    // Swipe to attack. Right part of screen. Medium priority (greater than jump button)
    auto swipe = std::make_shared<SwipeRecognizer>(PROJECTILE_SWIPE_RECOGNIZER, 51, thresholdDistance, thresholdMs);
    swipe->SetFrame(Rect{0.5f, 0.0f, 0.5f, 1.0f});
    recognizers.push_back(swipe);

    return recognizers;
}

bool ActorController::VOnTouch(const Touch_Event &evt) {
    auto code = evt.sdlEvent.user.code;
    switch (code) {
        case TAP:
            return OnTap(evt.recognizerId, evt.customData.tap);
        case JOYSTICK_MOVE:
        case JOYSTICK_RESET:
            return OnJoystick(evt.recognizerId, evt.customData.joystick);
        case SWIPE_START:
        case SWIPE_END:
            return OnSwipe(evt.recognizerId, evt.customData.swipe, code == SWIPE_START);
        case PRESS_START:
        case PRESS_END:
            return OnPress(evt.recognizerId, evt.customData.press, code == PRESS_START);
    }
    return false;
}

bool ActorController::OnTap(int id, const Touch_TapEvent &evt) {
    SDL_Keycode key;
    switch (id) {
        case PAUSE_TAP_RECOGNIZER:
        {
            // Open/close the ingame menu by feeding Escape through the same
            // dispatch path the engine uses. SDL_PushEvent is not read on the
            // WASM build (the main loop drains a custom queue, not SDL's), so
            // route directly via BaseGameApp::OnEvent instead.
            key = SDLK_ESCAPE;

            SDL_Event event{0};
            event.type = SDL_KEYDOWN;
            event.key.keysym.sym = key;
            event.key.keysym.scancode = SDL_GetScancodeFromKey(key);
            g_pApp->OnEvent(event);
            event.type = SDL_KEYUP;
            g_pApp->OnEvent(event);
            HapticFeedback::Trigger(HapticPreset::Light);
            return true;
        }
        case WEAPON_TAP_RECOGNIZER:
            key = SDLK_LSHIFT;
            HapticFeedback::Trigger(HapticPreset::Light);
            break;
        case ATTACK_TAP_RECOGNIZER:
            key = SDLK_LCTRL;
            HapticFeedback::Trigger(HapticPreset::Attack);
            break;
        default:
            key = SDLK_UNKNOWN;
            break;
    }

    if (key != SDLK_UNKNOWN) {
        VOnKeyDown(key);
        return VOnKeyUp(key);
    }
    return false;
}

bool ActorController::OnJoystick(int id, const Touch_JoystickEvent &evt) {
    if (id == MOVEMENT_JOYSTICK_RECOGNIZER) {
        const auto dx = evt.dx;
        const auto dy = evt.dy;

        const float deadzone = 0.05 * 0.05;

        SDL_Keycode horizontalKey = SDLK_UNKNOWN;
        SDL_Keycode verticalKey = SDLK_UNKNOWN;
        if (dx * dx + dy * dy > deadzone) {

            if (dx * 2 < dy && dx * 2 > -dy) {
                // 247 - 292 deg
                verticalKey = SDLK_DOWN;
            } else if (dx * 2 < -dy && dx * 2 > dy) {
                // 67 deg - 112 deg
                verticalKey = SDLK_UP;
            }

            if (dy < dx * 2 && dy > -dx * 2) {
                // 292 - 67 deg
                horizontalKey = SDLK_RIGHT;
            } else if (dy < -dx * 2 && dy > dx * 2) {
                // 112 - 247 deg
                horizontalKey = SDLK_LEFT;
            }
        }

        SDL_Keycode keys[] = {SDLK_LEFT, SDLK_UP, SDLK_RIGHT, SDLK_DOWN};
        for (SDL_Keycode key : keys) {
            bool isPressed = m_InputKeys[key];
            bool press = key == horizontalKey || key == verticalKey;
            if (!isPressed && press) {
                VOnKeyDown(key);
            } else if (isPressed && !press) {
                VOnKeyUp(key);
            }
        }

        return true;
    }
    return false;
}

bool ActorController::OnSwipe(int id, const Touch_SwipeEvent &evt, bool start) {
    if (id == PROJECTILE_SWIPE_RECOGNIZER) {
        if (start) {
            HapticFeedback::Trigger(HapticPreset::Attack); // fire, matches gamepad B
            return VOnKeyDown(SDLK_LALT);
        } else {
            return VOnKeyUp(SDLK_LALT);
        }
    }
    return false;
}

bool ActorController::OnPress(int id, const Touch_PressEvent &evt, bool start) {
    if (id == JUMP_PRESS_RECOGNIZER) {
        if (start) {
            // Jump haptic now fires in ClawControllableComponent::VOnStartJumping
            // (covers every jump regardless of input), so none needed here.
            return VOnKeyDown(SDLK_SPACE);
        } else {
            return VOnKeyUp(SDLK_SPACE);
        }
    }
    return false;
}

//-----------------------------------------------------------------------------
// IGamepadHandler implementation
//-----------------------------------------------------------------------------

bool ActorController::VOnGamepadButtonDown(GamepadButton button, float value)
{
    switch (button)
    {
        case GamepadButton::A:  // Jump
            return VOnKeyDown(SDLK_SPACE);

        case GamepadButton::X:  // Attack (sword)
            HapticFeedback::Trigger(HapticPreset::Attack);
            HandleAction(ActionType_Attack);
            return true;

        case GamepadButton::B:  // Fire (pistol/magic/dynamite)
            HapticFeedback::Trigger(HapticPreset::Attack);
            HandleAction(ActionType_Fire);
            return true;

        // Y button disabled (use LB/RB for ammo change)
        // Start button handled in HumanView for pause menu

        case GamepadButton::DPadUp:
            return VOnKeyDown(SDLK_UP);
        case GamepadButton::DPadDown:
            return VOnKeyDown(SDLK_DOWN);
        case GamepadButton::DPadLeft:
            return VOnKeyDown(SDLK_LEFT);
        case GamepadButton::DPadRight:
            return VOnKeyDown(SDLK_RIGHT);

        // Triggers disabled - use face buttons instead

        case GamepadButton::LeftBumper:  // Change ammo (previous)
            HandleAction(ActionType_Change_Ammo_Type_Prev);
            return true;

        case GamepadButton::RightBumper:  // Change ammo (next)
            HandleAction(ActionType_Change_Ammo_Type);
            return true;

        default:
            return false;
    }
}

bool ActorController::VOnGamepadButtonUp(GamepadButton button)
{
    switch (button)
    {
        case GamepadButton::A:
            return VOnKeyUp(SDLK_SPACE);

        case GamepadButton::B:
        {
            shared_ptr<EventData_Actor_Fire_Ended> pFireEndedEvent(
                new EventData_Actor_Fire_Ended(m_pControlledObject->VGetProperties()->GetActorId()));
            IEventMgr::Get()->VTriggerEvent(pFireEndedEvent);
            return true;
        }

        case GamepadButton::DPadUp:
            return VOnKeyUp(SDLK_UP);
        case GamepadButton::DPadDown:
            return VOnKeyUp(SDLK_DOWN);
        case GamepadButton::DPadLeft:
            return VOnKeyUp(SDLK_LEFT);
        case GamepadButton::DPadRight:
            return VOnKeyUp(SDLK_RIGHT);

        default:
            return false;
    }
}

bool ActorController::VOnGamepadAxis(GamepadAxis axis, float value)
{
    switch (axis)
    {
        case GamepadAxis::LeftStickX:
            m_GamepadMoveX = value;
            return true;

        case GamepadAxis::LeftStickY:
            m_GamepadMoveY = value;
            return true;

        default:
            return false;
    }
}
