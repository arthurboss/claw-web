#include "LocalAmbientSoundComponent.h"
#include "../../GameApp/BaseGameApp.h"
#include "../../GameApp/BaseGameLogic.h"
#include "PositionComponent.h"
#include "../../UserInterface/HumanView.h"
#include "../../Audio/Audio.h"
#include "../../Resource/Loaders/WavLoader.h"

const char* LocalAmbientSoundComponent::g_Name = "LocalAmbientSoundComponent";

LocalAmbientSoundComponent::LocalAmbientSoundComponent()
    :
    m_ActorsInTriggerArea(0),
    m_pActorInArea(NULL),
    m_DiagonalLength(0.0),
    m_SoundChannel(-1)
{

}

bool LocalAmbientSoundComponent::VInit(TiXmlElement* pData)
{
    assert(pData != NULL);

    m_Properties.LoadFromXml(pData, true);

    m_DiagonalLength = m_Properties.soundAreaSize.Length() / 2;

    return true;
}

TiXmlElement* LocalAmbientSoundComponent::VGenerateXml()
{
    return m_Properties.ToXml();
}

void LocalAmbientSoundComponent::VPostInit()
{
    shared_ptr<TriggerComponent> pTriggerComponent =
        MakeStrongPtr(m_pOwner->GetComponent<TriggerComponent>());
    assert(pTriggerComponent != nullptr);

    pTriggerComponent->AddObserver(this);
}

void LocalAmbientSoundComponent::VPostPostInit()
{
    assert(!m_Properties.soundAreaSize.IsZeroXY());
    if (!m_Properties.soundAreaSize.IsZeroXY())
    {
        ActorFixtureDef fixtureDef;
        fixtureDef.collisionShape = "Rectangle";
        fixtureDef.fixtureType = FixtureType_Trigger;
        fixtureDef.size = m_Properties.soundAreaSize;
        fixtureDef.offset = m_Properties.soundAreaOffset;
        fixtureDef.collisionMask = CollisionFlag_Controller | CollisionFlag_InvisibleController;
        fixtureDef.collisionFlag = CollisionFlag_Trigger;
        fixtureDef.isSensor = true;

        g_pApp->GetGameLogic()->VGetGamePhysics()->VAddActorFixtureToBody(m_pOwner->GetGUID(), &fixtureDef);
    }
}

void LocalAmbientSoundComponent::VUpdate(uint32 msDiff)
{
    if (m_ActorsInTriggerArea > 0)
    {
        UpdateAmbientSound();
    }
}

void LocalAmbientSoundComponent::VOnActorEnteredTrigger(Actor* pActorWhoEntered, FixtureType triggerType)
{
    m_ActorsInTriggerArea++;
    m_pActorInArea = pActorWhoEntered;
    assert(m_ActorsInTriggerArea == 1);

    PlayAmbientSound();
}

void LocalAmbientSoundComponent::VOnActorLeftTrigger(Actor* pActorWhoLeft, FixtureType triggerType)
{
    m_ActorsInTriggerArea--;
    m_pActorInArea = NULL;
    assert(m_ActorsInTriggerArea == 0);

    StopAmbientSound();
}

void LocalAmbientSoundComponent::PlayAmbientSound()
{
    // Defensive: Stop existing sound if already playing
    if (m_SoundChannel != -1)
    {
        StopAmbientSound();
    }

    assert(m_pActorInArea != NULL);

#ifdef __EMSCRIPTEN__
    // TODO: [EMSCRIPTEN] Try to implement Mix_Group* functions
    // For now, ambient sounds are not supported in Emscripten builds
    m_SoundChannel = -1;
    return;
#endif

    m_SoundChannel = Mix_GroupAvailable(1);
    if (m_SoundChannel == -1)
    {
        // No available sound channel, silently fail
        return;
    }

    shared_ptr<Mix_Chunk> pSound = WavResourceLoader::LoadAndReturnSound(m_Properties.sound.c_str());
    if (pSound == nullptr)
    {
        // Failed to load sound, release channel and fail
        m_SoundChannel = -1;
        return;
    }

    int globalVolume = (int)((((float)g_pApp->GetAudio()->GetSoundVolume()) / 100.0f) * (float)MIX_MAX_VOLUME);
    int chunkVolume = (int)((((float)m_Properties.volume) / 100.0f) * (float)globalVolume);
    Mix_VolumeChunk(pSound.get(), chunkVolume);

    m_SoundChannel = Mix_PlayChannel(m_SoundChannel, pSound.get(), -1);

    // Set positional properties
    UpdateAmbientSound();
}

void LocalAmbientSoundComponent::StopAmbientSound()
{
    // Defensive: If no valid sound channel was allocated, nothing to stop
    if (m_SoundChannel == -1)
    {
        return;
    }

    Mix_HaltChannel(m_SoundChannel);
    m_SoundChannel = -1;
}

void LocalAmbientSoundComponent::UpdateAmbientSound()
{
#ifdef __EMSCRIPTEN__
    // TODO: [EMSCRIPTEN] Try to implement Mix_SetDistance
    return;
#endif
    // Defensive: Don't update if no valid sound channel or actor
    if (m_SoundChannel == -1 || m_pActorInArea == NULL)
    {
        return;
    }

    Point soundDistanceDelta =
        m_pOwner->GetPositionComponent()->GetPosition() - m_pActorInArea->GetPositionComponent()->GetPosition();
    double distance = soundDistanceDelta.Length();
    double distanceRatio = distance / m_DiagonalLength;

    int sdlDistance = min(distanceRatio * 255, (double)255);

    Mix_SetDistance(m_SoundChannel, sdlDistance);

    /*LOG("Distance ratio: " + ToStr(distanceRatio));
    LOG("Distance: " + ToStr(distance));*/
}