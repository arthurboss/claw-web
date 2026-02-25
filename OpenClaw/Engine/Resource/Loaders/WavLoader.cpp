#include "WavLoader.h"

#include "../../GameApp/BaseGameApp.h"

#ifdef __EMSCRIPTEN__
#include "../../Audio/WebAudioAPI.h"
#endif

//=================================================================================================
// class WavResourceExtraData
//
//     This class implements the IResourceExtraData
//

// Custom deallocator for SDL stuff
void DeleteMixChunk(Mix_Chunk* sound)
{
    Mix_FreeChunk(sound);
}

WavResourceExtraData::~WavResourceExtraData()
{
    // Sound takes care of its own destruction
}

void WavResourceExtraData::LoadWavSound(char* rawBuffer, uint32 size)
{
    // Create a dummy Mix_Chunk for compatibility
    _sound = shared_ptr<Mix_Chunk>(new Mix_Chunk(), DeleteMixChunk);

    // CRITICAL FIX: Copy the buffer instead of storing the pointer
    // The rawBuffer will be deleted by ResourceCache after VLoadResource returns,
    // so we need our own copy of the WAV data
    _sound->abuf = new Uint8[size];
    memcpy(_sound->abuf, rawBuffer, size);
    _sound->alen = size;
    _sound->allocated = 1; // Mark as allocated so SDL will free it

    // The actual audio loading will be handled by the Audio class when PlaySound is called
}

//=================================================================================================
// class WavResourceLoader
//
//     This class implements the IResourceLoader interface with WAV sound format
//

bool WavResourceLoader::VLoadResource(char* rawBuffer, uint32 rawSize, std::shared_ptr<ResourceHandle> handle)
{
    if (rawSize <= 0 || rawBuffer == NULL)
    {
        LOG_ERROR("Received invalid rawBuffer or its size");
        return false;
    }

    shared_ptr<WavResourceExtraData> extraData = shared_ptr<WavResourceExtraData>(new WavResourceExtraData());
    extraData->LoadWavSound(rawBuffer, rawSize);

    if (extraData->GetSound() == NULL)
    {
        LOG_ERROR("Failed to load sound. Is sound system initialized ?");
    }

    handle->SetExtraData(extraData);

    return true;
}

uint32 WavResourceLoader::VGetLoadedResourceSize(char* rawBuffer, uint32 rawSize)
{
#ifdef __EMSCRIPTEN__
    // For Emscripten, just return the raw size since we don't need to decode
    return rawSize;
#else
    // TODO: This is inefficent, this resource gets basically loaded twice, once just
    // to find out how much room it takes and second time to actually create it
    SDL_RWops* soundRwOps = SDL_RWFromMem((void*)rawBuffer, rawSize);
    auto pSound = shared_ptr<Mix_Chunk>(Mix_LoadWAV_RW(soundRwOps, 1), DeleteMixChunk);
    return pSound->alen;
#endif
}

shared_ptr<Mix_Chunk> WavResourceLoader::LoadAndReturnSound(const char* resourceString)
{
    Resource resource(resourceString);

    shared_ptr<ResourceHandle> handle = g_pApp->GetResourceCache()->GetHandle(&resource);
    shared_ptr<WavResourceExtraData> extraData = std::static_pointer_cast<WavResourceExtraData>(handle->GetExtraData());

    if (!extraData)
    {
        LOG_ERROR("Could not cast type to WavResourceExtraData. Check if WavResourceLoader is registered.");
        return NULL;
    }

    return extraData->GetSound();
}

std::shared_ptr<WavResourceLoader> WavResourceLoader::Create()
{
    return shared_ptr<WavResourceLoader>(new WavResourceLoader());
}