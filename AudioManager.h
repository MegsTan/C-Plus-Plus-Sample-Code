// Copyright (c) 2019 MaochiGames.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FMODEvent.h"
#include "FMODAudioComponent.h"
#include "FMODBlueprintStatics.h"

#include "AudioManager/AudioManaged2dAssets.h"

#include "AudioManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(AudioManager, Log, All);

/**
 * 
 */
UCLASS()
class TITANZERO_API UAudioManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	UPROPERTY()
	bool bHasBeenInitialized;

	UPROPERTY()
	const UObject* InitialWorldContextObject;
	
	UPROPERTY()
	UAudioManaged2dAssets* AudioManaged2dAssets;

	UPROPERTY()
	AAudio3dManagedObject* ActiveLoadingScreenEventInstance;

	UPROPERTY()
	TArray<AAudio3dManagedObject*> RadioChattersEventInstances;

public:
	/**
	 * Initializes the audio using the object that uses this. Highly suggest to call this from the local player controller.
	 * @param InAudioManaged2dAssets Incoming audio managed 2d assets.
	 * @param InLoadingScreenMusicIndex Incoming loading screen music index.
	 * @return Bool if has been initialized or not.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Audio Initialize", WorldContext = "InInitialWorldContextObject"), Category = "Titan Zero|Audio Manager|Common")
	bool AudioInitialize(const UObject* InInitialWorldContextObject, UAudioManaged2dAssets* InAudioManaged2dAssets, const int32& InLoadingScreenMusicIndex);

	/**
	 * Plays the loading screen.
	 * @param InVolume Incoming loading screen volume.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Play Loading Screen"), Category = "Titan Zero|Audio Manager|Common")
	void PlayLoadingScreen(float InVolume);

	/**
	 * Adjusts the volume of the loading screen.
	 * @param InVolume Incoming new loading screen volume.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Adjust Volume On Loading Screen"), Category = "Titan Zero|Audio Manager|Common")
	void AdjustVolumeOnLoadingScreen(float InVolume);

	/**
	 * Stops playing loading screen.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Stop Playing Loading Screen"), Category = "Titan Zero|Audio Manager|Common")
	void StopPLayingLoadingScreen();

	/**
	 * Plays a radio chatter.
	 * @param InRadioChatterName Incoming radio chatter name.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Play Radio Chatter"), Category = "Titan Zero|Audio Manager|Common")
	void PlayRadioChatter(FString InRadioChatterName);
};
