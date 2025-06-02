// Copyright (c) 2019 MaochiGames.


#include "AudioManager/AudioManager.h"

DEFINE_LOG_CATEGORY(AudioManager);

void UAudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	bHasBeenInitialized  = false;
	InitialWorldContextObject = nullptr;
	AudioManaged2dAssets = nullptr;
	ActiveLoadingScreenEventInstance = nullptr;
	RadioChattersEventInstances = {};
}

void UAudioManager::Deinitialize()
{
	Super::Deinitialize();

	bHasBeenInitialized  = false;
	InitialWorldContextObject = nullptr;
	AudioManaged2dAssets = nullptr;
	ActiveLoadingScreenEventInstance = nullptr;
	RadioChattersEventInstances = {};
}

bool UAudioManager::AudioInitialize(const UObject* InInitialWorldContextObject, UAudioManaged2dAssets* InAudioManaged2dAssets, const int32& InLoadingScreenMusicIndex)
{
	InitialWorldContextObject = InInitialWorldContextObject;
	
	bool bIsInitialized = false;
	if(InAudioManaged2dAssets == nullptr)
	{
		bHasBeenInitialized = bIsInitialized;
		return bIsInitialized;
	}
	
	AudioManaged2dAssets = InAudioManaged2dAssets;

	if(UWorld* World = InitialWorldContextObject->GetWorld())
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// We also need to spawn the loading screen music.
		ActiveLoadingScreenEventInstance = World->SpawnActor<AAudio3dManagedObject>(InAudioManaged2dAssets->LoadingScreenMusicEvent[InLoadingScreenMusicIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);

		// We also spawn radio chatters.
		if(InAudioManaged2dAssets->RadioChatters.Num() > 0)
		{
			for(int i = 0; i < InAudioManaged2dAssets->RadioChatters.Num(); i++)
			{
				if(InAudioManaged2dAssets->RadioChatters[i] != nullptr)
				{
					AAudio3dManagedObject* SpawnedAudio3dManagedObject = World->SpawnActor<AAudio3dManagedObject>(InAudioManaged2dAssets->RadioChatters[i], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
					RadioChattersEventInstances.Add(SpawnedAudio3dManagedObject);
				}
			}
		}
	}

	bIsInitialized = true;
	bHasBeenInitialized = bIsInitialized;
	
	return bIsInitialized;
}

void UAudioManager::PlayLoadingScreen(float InVolume)
{
	if(!bHasBeenInitialized)
	{
		UE_LOG(AudioManager, Error, TEXT("UAudioManager::PlayLoadingScreen : Audio manager has not been initialized"));
		return;
	}

	if(ActiveLoadingScreenEventInstance == nullptr)
	{
		UE_LOG(AudioManager, Error, TEXT("UAudioManager::PlayLoadingScreen : No active loading screen event instance"));
		return;
	}

	ActiveLoadingScreenEventInstance->Exec_PlayAudioEventWithParameterValue(InVolume);
}

void UAudioManager::AdjustVolumeOnLoadingScreen(float InVolume)
{
	if(!bHasBeenInitialized)
	{
		UE_LOG(AudioManager, Error, TEXT("UAudioManager::AdjustVolumeOnLoadingScreen : Audio manager has not been initialized"));
		return;
	}
	
	if(ActiveLoadingScreenEventInstance == nullptr)
	{
		UE_LOG(AudioManager, Error, TEXT("UAudioManager::AdjustVolumeOnLoadingScreen : No active loading screen event instance"));
		return;
	}

	ActiveLoadingScreenEventInstance->Exec_PlayAudioEventWithParameterValue(InVolume, true);
}

void UAudioManager::StopPLayingLoadingScreen()
{
	if(!bHasBeenInitialized)
	{
		UE_LOG(AudioManager, Error, TEXT("UAudioManager::StopPLayingLoadingScreen : Audio manager has not been initialized"));
		return;
	}

	if(ActiveLoadingScreenEventInstance == nullptr)
	{
		UE_LOG(AudioManager, Error, TEXT("UAudioManager::StopPLayingLoadingScreen : No active loading screen event instance"));
		return;
	}

	ActiveLoadingScreenEventInstance->Exec_StopAudioEvent();
}

void UAudioManager::PlayRadioChatter(FString InRadioChatterName)
{
	if(!bHasBeenInitialized)
	{
		UE_LOG(AudioManager, Error, TEXT("UAudioManager::PlayRadioChatter : Audio manager has not been initialized"));
		return;
	}

	if(RadioChattersEventInstances.Num() < 0)
	{
		UE_LOG(AudioManager, Error, TEXT("UAudioManager::PlayRadioChatter : Radio chatters have not been initialized"));
		return;
	}

	for(int i = 0; i < RadioChattersEventInstances.Num(); i++)
	{
		// We compare but we ignore case sensitive.
		if(RadioChattersEventInstances[i]->IdentifierName.Equals(InRadioChatterName, ESearchCase::IgnoreCase))
		{
			RadioChattersEventInstances[i]->Exec_PlayAudioEvent();
			break;
		}
	}
}
