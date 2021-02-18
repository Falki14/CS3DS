//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
//
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
//
//-------------------------------------------------------------------------------------

#include "../../Dependencies/include/png.h"
#include "../../Dependencies/include/fmod.h"

#include "../../include/JSoundSystem.h"
#include "../../include/JFileSystem.h"

#include <cmath>

//////////////////////////////////////////////////////////////////////////
JMusic::JMusic()
{

}

JMusic::~JMusic()
{
    JSoundSystem::GetInstance()->StopMusic(this);
    //JSoundSystem::GetInstance()->FreeMusic(this);

//    if (mTrack)
//        FSOUND_Sample_Free(mTrack);
}


//////////////////////////////////////////////////////////////////////////
JSample::JSample()
{
    mVoice = -1;
    mVolume = 255;
    mPanning = 127;
}

JSample::~JSample()
{

    if (mSample)
        linearFree(mSample);
}


//////////////////////////////////////////////////////////////////////////
JSoundSystem* JSoundSystem::mInstance = NULL;


JSoundSystem* JSoundSystem::GetInstance()
{
    if (mInstance == NULL)
    {
        mInstance = new JSoundSystem();
        mInstance->InitSoundSystem();
    }

    return mInstance;
}


void JSoundSystem::Destroy()
{
    if (mInstance)
    {
        mInstance->DestroySoundSystem();
        delete mInstance;
        mInstance = NULL;
    }
}


JSoundSystem::JSoundSystem()
{

}


JSoundSystem::~JSoundSystem()
{

}


void JSoundSystem::InitSoundSystem()
{
//    FSOUND_Init(44100, 32, 0);
}


void JSoundSystem::DestroySoundSystem()
{
//    FSOUND_Close();

}


JMusic *JSoundSystem::LoadMusic(const char *fileName)
{
    JMusic* music = new JMusic();
//    if (music)
//    {
//        JFileSystem* fileSystem = JFileSystem::GetInstance();
//        if (fileSystem->OpenFile(fileName))
//        {
//
//            //		FMUSIC is for MOD...
//            // 		int size = fileSystem->GetFileSize();
//            // 		char *buffer = new char[size];
//            // 		fileSystem->ReadFile(buffer, size);
//            // 		music->mTrack = FMUSIC_LoadSongEx(buffer, 0, size, FSOUND_LOADMEMORY, NULL, 0);
//            //
//            // 		delete[] buffer;
//            // 		fileSystem->CloseFile();
//
//            int size = fileSystem->GetFileSize();
//            char *buffer = new char[size];
//            fileSystem->ReadFile(buffer, size);
//            music->mTrack = FSOUND_Sample_Load(FSOUND_UNMANAGED, buffer, FSOUND_LOADMEMORY, 0, size);
//
//            delete[] buffer;
//            fileSystem->CloseFile();
//        }
//
//    }

    return music;
}

// void JSoundSystem::FreeMusic(JMusic *music)
// {
// 	if (music)
// 	{
// 		// 		if (music->mTrack)
// 		// 			FMUSIC_FreeSong(music->mTrack);
// 		// 		delete music;
// 		// 		music = NULL;
//
// 		if (music->mTrack)
// 			FSOUND_Sample_Free(music->mTrack);
//
// 		//delete music;
// 		//music = NULL;
// 	}
// }

void JSoundSystem::PlayMusic(JMusic *music, bool looping)
{
    // 	if (music && music->mTrack)
    // 	{
    // 		FMUSIC_SetLooping(music->mTrack, (looping?1:0));
    // 		FMUSIC_PlaySong(music->mTrack);
    // 	}

//    if (music && music->mTrack)
//    {
//        mChannel = FSOUND_PlaySound(FSOUND_FREE, music->mTrack);
//
//        if (looping)
//            FSOUND_SetLoopMode(mChannel, FSOUND_LOOP_NORMAL);
//        else
//            FSOUND_SetLoopMode(mChannel, FSOUND_LOOP_OFF);
//
//    }
}


void JSoundSystem::StopMusic(JMusic *music)
{
    // 	if (music && music->mTrack)
    // 		FMUSIC_StopSong(music->mTrack);

//    FSOUND_StopSound(mChannel);
}


void JSoundSystem::SetVolume(int volume)
{
    //	if (music && music->mTrack)
    //		FMUSIC_SetMasterVolume(music->mTrack, volume);
}

JSample *JSoundSystem::LoadSample(const char *fileName)
{
    JSample* sample = new JSample();
    if (sample)
    {
        JFileSystem* fileSystem = JFileSystem::GetInstance();
        if (fileSystem->OpenFile(fileName))
        {
            int size = fileSystem->GetFileSize();
            u8 *buffer = (u8 *)linearAlloc(size);
            fileSystem->ReadFile(buffer, size);
            ndspWaveBuf waveBuf;
            memset(&waveBuf,0,sizeof(ndspWaveBuf));
            waveBuf.data_vaddr = &buffer[0];
            waveBuf.nsamples = size;
            sample->mSample = (u32 *)buffer;
            sample->waveBuf = waveBuf;
            for (int i = 0; i < size; ++i) {
                buffer[i] = buffer[i] + 0x80;
            }
            DSP_FlushDataCache(buffer, size);
            fileSystem->CloseFile();
        }

    }

    return sample;
}


// void JSoundSystem::FreeSample(JSample *sample)
// {
// 	if (sample)
// 	{
// 		if (sample->mSample)
// 			FSOUND_Sample_Free(sample->mSample);
// 
// 		//delete sample;
// 		//sample = NULL;
// 	}
// 
// }

void JSoundSystem::PlaySample(JSample *sample)
{
    float panLeft = cos(M_PI/2.0f * max(0.0f, (float)(sample->mPanning-1)/256.0f));
    float panRight =  sin(M_PI/2.0f * max(0.0f, (float)(sample->mPanning-1)/256.0f));
    float volume = (float)sample->mVolume / 256.0f;
    float mix[12];
    memset(mix, 0, sizeof(mix));
    float lVol = panLeft * volume;
    float rVol = panRight * volume;
    mix[0] = lVol;
    mix[1] = rVol;
    for (int i = 0; i < 24; ++i) {
        if (!ndspChnIsPlaying(i)) {
            ndspChnWaveBufClear(i);
            ndspChnSetMix(i, mix);
            ndspChnWaveBufAdd(i, &sample->waveBuf);
            return;
        }
    }

    ndspChnWaveBufClear(0);
    ndspChnSetMix(0, mix);
    ndspChnWaveBufAdd(0, &sample->waveBuf);
}

void JSoundSystem::StopSample(int voice)
{

}