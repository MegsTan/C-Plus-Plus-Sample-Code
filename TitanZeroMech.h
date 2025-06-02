// Copyright (c) 2019 MaochiGames.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Characters/BaseMaochiGamesCharacter.h"
#include "Components/WidgetComponent.h"

#include "Mechs/TitanZeroMechDataAsset.h"
#include "Interfaces/MechSoundEffectsPlayable.h"
#include "Interfaces/Lockable.h"
#include "Interfaces/Destroyable.h"

#include "TitanZeroMech.generated.h"

class ATitanZeroPlayerState;
class ATitanZeroCockpit;
class AAudio3dManagedObject;
class AMinimapTaggedActor;
class UInputMappingContext;
class UInputAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FIsVisibleFromLocalMech, bool, bIsVisible);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLeftArmWeaponsActivated, bool, bValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRightArmWeaponsActivated, bool, bValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTorsoWeaponsActivated, bool, bValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReceivedAttackingMech, ATitanZeroMech*, InAttackingMech);

UCLASS()
class TITANZERO_API ATitanZeroMech : public ABaseMaochiGamesCharacter, public IMechSoundEffectsPlayable, public ILockable, public IDestroyable
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATitanZeroMech(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when an instance of this class is placed (in editor) or spawned.
	virtual void OnConstruction(const FTransform& Transform) override;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Called when this character has been possessed.
	virtual void PossessedBy(AController* NewController) override;

	// Called when the player state is ready. This is not for standalone.
	virtual void OnRep_PlayerState() override;

	// Called to notify about a change in controller on both the server and owning client.
	virtual void NotifyControllerChanged() override;

	// Returns the properties used for network replication, this needs to be overriden by all actor classes with native replicated properties.
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Called when character landed.	
	virtual void Landed(const FHitResult& Hit) override;

	// Is called to play a footstep sound effect.
	virtual void OnPlayFootStepSoundEffect_Implementation() override;

	// Is called if lock by local mech.
	virtual void OnLockByLocalMech_Implementation() override;

	// Is called if the local mech unlocks this mech.
	virtual void OnUnlockByLocalMech_Implementation() override;

	// Is called when this is destroyed.
	virtual void OnWhenDestroyed_Implementation() override;

	// Is called when left arm destroyed.
	virtual void OnWhenLeftArmDestroyed_Implementation() override;

	// Is called when right arm destroyed.
	virtual void OnWhenRightArmDestroyed_Implementation() override;

	// Is called when left leg destroyed.
	virtual void OnWhenLeftLegDestroyed_Implementation() override;

	// Is called when right leg destroyed.
	virtual void OnWhenRightLegDestroyed_Implementation() override;

private:
	// This is the vr origin component.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Components")
	USceneComponent* VrOriginComponent;

	// This is the vr origin offset component.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Components")
	USceneComponent* VrOriginOffsetComponent;

	// This is the player offset component.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Components")
	USceneComponent* PlayerOffsetComponent;

	// This is where the cockpit skeletal mesh is being attached.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Components")
	USceneComponent* CockpitComponent;

	// This is the component that looks at where the torso crosshair is.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Components")
	USceneComponent* CockpitTargetRotationComponent;

	// This is where the cockpit is attached to.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Components")
	USceneComponent* CockpitParentComponent;

	// We can use this for any line tracing that has nothing to do with cockpit e.g. minimap line tracing, etc.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Components")
	USceneComponent* LineTracerHeadComponent;

	// This is the networked torso crosshair location.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Components")
	USceneComponent* TorsoCrosshairComponent;

	// This is where the acquire icon component is handled.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Components")
	UWidgetComponent* MechInfoWidgetComponent;

	// The mech head indicator widget component.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Components")
	UWidgetComponent* MechHeadIndicatorWidgetComponent;

private:
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Enhanced Input|Vr|Mapping Context")
	UInputMappingContext* VrMappingContext;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Enhanced Input|Vr|Input Actions")
	UInputAction* LeftGripAction;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Enhanced Input|Vr|Input Actions")
	UInputAction* LeftThumbstickYAxisAction;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Enhanced Input|Vr|Input Actions")
	UInputAction* LeftThumbstickXAxisAction;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Enhanced Input|Vr|Input Actions")
	UInputAction* RightGripAction;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Enhanced Input|Vr|Input Actions")
	UInputAction* RightThumbstickYAxisAction;
	
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Enhanced Input|Vr|Input Actions")
	UInputAction* RightThumbstickXAxisAction;
	
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Enhanced Input|Pc|Mapping Context")
	UInputMappingContext* PcMappingContext;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Enhanced Input|Pc|Input Actions")
	UInputAction* LookRotationAction;

public:
	UPROPERTY(BlueprintAssignable, Category = "Game Framework|Character|Event Dispatchers")
	FIsVisibleFromLocalMech IsVisibleToLocalMechEventDispatcher;

	UPROPERTY(BlueprintAssignable, Category = "Game Framework|Character|Event Dispatchers")
	FOnLeftArmWeaponsActivated OnLeftArmWeaponsActivatedDispatcher;

	UPROPERTY(BlueprintAssignable, Category = "Game Framework|Character|Event Dispatchers")
	FOnRightArmWeaponsActivated OnRightArmWeaponsActivatedDispatcher;

	UPROPERTY(BlueprintAssignable, Category = "Game Framework|Character|Event Dispatchers")
	FOnTorsoWeaponsActivated OnTorsoWeaponsActivatedDispatcher;

	UPROPERTY(BlueprintAssignable, Category = "Game Framework|Character|Event Dispatchers")
	FOnReceivedAttackingMech OnReceivedAttackingMech;

public:
	// Defines if the mech is turning.
	UPROPERTY(ReplicatedUsing = OnRep_bIsTurning, BlueprintReadOnly, Category = "Game Framework|Character|Replicated")
	bool bIsTurning;

	// Determines if this mech is overheating or not.
	UPROPERTY(ReplicatedUsing = OnRep_bIsOverheating, BlueprintReadOnly, Category = "Game Framework|Character|Replicated")
	bool bIsOverheating;

	// If you notice this is a byte because we are only going to use 0 - 100 as an established value instead of a float which is heavier on the network.
	// We still convert this to a normalized 0-1 value as the heat sink is a normalized value. We do that in the example below.
	// Let say the value is 15, we just divide this by 100. 15/100 - 0.15.
	UPROPERTY(ReplicatedUsing = OnRep_HeatSinkValue, BlueprintReadOnly, Category = "Game Framework|Character|Replicated")
	uint8 HeatSinkValue;

public:
	// The jump jets current fuel.
	UPROPERTY(BlueprintReadOnly, Category = "Game Framework|Character|Jump Jet")
	float JumpJetCurrentFuel;

	// The jump jets current fuel in normalized value 0 - 1.
	UPROPERTY(BlueprintReadOnly, Category = "Game Framework|Character|Jump Jet")
	float JumpJetCurrentFuelNormalized;
	
	// Torso rotation for look up and look down.
	UPROPERTY(BlueprintReadOnly, Category = "Game Framework|Character|Common")
	float TorsoPitchRotation;

	// Torso rotation for look left and right.
	UPROPERTY(BlueprintReadOnly, Category = "Game Framework|Character|Common")
	float TorsoYawRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Game Framework|Character|Common")
	bool bHasJumpJet;

	// How far this mech is visible to other mechs.
	UPROPERTY(BlueprintReadOnly, Category = "Game Framework|Character|Radar")
	float MaxVisibilityDistance;

	// How much angle can this mech can view. For Autonomous proxy.
	UPROPERTY(BlueprintReadOnly, Category = "Game Framework|Character|Radar")
	float MaxViewAngle;

	// How long does it take before this icon disappears from the local mechs minimap/radar.
	UPROPERTY(BlueprintReadOnly, Category = "Game Framework|Character|Radar")
	float MaxDelayBeforeIconDisappears;

	// Mech info minimum scale.
	UPROPERTY(BlueprintReadOnly, Category = "Game Framework|Character|Radar")
	float MechInfoMinimumScale;

	// Mech info maximum scale.
	UPROPERTY(BlueprintReadOnly, Category = "Game Framework|Character|Radar")
	float MechInfoMaximumScale;

	// Mech head indicator minimum scale.
	UPROPERTY(BlueprintReadOnly, Category = "Game Framework|Character|Radar")
	float MechHeadIndicatorMinimumScale;

	// Mech head indicator maximum scale.
	UPROPERTY(BlueprintReadOnly, Category = "Game Framework|Character|Radar")
	float MechHeadIndicatorMaximumScale;

	// This mechs cockpit reference.
	UPROPERTY(BlueprintReadOnly, Category = "Game Framework|Character|Common")
	ATitanZeroCockpit* CockpitReference;

protected:
	// This is only used for local player.
	UPROPERTY(BlueprintReadOnly, Category = "Game Framework|Character|Common")
	ATitanZeroMech* CurrentLockedTargetMech;

	UPROPERTY(BlueprintReadOnly, Category = "Game Framework|Character|Common")
	FVector MechExplosionOffset;

	UPROPERTY(BlueprintReadOnly, Category = "Game Framework|Character|Common")
	FVector MechLeftArmExplosionOffset;
	
	UPROPERTY(BlueprintReadOnly, Category = "Game Framework|Character|Common")
	FVector MechRightArmExplosionOffset;
	
	UPROPERTY(BlueprintReadOnly, Category = "Game Framework|Character|Common")
	FVector MechLeftLegExplosionOffset;
	
	UPROPERTY(BlueprintReadOnly, Category = "Game Framework|Character|Common")
	FVector MechRightLegExplosionOffset;

	UPROPERTY(BlueprintReadOnly, Category = "Game Framework|Character|Common")
	FVector CockpitLeftArmExplosionOffset;

	UPROPERTY(BlueprintReadOnly, Category = "Game Framework|Character|Common")
	FVector CockpitRightArmExplosionOffset;
	
	UPROPERTY(BlueprintReadOnly, Category = "Game Framework|Character|Common")
	FVector CockpitLeftLegExplosionOffset;
	
	UPROPERTY(BlueprintReadOnly, Category = "Game Framework|Character|Common")
	FVector CockpitRightLegExplosionOffset;

	UPROPERTY(BlueprintReadOnly, Category = "Game Framework|Character|Common")
	int32 LeftLegMaterialIndex;

	UPROPERTY(BlueprintReadOnly, Category = "Game Framework|Character|Common")
	int32 RightLegMaterialIndex;

public:
	/**
	 * Returns the list of target cockpit locations.
	 * @return List of vectors as list of target cockpit locations.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Cockpit Target Locations"), Category = "Game Framework|Common")
	TArray<FVector> GetCockpitTargetLocations();

	/**
	 * Returns the left arm target location.
	 * @return Vector of left arm target location.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Cockpit Left Arm Target Location"), Category = "Game Framework|Common")
	FVector GetCockpitLeftArmTargetLocation();

	/**
	 * Returns the right arm target location.
	 * @return Vector of right arm target location.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Cockpit Right Arm Target Location"), Category = "Game Framework|Common")
	FVector GetCockpitRightArmTargetLocation();

protected:
	/**
	 * Called to notify about a change in controller on both the server and owning client.
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Notify Controller Changed"), Category = "Game Framework|Events")
	void OnNotifyControllerChanged();	

protected:
	// Defines if jump jet is activated.
	UPROPERTY(ReplicatedUsing = OnRep_bIsJumpJetActivated, BlueprintReadOnly, Category = "Game Framework|Character|Replicated")
	bool bIsJumpJetActivated;
	
	// Defines if the left arm weapon has been activated.
	UPROPERTY(ReplicatedUsing = OnRep_LeftArmWeaponActivated, BlueprintReadOnly, Category = "Game Framework|Character|Replicated")
	bool LeftArmWeaponActivated;

	// Defines if the right arm weapon has been activated.
	UPROPERTY(ReplicatedUsing = OnRep_RightArmWeaponActivated, BlueprintReadOnly, Category = "Game Framework|Character|Replicated")
	bool RightArmWeaponActivated;

	// Defines if the torso weapon has been activated.
	UPROPERTY(ReplicatedUsing = OnRep_TorsoWeaponActivated, BlueprintReadOnly, Category = "Game Framework|Character|Replicated")
	bool TorsoWeaponActivated;

private:
	// The heat sink hot meter counter, this is used to determine when to final shutdown.
	UPROPERTY(ReplicatedUsing = OnRep_HeatSinkVeryHotMeterCounter)
	uint8 HeatSinkVeryHotMeterCounter;

	// Defines the mech state. 0 = Not Ready, 1 = Ready.
	UPROPERTY(ReplicatedUsing = OnRep_MechState)
	uint8 MechState;

private:
	// Defines if this frame is using stereo rendering (VR).
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Common")
	bool bIsUsingStereoRendering;
	
	// List of small mechs data assets.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "MaochiGames|Mech Settings|Small")
	TArray<UTitanZeroMechDataAsset*> SmallMechDataAssets;

	// List of assault mechs data assets.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "MaochiGames|Mech Settings|Assault")
	TArray<UTitanZeroMechDataAsset*> AssaultMechDataAssets;

	// List of sentinel mechs data assets.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "MaochiGames|Mech Settings|Sentinel")
	TArray<UTitanZeroMechDataAsset*> SentinelMechDataAssets;
	
	// Defines whether player is holder virtual left controller.
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Common")
	bool bIsPlayerHoldingLeftController;

	// Defines whether player is holder virtual right controller.
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Common")
	bool bIsPlayerHoldingRightController;

	// This is just to define if the player is gripping the right controller.
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Common")
	bool bIsPlayerGrippingRightController;

private:
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Common")
	float ForwardSpeed;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Common")
	float BackwardSpeed;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Common")
	float DamageForwardSpeed;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Common")
	float DamageBackwardSpeed;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Common")
	float TurnSpeed;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Common")
	float TorsoRotationSpeed;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Jump Jets")
	float JumpJetLiftDelay;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Jump Jets")
	float JumpJetMaxFuel;
	
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Jump Jets")
	bool bIsConsumingJumpJet;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Audio")
	AAudio3dManagedObject* MechFootStepsAudio;
	
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Audio")
	AAudio3dManagedObject* MechJumpJetAudio;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Audio")
	TSubclassOf<AAudio3dManagedObject> CockpitFootStepsAudio;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Audio")
	TSubclassOf<AAudio3dManagedObject> CockpitEngineAudio;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Audio")
	TSubclassOf<AAudio3dManagedObject> CockpitRotationAudio;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Audio")
	TSubclassOf<AAudio3dManagedObject> CockpitJumpJetAudio;
	
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Radar")
	TSubclassOf<AMinimapTaggedActor> MinimapTaggedActor;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Radar")
	UMaterialInstance* DestroyedMaterial;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Common")
	bool bIsControllerDisabled;
	
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Weapons")
	UTitanZeroWeaponComponent* RightArmWeaponComponent;
	
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Weapons")
	UTitanZeroWeaponComponent* LeftArmWeaponComponent;
	
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Game Framework|Character|Weapons")
	UTitanZeroWeaponComponent* TorsoWeaponComponent;
	
private:
	UPROPERTY()
	ERotationDirection RotationDirection;

	UPROPERTY()
	float MechVerticalVelocity;
	
	UPROPERTY()
	float MechVerticalVelocityTimeElapsed;
	
	UPROPERTY()
	bool bIsJumpJetInProgress;

	UPROPERTY()
	FTimerHandle JumpJetLiftDelayTimerHandle;

	UPROPERTY()
	FTimerHandle JumpJetLoopTimerHandle;

	UPROPERTY()
	FTimerHandle JumpJetCoolDownLoopTimerHandle;

	UPROPERTY()
	float MechJumpZVelocity;

	UPROPERTY()
	float MechAirControl;

	UPROPERTY()
	float MechBrakingDecelerationFalling;

	UPROPERTY()
	float MechDefaultJumpZVelocity;

	UPROPERTY()
	float MechDefaultAirControl;

	UPROPERTY()
	float MechDefaultBrakingDecelerationFalling;

	UPROPERTY()
	float PreviousSpeed;

	UPROPERTY()
	bool bIsMechOverheatCoolingDown;

	UPROPERTY()
	bool bHasSpawnedCockpitOnStandAlone;

	UPROPERTY()
	TSubclassOf<ATitanZeroMechPart> TargetMechCockpit;

	UPROPERTY()
	FVector CockpitLocation;

	UPROPERTY()
	FRotator CockpitRotation;

	UPROPERTY()
	FVector CockpitMeshScale;

	UPROPERTY()
	FVector CockpitDebugMeshScale;

	UPROPERTY()
	FVector CockpitDebugLocation;
	
	UPROPERTY()
	FRotator CockpitDebugRotation;
	
	UPROPERTY()
	float TorsoRotationPitchMinimum;

	UPROPERTY()
	float TorsoRotationPitchMaximum;

	UPROPERTY()
	float TorsoRotationYawMinimum;

	UPROPERTY()
	float TorsoRotationYawMaximum;

	UPROPERTY()
	FTimerHandle HeatSinkBufferingBeforeCoolingDown;
	
	UPROPERTY()
	FTimerHandle HeatSinkCoolingDownTimerHandle;
	
	UPROPERTY()
	bool bIsImmuneFromDamage;

	UPROPERTY()
	FTimerHandle ControllerDisabledTimerHandle;

	UPROPERTY()
	float HeatSinkBufferTimeBeforeCoolDown;

	UPROPERTY()
	FTimerHandle WaitForLocalMechTimerHandle;

	UPROPERTY()
	ATitanZeroPlayerState* RemoteTitanZeroPlayerState;

	UPROPERTY()
	int32 MaxTorsoHealth;
	
	UPROPERTY()
	int32 MaxLeftArmHealth;
	
	UPROPERTY()
	int32 MaxRightArmHealth;
	
	UPROPERTY()
	int32 MaxLeftLegHealth;

	UPROPERTY()
	int32 MaxRightLegHealth;

	UPROPERTY()
	TSubclassOf<UCommonHud> MechInfoWidgetReference;

	UPROPERTY()
	FVector MechInfoInitialScale;

	UPROPERTY()
	TSubclassOf<UCommonHud> MechHeadIndicatorReference;

	UPROPERTY()
	FVector MechHeadIndicatorInitialScale;

	UPROPERTY()
	bool bCanAcquireMech;

	UPROPERTY()
	float AcquireHoldTime;

	UPROPERTY()
	float LastTorsoRotationYaw;

	UPROPERTY()
	float LastTorsoRotationPitch;
	
	UPROPERTY()
	FTimerHandle UnlockMechTimerHandle;
	
	const int32 HEAT_SINK_MIN_METER_COUNTER = 0;
	const int32 HEAT_SINK_MAX_METER_COUNTER = 4;
	const uint8 HEAT_SINK_MIN_VALUE = 0;
	const uint8 HEAT_SINK_MAX_VALUE = 100;

	UPROPERTY()
	bool bIsOrAreLegsDamaged;
	
	UPROPERTY()
	FTimerHandle AttackingMechTimerHandle;
	
public:
	/**
	 * Initializes the mech on the server side.
	 * @param InMechFriendlyName Incoming mech friendly name.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, meta = (DisplayName = "(SERVER) Initialize Mech"), Category = "Game Framework|Character|Replicated")
	void Server_InitializeMech(const FName& InMechFriendlyName);

	/**
	 * Adds a heat sink value.
	 * @param InValue Adds a new heat sink value.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, meta = (DisplayName = "(SERVER) Add Heatsink"), Category = "Game Framework|Character|Replicated")
	void Server_AddHeatSink(const uint8& InValue);

	/**
	 * Is called to let this mech takes health damage.
	 * 
	 * Body Parts Code:
	 * 0 = Torso
	 * 1 = Left Arm
	 * 2 = Right Arm
	 * 3 = Left Leg
	 * 4 = Right Leg
	 * @param InPartToDamage Incoming part to damage, 0 = Torso, 1 = Left Arm, 2 = Right Arm, 3 = Left Leg, 4 = Right Leg
	 * @param InDamageValue Incoming damage value.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, meta = (DisplayName = "(Server) Take Damage"), Category = "Game Framework|Character|Replcated")
	void Server_TakeDamage(const uint8& InPartToDamage, const int32& InDamageValue);

	/**
	 * Activate/Deactivate left arm weapon(s).
	 * @param bIsActivationValue Bool as new activation value.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, meta = (DisplayName = "(Server) Set Left Arm Weapon(s) Activation"), Category = "Game Framework|Character|Replcated")
	void Server_SetLeftArmWeaponsActivation(bool bIsActivationValue);

	/**
	 * Activate/Deactivate right arm weapon(s).
	 * @param bIsActivationValue Bool as new activation value.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, meta = (DisplayName = "(Server) Set Right Arm Weapon(s) Activation"), Category = "Game Framework|Character|Replcated")
	void Server_SetRightArmWeaponsActivation(bool bIsActivationValue);

	/**
	 * Activate/Deactivate torso weapon(s).
	 * @param bIsActivationValue Bool as new activation value.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, meta = (DisplayName = "(Server) Set Torso Weapon(s) Activation"), Category = "Game Framework|Character|Replcated")
	void Server_SetTorsoWeaponsActivation(bool bIsActivationValue);

	/**
	 * Updates the network torso crosshair location.
	 * @param InLocation Incoming world location.
	 */
	UFUNCTION(Server, Unreliable, BlueprintCallable, meta = (DisplayName = "(Server) Update Torso Crosshair Location"), Category = "Game Framework|Character|Replcated")
	void Server_UpdateTorsoCrosshairLocation(const FQuantizedVector& InLocation);

public:
	/**
	 * Updates if mech is turning.
	 * @param bValue Incoming new value.
	 */
	UFUNCTION(Server, Reliable)
	void Server_MechIsTurning(const bool bValue);

	/**
	 * Updates the network torso crosshair location.
	 * @param InLocation Incoming world location.
	 */
	UFUNCTION(NetMulticast, Unreliable)
	void NetMulticast_UpdateTorsoCrosshairLocation(const FQuantizedVector& InLocation);

public:
	/**
	 * Returns the cockpit parent component.
	 * @return USceneComponent as cockpit parent component.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "Get Cockpit Parent Component"), Category = "Game Framework|Character|Common")
	USceneComponent* GetCockpitParentComponent();

	/**
	 * Returns the mech speed in normalized value between 0 - 1.
	 * @return Float as mechs speed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "Get Mech Speed (Normalized)"), Category = "Game Framework|Character|Common")
	float GetMechSpeedClampNormalized();

	/**
	 * Returns the mech speed in kilometers.
	 * @return Float as mech speed in kilometers.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "Get Mech Speed (Kilometers)"), Category = "Game Framework|Character|Common")
	float GetMechSpeedInKilometers();

	/**
	 * Returns whether the left arm is still online.
	 * @return Boolean if true or false.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "Check If Left Arm Still Online"), Category = "Game Framework|Character|Common")
	bool CheckIfLeftArmStillOnline();

	/**
	 * Returns whether the right arm is still online.
	 * @return Boolean if true or false.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "Check If Right Arm Still Online"), Category = "Game Framework|Character|Common")
	bool CheckIfRightArmStillOnline();
	
	/**
	 * Set a new attacking mech.
	 * @param InAttackingMech Incoming attacking mech.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Attacking Mech"), Category = "Game Framework|Character|Common")
	void SetAttackingMech(ATitanZeroMech* InAttackingMech);
	
public:
	/**
	 * Tracks if this remote mech is visible to the local mech.
	 */
	UFUNCTION()
	bool IsRemoteMechVisibleToLocalMech();

protected:
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Look Rotation"), Category = "Game Framework|Events")
	void OnLookRotation(const FVector2D InputValue);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Left Grip"), Category = "Game Framework|Events")
	void OnLeftGrip(const float Value);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Left Thumbstick Y Axis"), Category = "Game Framework|Events")
	void OnLeftThumbstickYAxis(const float Value);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Left Thumbstick X Axis"), Category = "Game Framework|Events")
	void OnLeftThumbstickXAxis(const float Value);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Right Grip"), Category = "Game Framework|Events")
	void OnRightGrip(const float Value);
	
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Right Thumbstick Y Axis"), Category = "Game Framework|Events")
	void OnRightThumbstickYAxis(const float Value);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Right Thumbstick X Axis"), Category = "Game Framework|Events")
	void OnRightThumbstickXAxis(const float Value);

protected:
	/**
	 * Is called when the mech is moving forward and turning left or right. Autonomous only.
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Leaning While Moving Forward"), Category = "Game Framework|Events")
	void OnLeaningWhileMovingForward(const ERotationDirection& Value);

	/**
	 * Is called when the jump started, this is immediately called when jump is pressed.
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Jump Jet Start"), Category = "Game Framework|Events")
	void OnJumpJetStart();

	/**
	 * Is called when the jump jet has stopped, this is immediately called when jump jet ran out of fuel.
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Jump Jet Stop"), Category = "Game Framework|Events")
	void OnJumpJetStop();

	/**
	 * Is called when the mech speed updated.
	 * @param InValue Incoming value.
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Speed Updated"), Category = "Game Framework|Events")
	void OnSpeedUpdated(const float InValue);

	/**
	 * Is called to update the torso rotation.
	 * @param InValue Incoming value.
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Torso Rotation Updated"), Category = "Game Framework|Events")
	void OnTorsoRotationUpdate(const float InValue);

	/**
	 * Is called when the mech starts overheating in blueprint side.
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Start Overheating"), Category = "Game Framework|Events")
	void OnStartOverheating();

	/**
	 * Is called when the mech stops overheating in blueprint side.
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Stop Overheating"), Category = "Game Framework|Events")
	void OnStopOverheating();

	/**
	 * Is called if the mech is ready. This is called on authority, autonomous, simulated.
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Mech Is Ready"), Category = "Game Framework|Events")
	void OnMechIsReady();

	/**
	 * Is called when jump jet toggles.
	 * @param bInValue Bool as true or false.
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Is Jump Jet Activated"), Category = "Game Framework|Events")
	void OnIsJumpJetActivated(bool bInValue);

	/**
	 * Is called when a mech is locked by this mech.
	 * @param InLockedMech Incoming locked mech.
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Locked A Mech"), Category = "Game Framework|Events")
	void OnLocked_A_Mech(const ATitanZeroMech* InLockedMech);

	/**
	 * Is called when this mech unlocks a mech.
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Unlocked A Mech"), Category = "Game Framework|Events")
	void OnUnlocked_A_Mech();

protected:
	/**
	 * Tries to acquire a mech. It returns false if either is returned bCanAcquireMech = false or bIsControllerDisabled is true or bIsInValidDistance = false or the mech has no lockable interface.
	 * @return Boolean as either of the following, bCanAcquireMech = false or bIsControllerDisabled is true  or bIsInValidDistance = false or the mech has no lockable interface will return false.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Acquire A Mech"), Category = "Game Framework|Common")
	bool Acquire_A_Mech();
	
	/**
	 * Updates the torso rotation.
	 * @param InHeadCrosshairComponent Incoming head crosshair component.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Update Torso Rotation"), Category = "Game Framework|Common")
	void UpdateTorsoRotation(USceneComponent* InHeadCrosshairComponent);

	/**
	 * Is called to reset the mech.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Reset Mech"), Category = "Game Framework|Common")
	void ResetMech();

	/**
	 * Setup a minimap tagged actor.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Setup Minimap Tagged Actor"), Category = "Game Framework|Common")
	void SetupMinimapTaggedActor(float BelowMechDistance = 1000.f);

	/**
	 * Setup the acquire icon. NOTE: This is for none local player. This also returns the widget.
	 * @return UserWidget as widget.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Setup Acquire Icon"), Category = "Game Framework|Common")
	UUserWidget* SetupAcquireIcon();

	/**
	 * Setup the head icon. NOTE: This is for none local player. This also returns the widget.
	 * @return UserWidget as widget.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Setup Head Icon"), Category = "Game Framework|Common")
	UUserWidget* SetupHeadIcon();

	/**
	 * Disables the mech.
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Disable Mech"), Category = "Game Framework|Common")
	void DisableMech();

protected:
	/**
	 * Toggles the jump jet.
	 * @param bIsInValue Bool as true or false.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, meta = (DisplayName = "(Server) Toggle Jump Jet"), Category = "Game Framework|Character|Replcated")
	void Server_ToggleJumpJet(bool bIsInValue);

private:
	/**
	 * Initializes the mech on clients side.
	 * @param InMechFriendlyName Incoming mech friendly name.
	 */
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_InitializeMech(const FName& InMechFriendlyName);
	
	/**
	 * Updates the torso rotation on the server.
	 * @param InRotation Incoming torso rotation.
	 */
	UFUNCTION(Server, Unreliable)
	void Server_UpdateTorsoRotation(const FQuantizedVector& InRotation);

	/**
	 * Updates the torso rotation on clients.
	 * @param InRotation Incoming torso rotation.
	 */
	UFUNCTION(NetMulticast, Unreliable)
	void NetMulticast_UpdateTorsoRotation(const FQuantizedVector& InRotation);

	/**
	 * Is called when the mech stops overheating.
	 */
	UFUNCTION(Server, Reliable)
	void Server_StopOverheating();

	/**
	 * Is called to let this mech know this is ready.
	 */
	UFUNCTION(Server, Reliable)
	void Server_SetMechIsReady();

	/**
	 * Is called to initialize passive abilities such as health, weapons.
	 */
	UFUNCTION(Server, Reliable)
	void Server_InitializePassiveAbilities();

private:
	/**
	 * Configure mech.
	 * @param InMaochiGamesNetMode Incoming net mode.
	 * @param InMechData Incoming mech data.
	 */
	UFUNCTION()
	void ConfigureMech(const EMaochiGamesNetMode InMaochiGamesNetMode, UTitanZeroMechDataAsset* const& InMechData);

private:
	UFUNCTION()
	void OnLeftGripTriggered(const FInputActionValue& Value);

	UFUNCTION()
	void OnLeftGripCompleted(const FInputActionValue& Value);

	UFUNCTION()
	void OnLeftThumbstickYAxisTriggered(const FInputActionValue& Value);

	UFUNCTION()
	void OnLeftThumbstickYAxisCompleted(const FInputActionValue& Value);
	
	UFUNCTION()
	void OnLeftThumbstickXAxisTriggered(const FInputActionValue& Value);

	UFUNCTION()
	void OnLeftThumbstickXAxisCompleted(const FInputActionValue& Value);

	UFUNCTION()
	void OnRightGripTriggered(const FInputActionValue& Value);

	UFUNCTION()
	void OnRightGripCompleted(const FInputActionValue& Value);
	
	UFUNCTION()
	void OnRightThumbstickYAxisTriggered(const FInputActionValue& Value);

	UFUNCTION()
	void OnRightThumbstickYAxisCompleted(const FInputActionValue& Value);
	
	UFUNCTION()
	void OnRightThumbstickXAxisTriggered(const FInputActionValue& Value);

	UFUNCTION()
	void OnRightThumbstickXAxisCompleted(const FInputActionValue& Value);

	UFUNCTION()
	void OnLookRotationTriggered(const FInputActionValue& Value);
	
	UFUNCTION()
	void OnLookRotationCompleted(const FInputActionValue& Value);

	UFUNCTION()
	void MechVerticalAxis(float Value);

	UFUNCTION()
	void MechTurnAxis(float Value);

	UFUNCTION()
	void MechJumpPressed();

	UFUNCTION()
	void MechJumpReleased();

	UFUNCTION()
	void OnToggleLockLookForVr();

	UFUNCTION()
	void OnToggleLockLookForPc();

private:
	UFUNCTION()
	float GetJumpJetFuelNormalizedValue();

	UFUNCTION()
	void OnJumpJetLiftDelayCompleted();

	UFUNCTION()
	void OnJumpJetLoopTimerHandle();

	UFUNCTION()
	void OnJumpJetCoolDownLoopTimerHandle();
	
	UFUNCTION()
	void OnHeatSinkBufferingBeforeCoolingDownTimerHandle();
	
	UFUNCTION()
	void OnHeatSinkCoolingDownTimerHandle();

	UFUNCTION()
	void OnControllerDisabledTimerHandle();

	UFUNCTION()
	void OnWaitForLocalMechTimerHandle();

	UFUNCTION()
	void FindLocalMechAndDetermineIfOpponentOrATeammate();

	UFUNCTION()
	void OnUnlockMechTimerHandle();

	UFUNCTION()
	void OnAttackingMechTimerHandleComplete();

private:
	UFUNCTION()
	void OnRep_bIsTurning();

	UFUNCTION()
	void OnRep_bIsOverheating();

	UFUNCTION()
	void OnRep_HeatSinkValue();

	UFUNCTION()
	void OnRep_HeatSinkVeryHotMeterCounter();

	UFUNCTION()
	void OnRep_MechState();

	UFUNCTION()
	void OnRep_bIsJumpJetActivated();

	UFUNCTION()
	void OnRep_LeftArmWeaponActivated();

	UFUNCTION()
	void OnRep_RightArmWeaponActivated();

	UFUNCTION()
	void OnRep_TorsoWeaponActivated();
};