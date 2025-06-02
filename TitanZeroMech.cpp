// Copyright (c) 2019 MaochiGames.

/********************************************************************************************************************************************/
/*														JUMP JET LOGIC																		*/
/* SETUP:																																	*/
/* - In editor, B_Mech blueprint class, click the root component which is B_Mech (self)														*/
/* - Click the details panel, type in "Jump", under Character category																		*/
/* - Jump Max Hold Time, set it to 100																										*/
/* - Jump Max Count, set it to 100																											*/
/* In that sense you allow the jump input to feel like unlimited since jump jet has limit and cool down time								*/
/*																																			*/
/* C++ SETUP:																																*/
/* - In C++ setup, I already set the jump max hold time to 100.f, and jump max count to 100, so it does not matter in blueprint anymore     */
/*																																			*/
/*																																			*/
/*																																			*/
/********************************************************************************************************************************************/

/********************************************************************************************************************************************/
/*														MODIFIABLE PROPERTIES																*/
/*																																			*/
/*	- The idea is this is part of the players data that will be stored in game instance and once player joins the multiplayer,				*/
/*	  player controller will pull this from the game instance then to some has to go to the server and some will just be kept				*/
/*	  on the player side.																													*/
/*																																			*/
/*	BACK-END -> GAME INSTANCE -> PLAYER CONTROLLER (SOME WILL STAY HERE) -> SERVER															*/
/*																																			*/
/********************************************************************************************************************************************/

#include "Mechs/TitanZeroMech.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "StereoRendering.h"
#include "IXRTrackingSystem.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Engine/StreamableManager.h"

#include "PlayerControllers/TitanZeroPlayerController.h"
#include "PlayerStates/TitanZeroPlayerState.h"
#include "GameStates/TitanZeroGameState.h"
#include "Bots/TitanZeroBotController.h"
#include "Core/TitanZero.h"
#include "Engine/AssetManager.h"
#include "Mechs/TitanZeroCockpit.h"
#include "Minimap/MinimapTaggedActor.h"
#include "HelperUtilities/TitanZeroHelperUtility.h"
#include "Settings/MaochiGamesSettings.h"

// Sets default values
ATitanZeroMech::ATitanZeroMech(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// We set this character class jump max hold time and jump max count.
	JumpMaxHoldTime = 100.f;
	JumpMaxCount = 100;

	VrOriginComponent = CreateDefaultSubobject<USceneComponent>(TEXT("VR Origin Component"));
	VrOriginComponent->SetupAttachment(RootComponent);
	
	VrOriginOffsetComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Origin Offset Component"));
	VrOriginOffsetComponent->SetupAttachment(VrOriginComponent);

	PlayerOffsetComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Player Offset Component"));
	PlayerOffsetComponent->SetupAttachment(VrOriginOffsetComponent);

	CockpitComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Cockpit Component"));
	CockpitComponent->SetupAttachment(VrOriginOffsetComponent);

	CockpitTargetRotationComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Cockpit Target Rotation Component"));
	CockpitTargetRotationComponent->SetRelativeLocation(FVector::ZeroVector);
	CockpitTargetRotationComponent->SetRelativeRotation(FRotator::ZeroRotator);
	CockpitTargetRotationComponent->SetRelativeScale3D(FVector::OneVector);
	CockpitTargetRotationComponent->SetupAttachment(CockpitComponent);

	CockpitParentComponent	= CreateDefaultSubobject<USceneComponent>(TEXT("Cockpit Parent Component"));
	CockpitParentComponent->SetRelativeLocation(FVector::ZeroVector);
	CockpitParentComponent->SetRelativeRotation(FRotator::ZeroRotator);
	CockpitParentComponent->SetRelativeScale3D(FVector::OneVector);
	CockpitParentComponent->SetupAttachment(CockpitComponent);

	LineTracerHeadComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Line Tracer Head Component"));
	LineTracerHeadComponent->SetRelativeLocation(FVector::ZeroVector);
	LineTracerHeadComponent->SetRelativeRotation(FRotator::ZeroRotator);
	LineTracerHeadComponent->SetRelativeScale3D(FVector::OneVector);
	LineTracerHeadComponent->SetupAttachment(RootComponent);

	TorsoCrosshairComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Torso Crosshair Component"));
	TorsoCrosshairComponent->SetRelativeLocation(FVector::ZeroVector);
	TorsoCrosshairComponent->SetRelativeRotation(FRotator::ZeroRotator);
	TorsoCrosshairComponent->SetRelativeScale3D(FVector::OneVector);
	TorsoCrosshairComponent->SetupAttachment(RootComponent);

	MechInfoWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Mech Info Widget Component"));
	MechInfoWidgetComponent->SetRelativeLocation(FVector::ZeroVector);
	MechInfoWidgetComponent->SetRelativeRotation(FRotator::ZeroRotator);
	MechInfoWidgetComponent->SetRelativeScale3D(FVector::OneVector);
	MechInfoWidgetComponent->SetupAttachment(RootComponent);
	MechInfoWidgetComponent->SetCollisionProfileName(TEXT("NoCollision"));

	MechHeadIndicatorWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Mech Head Indicator Widget Component"));
	MechHeadIndicatorWidgetComponent->SetRelativeLocation(FVector::ZeroVector);
	MechHeadIndicatorWidgetComponent->SetRelativeRotation(FRotator::ZeroRotator);
	MechHeadIndicatorWidgetComponent->SetRelativeScale3D(FVector::OneVector);
	MechHeadIndicatorWidgetComponent->SetupAttachment(RootComponent);
	MechHeadIndicatorWidgetComponent->SetCollisionProfileName(TEXT("NoCollision"));
	
	VrMappingContext = nullptr;
	LeftGripAction = nullptr;
	LeftThumbstickYAxisAction = nullptr;
	LeftThumbstickXAxisAction = nullptr;
	
	RightGripAction = nullptr;
	RightThumbstickYAxisAction = nullptr;
	RightThumbstickXAxisAction = nullptr;
	
	PcMappingContext = nullptr;
	LookRotationAction = nullptr;

	bIsPlayerHoldingLeftController = false;
	bIsPlayerHoldingRightController = false;

	bIsUsingStereoRendering = false;

	bHasSpawnedCockpitOnStandAlone = false;
	TargetMechCockpit = nullptr;
	CockpitLocation = FVector::ZeroVector;
	CockpitRotation = FRotator::ZeroRotator;
	CockpitMeshScale = FVector::OneVector;
	CockpitDebugMeshScale = FVector::OneVector;
	CockpitDebugLocation = FVector::ZeroVector;
	CockpitDebugRotation = FRotator::ZeroRotator;

	ForwardSpeed = 600.f;
	BackwardSpeed = 200.f;
	DamageForwardSpeed = 300.f;
	DamageBackwardSpeed = 100.f;

	TurnSpeed = 1.f;
	TorsoRotationSpeed = 5.f;

	TorsoRotationPitchMinimum = 0.f;
	TorsoRotationPitchMaximum = 0.f;
	TorsoRotationYawMinimum = 0.f;
	TorsoRotationYawMaximum = 0.f;

	TorsoPitchRotation = 0.f;
	TorsoYawRotation = 0.f;
	
	bIsTurning = false;
	RotationDirection = ERotationDirection::None;

	MechVerticalVelocity = 0.f;
	MechVerticalVelocityTimeElapsed = 0.f;

	bHasJumpJet = false;
	JumpJetLiftDelay = 2.5f;
	JumpJetMaxFuel = 14.5f;
	JumpJetCurrentFuel = 0.f;
	bIsJumpJetInProgress = false;
	bIsConsumingJumpJet = false;
	JumpJetLiftDelayTimerHandle = {};
	JumpJetLoopTimerHandle = {};
	JumpJetCoolDownLoopTimerHandle = {};

	MechJumpZVelocity = 420.f;
	MechAirControl = 0.05f;
	MechBrakingDecelerationFalling = 0.f;

	MechDefaultJumpZVelocity = GetCharacterMovement()->JumpZVelocity;
	MechDefaultAirControl = GetCharacterMovement()->AirControl;
	MechDefaultBrakingDecelerationFalling = GetCharacterMovement()->BrakingDecelerationFalling;

	PreviousSpeed = 0.f;

	bIsOverheating = false;
	HeatSinkBufferTimeBeforeCoolDown = 0.f;
	bIsMechOverheatCoolingDown = false;

	bIsJumpJetActivated = false;

	HeatSinkValue = 0.f;
	bIsImmuneFromDamage = false;
	HeatSinkBufferingBeforeCoolingDown = {};
	HeatSinkCoolingDownTimerHandle = {};
	HeatSinkVeryHotMeterCounter = 0;

	bIsControllerDisabled = false;
	MechState = 0;

	MaxVisibilityDistance = 0.f;
	MaxViewAngle = 35.f;
	MaxDelayBeforeIconDisappears = 0.1f;

	RemoteTitanZeroPlayerState = nullptr;

	MaxTorsoHealth = 1000;
	MaxLeftArmHealth = 1000;
	MaxRightArmHealth = 1000;
	MaxLeftLegHealth = 1000;
	MaxRightLegHealth = 1000;

	MechFootStepsAudio = nullptr;
	MechJumpJetAudio = nullptr;
	CockpitFootStepsAudio = nullptr;
	CockpitEngineAudio = nullptr;
	CockpitRotationAudio = nullptr;
	CockpitJumpJetAudio = nullptr;

	MinimapTaggedActor = nullptr;
	
	MechInfoWidgetReference = nullptr;
	MechInfoInitialScale = FVector::OneVector;
	MechInfoMinimumScale = 0.f;
	MechInfoMaximumScale = 0.f;

	MechHeadIndicatorReference = nullptr;
	MechHeadIndicatorInitialScale = FVector::OneVector;
	MechHeadIndicatorMinimumScale = 0.f;
	MechHeadIndicatorMaximumScale = 0.f;

	bCanAcquireMech = false;
	AcquireHoldTime = 0.f;
	CurrentLockedTargetMech = nullptr;

	LastTorsoRotationPitch = 0.f;
	LastTorsoRotationYaw = 0.f;

	UnlockMechTimerHandle = {};

	DestroyedMaterial = nullptr;

	MechExplosionOffset = FVector::ZeroVector;
	MechLeftArmExplosionOffset = FVector::ZeroVector;
	MechRightArmExplosionOffset = FVector::ZeroVector;
	MechLeftLegExplosionOffset = FVector::ZeroVector;
	MechRightLegExplosionOffset = FVector::ZeroVector;

	CockpitLeftArmExplosionOffset = FVector::ZeroVector;
	CockpitRightArmExplosionOffset = FVector::ZeroVector;
	CockpitLeftLegExplosionOffset = FVector::ZeroVector;
	CockpitRightLegExplosionOffset = FVector::ZeroVector;

	LeftLegMaterialIndex = 0;
	RightLegMaterialIndex = 0;

	bIsOrAreLegsDamaged = false;

	LeftArmWeaponActivated = false;
	RightArmWeaponActivated = false;
	TorsoWeaponActivated = false;

	RightArmWeaponComponent = nullptr;
	LeftArmWeaponComponent = nullptr;
	TorsoWeaponComponent = nullptr;
}

void ATitanZeroMech::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

// Called when the game starts or when spawned
void ATitanZeroMech::BeginPlay()
{
	Super::BeginPlay();

	bIsPlayerHoldingLeftController = false;
	bIsPlayerHoldingRightController = false;
	MechVerticalVelocityTimeElapsed = 0.f;
	bIsOverheating = false;
	bIsMechOverheatCoolingDown = false;
	HeatSinkValue = 0.f;
	HeatSinkVeryHotMeterCounter = 0;
	bIsControllerDisabled = false;

	JumpJetCurrentFuelNormalized = 1.f;
	MechState = 0;

	CurrentLockedTargetMech = nullptr;

	bIsOrAreLegsDamaged = false;
	
	LeftArmWeaponActivated = false;
	RightArmWeaponActivated = false;
	TorsoWeaponActivated = false;

	MechInfoWidgetComponent->SetCollisionProfileName(TEXT("NoCollision"));
	MechHeadIndicatorWidgetComponent->SetCollisionProfileName(TEXT("NoCollision"));
}

// Called every frame
void ATitanZeroMech::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ATitanZeroMech::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	/* This is for non enhance input as it seems like enhance input does not work well with VR */

	// -------------------- Mech --------------------
	PlayerInputComponent->BindAxis("Mech Vertical Axis", this, &ATitanZeroMech::MechVerticalAxis);
	PlayerInputComponent->BindAxis("Mech Turn Axis", this, &ATitanZeroMech::MechTurnAxis);

	PlayerInputComponent->BindAction("Mech Jump", IE_Pressed, this, &ATitanZeroMech::MechJumpPressed);
	PlayerInputComponent->BindAction("Mech Jump", IE_Released, this, &ATitanZeroMech::MechJumpReleased);

	PlayerInputComponent->BindAction("Right Thumbstick Button", IE_Pressed, this, &ATitanZeroMech::OnToggleLockLookForVr);

	PlayerInputComponent->BindAction("Toggle Lock Look For Pc", IE_Pressed, this, &ATitanZeroMech::OnToggleLockLookForPc);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// -------------------- VR --------------------
		EnhancedInputComponent->BindAction(LeftGripAction, ETriggerEvent::Triggered, this, &ATitanZeroMech::OnLeftGripTriggered);
		EnhancedInputComponent->BindAction(LeftGripAction, ETriggerEvent::Completed, this, &ATitanZeroMech::OnLeftGripCompleted);

		EnhancedInputComponent->BindAction(LeftThumbstickYAxisAction, ETriggerEvent::Triggered, this, &ATitanZeroMech::OnLeftThumbstickYAxisTriggered);
		EnhancedInputComponent->BindAction(LeftThumbstickYAxisAction, ETriggerEvent::Completed, this, &ATitanZeroMech::OnLeftThumbstickYAxisCompleted);

		EnhancedInputComponent->BindAction(LeftThumbstickXAxisAction, ETriggerEvent::Triggered, this, &ATitanZeroMech::OnLeftThumbstickXAxisTriggered);
		EnhancedInputComponent->BindAction(LeftThumbstickXAxisAction, ETriggerEvent::Completed, this, &ATitanZeroMech::OnLeftThumbstickXAxisCompleted);

		EnhancedInputComponent->BindAction(RightGripAction, ETriggerEvent::Triggered, this, &ATitanZeroMech::OnRightGripTriggered);
		EnhancedInputComponent->BindAction(RightGripAction, ETriggerEvent::Completed, this, &ATitanZeroMech::OnRightGripCompleted);

		EnhancedInputComponent->BindAction(RightThumbstickYAxisAction, ETriggerEvent::Triggered, this, &ATitanZeroMech::OnRightThumbstickYAxisTriggered);
		EnhancedInputComponent->BindAction(RightThumbstickYAxisAction, ETriggerEvent::Completed, this, &ATitanZeroMech::OnRightThumbstickYAxisCompleted);

		EnhancedInputComponent->BindAction(RightThumbstickXAxisAction, ETriggerEvent::Triggered, this, &ATitanZeroMech::OnRightThumbstickXAxisTriggered);
		EnhancedInputComponent->BindAction(RightThumbstickXAxisAction, ETriggerEvent::Completed, this, &ATitanZeroMech::OnRightThumbstickXAxisCompleted);

		// -------------------- PC --------------------
		EnhancedInputComponent->BindAction(LookRotationAction, ETriggerEvent::Triggered, this, &ATitanZeroMech::OnLookRotationTriggered);
		EnhancedInputComponent->BindAction(LookRotationAction, ETriggerEvent::Completed, this, &ATitanZeroMech::OnLookRotationCompleted);
	}
}

void ATitanZeroMech::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if(const ATitanZeroBotController* TitanZeroBotController = Cast<ATitanZeroBotController>(NewController))
	{
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;

		if(GetWorld()->GetNetMode() == NM_Standalone)
		{
			// There might be a chance that we still have no local mech, we wait first.
			GetWorld()->GetTimerManager().SetTimer(WaitForLocalMechTimerHandle, this, &ATitanZeroMech::OnWaitForLocalMechTimerHandle, 0.1f, true, 0.1f);
		}
	}
	else
	{
		// Setting the input on begin play only works for multiplayer. Setting up input in single player works here.
		if(GetWorld()->GetNetMode() == NM_Standalone)
		{
			if(APlayerController* PlayerController = Cast<APlayerController>(NewController))
			{
				/*
				* Perform a console command for VR for this specific local player/client only.
				* The vr.PixelDensity cvar will under or over-sample the resolution for your Head Mounted Display.
				* More info: https://www.unrealengine.com/blog/significant-changes-coming-to-vr-resolution-settings-in-4-19.
				*/
			
				// We get the vr stereo if existing. We check if this is being used in this frame.
				IHeadMountedDisplay *VrHmd = nullptr;
				TSharedPtr<IStereoRendering, ESPMode::ThreadSafe> VrStereo = nullptr;
				if (GEngine)
				{
					// Not being used right now, just keeping this we might need this at a later time.
					VrHmd = GEngine->XRSystem->GetHMDDevice();
					// Is use to check if vr is enabled at this frame.
					VrStereo = GEngine->XRSystem->GetStereoRenderingDevice();
				}
				
				if(VrStereo->IsStereoEnabled())
				{
					bIsUsingStereoRendering = true;
					
					// We set this to local as for seated experiences.		        
					UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Local);
					UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition(0.f, EOrientPositionSelector::OrientationAndPosition);
					//UHeadMountedDisplayFunctionLibrary::SetClippingPlanes(0.01f, 999999999999999.f);
					PlayerController->ConsoleCommand("vr.PixelDensity 1.0");
			
					if(UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
					{
						InputSubsystem->AddMappingContext(VrMappingContext, 0);
					}
				}
				else
				{
					bIsUsingStereoRendering = false;
					
					if(UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
					{
						InputSubsystem->AddMappingContext(PcMappingContext, 0);
					}			    
				}
			}		
		}
	}

	// We also need to give passive abilities.
	Server_InitializePassiveAbilities();

	// Then we let the server know we are ready.
	Server_SetMechIsReady();
}

void ATitanZeroMech::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if(GetWorld()->GetNetMode() != NM_Standalone)
	{
		if(GetLocalRole() == ROLE_SimulatedProxy)
		{
			// There might be a chance that we still have no local mech, we wait first.
			GetWorld()->GetTimerManager().SetTimer(WaitForLocalMechTimerHandle, this, &ATitanZeroMech::OnWaitForLocalMechTimerHandle, 0.1f, true, 0.1f);
		}
	}
}

void ATitanZeroMech::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	if(GetWorld()->GetNetMode() == NM_Standalone)
	{
		if(APlayerController* PlayerController = Cast<APlayerController>(Controller))
		{
			/*
			* Perform a console command for VR for this specific local player/client only.
			* The vr.PixelDensity cvar will under or over-sample the resolution for your Head Mounted Display.
			* More info: https://www.unrealengine.com/blog/significant-changes-coming-to-vr-resolution-settings-in-4-19.
			*/
			if(PlayerController->IsLocalPlayerController())
			{
				bIsUsingStereoRendering = false;
				
			    // We get the vr stereo if existing. We check if this is being used in this frame.
			    IHeadMountedDisplay* VrHmd = nullptr;
			    TSharedPtr<IStereoRendering, ESPMode::ThreadSafe> VrStereo = nullptr;
			    if (GEngine)
			    {
			    	if(GEngine->XRSystem != nullptr)
			    	{
			    		if(GEngine->XRSystem->GetHMDDevice() != nullptr)
			    		{
			    			// Not being used right now, just keeping this we might need this at a later time.
			    			VrHmd = GEngine->XRSystem->GetHMDDevice();
			    			// Is use to check if vr is enabled at this frame.
			    			VrStereo = GEngine->XRSystem->GetStereoRenderingDevice();

			    			if(VrStereo->IsStereoEnabled()) bIsUsingStereoRendering = true;
			    		}
			    	}
			    }

			    if(bIsUsingStereoRendering)
			    {
			        // We set this to local as for seated experiences.		        
	                UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Local);
	                UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition(0.f, EOrientPositionSelector::OrientationAndPosition);
				    //UHeadMountedDisplayFunctionLibrary::SetClippingPlanes(0.00000001f, 999999999999999.f);
			        PlayerController->ConsoleCommand("vr.PixelDensity 1.0");
		
				    if(UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
				    {
					    InputSubsystem->AddMappingContext(VrMappingContext, 0);
				    }
			    }
			    else
			    {
				    if(UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
				    {
					    InputSubsystem->AddMappingContext(PcMappingContext, 0);
				    }	    
			    }

				// Since in standalone for some reason this is called twice, so we filter this to only happen once.
				if(!bHasSpawnedCockpitOnStandAlone)
				{
					// Spawn cockpit here.
					FActorSpawnParameters SpawnParameters;
					SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
					SpawnParameters.Owner = this;
					SpawnParameters.Instigator = this;
				
					ATitanZeroCockpit* SpawnedCockpit = GetWorld()->SpawnActor<ATitanZeroCockpit>(TargetMechCockpit, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
					SpawnedCockpit->AttachToComponent(CockpitParentComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, NAME_None);

					SpawnedCockpit->CockpitSkeletalMeshComponent->SetRelativeLocation(CockpitLocation);
					SpawnedCockpit->CockpitSkeletalMeshComponent->SetRelativeRotation(CockpitRotation);
					SpawnedCockpit->CockpitSkeletalMeshComponent->SetRelativeScale3D(CockpitMeshScale);

					if(!bIsUsingStereoRendering)
					{
						// We override the scale.
						SpawnedCockpit->CockpitSkeletalMeshComponent->SetWorldScale3D(CockpitDebugMeshScale);
						// We override the location.
						SpawnedCockpit->CockpitSkeletalMeshComponent->SetRelativeLocation(CockpitDebugLocation);
						// We override the rotation.
						SpawnedCockpit->CockpitSkeletalMeshComponent->SetRelativeRotation(CockpitDebugRotation);
					}
					
					SpawnedCockpit->CockpitSkeletalMeshComponent->SetOnlyOwnerSee(true);
					SpawnedCockpit->CockpitSkeletalMeshComponent->SetOwnerNoSee(false);

					SpawnedCockpit->CockpitLeftArmExplosionOffset = CockpitLeftArmExplosionOffset;
					SpawnedCockpit->CockpitRightArmExplosionOffset = CockpitRightArmExplosionOffset;
					SpawnedCockpit->CockpitLeftLegExplosionOffset = CockpitLeftLegExplosionOffset;
					SpawnedCockpit->CockpitRightLegExplosionOffset = CockpitRightLegExplosionOffset;

					if(CockpitFootStepsAudio != nullptr)
					{
						AAudio3dManagedObject* SpawnedCockpitFootStepsAudio = GetWorld()->SpawnActor<AAudio3dManagedObject>(CockpitFootStepsAudio, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
						SpawnedCockpitFootStepsAudio->AttachToComponent(SpawnedCockpit->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, NAME_None);
						SpawnedCockpit->CockpitFootStepsAudio = SpawnedCockpitFootStepsAudio;
					}

					if(CockpitEngineAudio != nullptr)
					{
						AAudio3dManagedObject* SpawnedCockpitEngineAudio = GetWorld()->SpawnActor<AAudio3dManagedObject>(CockpitEngineAudio, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
						SpawnedCockpitEngineAudio->AttachToComponent(SpawnedCockpit->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, NAME_None);
						SpawnedCockpit->CockpitEngineAudio = SpawnedCockpitEngineAudio;
					}

					if(CockpitRotationAudio != nullptr)
					{
						AAudio3dManagedObject* SpawnedCockpitRotationAudio = GetWorld()->SpawnActor<AAudio3dManagedObject>(CockpitRotationAudio, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
						SpawnedCockpitRotationAudio->AttachToComponent(SpawnedCockpit->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, NAME_None);
						SpawnedCockpit->CockpitRotationAudio = SpawnedCockpitRotationAudio;
					}

					if(CockpitJumpJetAudio != nullptr)
					{
						AAudio3dManagedObject* SpawnedCockpitJumpJetAudio = GetWorld()->SpawnActor<AAudio3dManagedObject>(CockpitJumpJetAudio, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
						SpawnedCockpitJumpJetAudio->AttachToComponent(SpawnedCockpit->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, NAME_None);
						SpawnedCockpit->CockpitJumpJetAudio = SpawnedCockpitJumpJetAudio;
					}
					
					SpawnedCockpit->InitializeCockpit();
					
					CockpitReference = SpawnedCockpit;
					bHasSpawnedCockpitOnStandAlone = true;
				}
			}
		}
	}
	else if(GetWorld()->GetNetMode() == NM_Client)
	{
		if(APlayerController* PlayerController = Cast<APlayerController>(Controller))
		{
			/*
			* Perform a console command for VR for this specific local player/client only.
			* The vr.PixelDensity cvar will under or over-sample the resolution for your Head Mounted Display.
			* More info: https://www.unrealengine.com/blog/significant-changes-coming-to-vr-resolution-settings-in-4-19.
			*/
			if(PlayerController->IsLocalPlayerController())
			{
				bIsUsingStereoRendering = false;
				
			    // We get the vr stereo if existing. We check if this is being used in this frame.
			    IHeadMountedDisplay* VrHmd = nullptr;
			    TSharedPtr<IStereoRendering, ESPMode::ThreadSafe> VrStereo = nullptr;
			    if (GEngine)
			    {
			    	if(GEngine->XRSystem != nullptr)
			    	{
			    		if(GEngine->XRSystem->GetHMDDevice() != nullptr)
			    		{
			    			// Not being used right now, just keeping this we might need this at a later time.
			    			VrHmd = GEngine->XRSystem->GetHMDDevice();
			    			// Is use to check if vr is enabled at this frame.
			    			VrStereo = GEngine->XRSystem->GetStereoRenderingDevice();

			    			if(VrStereo->IsStereoEnabled()) bIsUsingStereoRendering = true;
			    		}
			    	}
			    }

			    if(bIsUsingStereoRendering)
			    {
			        // We set this to local as for seated experiences.		        
	                UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Local);
	                UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition(0.f, EOrientPositionSelector::OrientationAndPosition);
				    //UHeadMountedDisplayFunctionLibrary::SetClippingPlanes(0.00000001f, 999999999999999.f);
			        PlayerController->ConsoleCommand("vr.PixelDensity 1.0");
		
				    if(UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
				    {
					    InputSubsystem->AddMappingContext(VrMappingContext, 0);
				    }
			    }
			    else
			    {
				    if(UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
				    {
					    InputSubsystem->AddMappingContext(PcMappingContext, 0);
				    }	    
			    }

				// Spawn cockpit here.
				FActorSpawnParameters SpawnParameters;
				SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				SpawnParameters.Owner = this;
				SpawnParameters.Instigator = this;
				
				ATitanZeroCockpit* SpawnedCockpit = GetWorld()->SpawnActor<ATitanZeroCockpit>(TargetMechCockpit, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
				SpawnedCockpit->AttachToComponent(CockpitParentComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, NAME_None);

				SpawnedCockpit->CockpitSkeletalMeshComponent->SetRelativeLocation(CockpitLocation);
				SpawnedCockpit->CockpitSkeletalMeshComponent->SetRelativeRotation(CockpitRotation);
				SpawnedCockpit->CockpitSkeletalMeshComponent->SetRelativeScale3D(CockpitMeshScale);
				
				if(!bIsUsingStereoRendering)
				{
					// We override the scale.
					SpawnedCockpit->CockpitSkeletalMeshComponent->SetWorldScale3D(CockpitDebugMeshScale);
					// We override the location.
					SpawnedCockpit->CockpitSkeletalMeshComponent->SetRelativeLocation(CockpitDebugLocation);
					// We override the rotation.
					SpawnedCockpit->CockpitSkeletalMeshComponent->SetRelativeRotation(CockpitDebugRotation);
				}

				SpawnedCockpit->CockpitSkeletalMeshComponent->SetOnlyOwnerSee(true);
				SpawnedCockpit->CockpitSkeletalMeshComponent->SetOwnerNoSee(false);

				SpawnedCockpit->CockpitLeftArmExplosionOffset = CockpitLeftArmExplosionOffset;
				SpawnedCockpit->CockpitRightArmExplosionOffset = CockpitRightArmExplosionOffset;
				SpawnedCockpit->CockpitLeftLegExplosionOffset = CockpitLeftLegExplosionOffset;
				SpawnedCockpit->CockpitRightLegExplosionOffset = CockpitRightLegExplosionOffset;

				if(CockpitFootStepsAudio != nullptr)
				{
					AAudio3dManagedObject* SpawnedCockpitFootStepsAudio = GetWorld()->SpawnActor<AAudio3dManagedObject>(CockpitFootStepsAudio, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
					SpawnedCockpitFootStepsAudio->AttachToComponent(SpawnedCockpit->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, NAME_None);
					SpawnedCockpit->CockpitFootStepsAudio = SpawnedCockpitFootStepsAudio;
				}

				if(CockpitEngineAudio != nullptr)
				{
					AAudio3dManagedObject* SpawnedCockpitEngineAudio = GetWorld()->SpawnActor<AAudio3dManagedObject>(CockpitEngineAudio, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
					SpawnedCockpitEngineAudio->AttachToComponent(SpawnedCockpit->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, NAME_None);
					SpawnedCockpit->CockpitEngineAudio = SpawnedCockpitEngineAudio;
				}

				if(CockpitRotationAudio != nullptr)
				{
					AAudio3dManagedObject* SpawnedCockpitRotationAudio = GetWorld()->SpawnActor<AAudio3dManagedObject>(CockpitRotationAudio, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
					SpawnedCockpitRotationAudio->AttachToComponent(SpawnedCockpit->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, NAME_None);
					SpawnedCockpit->CockpitRotationAudio = SpawnedCockpitRotationAudio;
				}

				if(CockpitJumpJetAudio != nullptr)
				{
					AAudio3dManagedObject* SpawnedCockpitJumpJetAudio = GetWorld()->SpawnActor<AAudio3dManagedObject>(CockpitJumpJetAudio, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
					SpawnedCockpitJumpJetAudio->AttachToComponent(SpawnedCockpit->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, NAME_None);
					SpawnedCockpit->CockpitJumpJetAudio = SpawnedCockpitJumpJetAudio;
				}
				
				SpawnedCockpit->InitializeCockpit();
				
				CockpitReference = SpawnedCockpit;
			}
		}
	}

	OnNotifyControllerChanged();
}

void ATitanZeroMech::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(ATitanZeroMech, bIsTurning, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ATitanZeroMech, bIsOverheating, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ATitanZeroMech, HeatSinkValue, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ATitanZeroMech, HeatSinkVeryHotMeterCounter, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ATitanZeroMech, MechState, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ATitanZeroMech, bIsJumpJetActivated, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(ATitanZeroMech, LeftArmWeaponActivated, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ATitanZeroMech, RightArmWeaponActivated, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ATitanZeroMech, TorsoWeaponActivated, COND_None, REPNOTIFY_Always);
}

void ATitanZeroMech::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if(Controller != nullptr)
	{
		if(Controller->IsLocalPlayerController())
		{
			// We discontinue jump jet progress.
			bIsJumpJetInProgress = false;
			// If this is valid, we clear this first.
			if(JumpJetCoolDownLoopTimerHandle.IsValid()) GetWorld()->GetTimerManager().ClearTimer(JumpJetCoolDownLoopTimerHandle);
			// We start the cool down timer.
			GetWorld()->GetTimerManager().SetTimer(JumpJetCoolDownLoopTimerHandle, this, &ATitanZeroMech::OnJumpJetCoolDownLoopTimerHandle, 0.1f, true, 0.1f);
		}
	}
}

void ATitanZeroMech::OnPlayFootStepSoundEffect_Implementation()
{
	IMechSoundEffectsPlayable::OnPlayFootStepSoundEffect_Implementation();

	// Implements in blueprint.
}

void ATitanZeroMech::OnLockByLocalMech_Implementation()
{
	ILockable::OnLockByLocalMech_Implementation();

	// Implements in blueprint.
}

void ATitanZeroMech::OnUnlockByLocalMech_Implementation()
{
	ILockable::OnUnlockByLocalMech_Implementation();

	// Implements in blueprint.
}

void ATitanZeroMech::OnWhenDestroyed_Implementation()
{
	IDestroyable::OnWhenDestroyed_Implementation();

	// Implements in blueprint.
}

void ATitanZeroMech::OnWhenLeftArmDestroyed_Implementation()
{
	IDestroyable::OnWhenLeftArmDestroyed_Implementation();

	// Implements in blueprint.
}

void ATitanZeroMech::OnWhenRightArmDestroyed_Implementation()
{
	IDestroyable::OnWhenRightArmDestroyed_Implementation();

	// Implements in blueprint.
}

void ATitanZeroMech::OnWhenLeftLegDestroyed_Implementation()
{
	IDestroyable::OnWhenLeftLegDestroyed_Implementation();

	// We just implement a speed adjustment when a leg has been damaged.
	bIsOrAreLegsDamaged = true;

	// Implements in blueprint.
}

void ATitanZeroMech::OnWhenRightLegDestroyed_Implementation()
{
	IDestroyable::OnWhenRightLegDestroyed_Implementation();

	// We just implement a speed adjustment when a leg has been damaged.
	bIsOrAreLegsDamaged = true;

	// Implements in blueprint.
}

TArray<FVector> ATitanZeroMech::GetCockpitTargetLocations()
{
	return CockpitReference->GetTargetLocations();
}

FVector ATitanZeroMech::GetCockpitLeftArmTargetLocation()
{
	return CockpitReference->GetLeftArmTargetLocation();
}

FVector ATitanZeroMech::GetCockpitRightArmTargetLocation()
{
	return CockpitReference->GetRightArmTargetLocation();
}

void ATitanZeroMech::Server_InitializeMech_Implementation(const FName& InMechFriendlyName)
{
	if(GetWorld()->GetNetMode() == NM_Standalone)
	{
		// We need to identify the mech type so that we can load the proper mech customization. e.g. small, assault, sentinel.
		if(InMechFriendlyName.ToString().Contains("{SMALL}"))
		{
			// Iterate through the small mech data assets and find the mech.
			for(UTitanZeroMechDataAsset* const& ThisMech : SmallMechDataAssets)
			{
				if(ThisMech->FriendlyName == InMechFriendlyName)
				{
					ConfigureMech(EMaochiGamesNetMode::StandAlone, ThisMech);
					break;
				}
			}
		}
		else if(InMechFriendlyName.ToString().Contains("{ASSAULT}"))
		{
		
		}
		else if(InMechFriendlyName.ToString().Contains("{SENTINEL}"))
		{
		
		}
	}
	else
	{
		// We need to identify the mech type so that we can load the proper mech customization. e.g. small, assault, sentinel.
		if(InMechFriendlyName.ToString().Contains("{SMALL}"))
		{
			// Iterate through the small mech data assets and find the mech.
			for(UTitanZeroMechDataAsset* const& ThisMech : SmallMechDataAssets)
			{
				if(ThisMech->FriendlyName == InMechFriendlyName)
				{
					ConfigureMech(EMaochiGamesNetMode::DedicatedServer, ThisMech);
					break;
				}
			}
		}
		else if(InMechFriendlyName.ToString().Contains("{ASSAULT}"))
		{
		
		}
		else if(InMechFriendlyName.ToString().Contains("{SENTINEL}"))
		{
		
		}

		NetMulticast_InitializeMech(InMechFriendlyName);
	}
}

void ATitanZeroMech::NetMulticast_InitializeMech_Implementation(const FName& InMechFriendlyName)
{
	// We do not want server here.
	if(HasAuthority()) return;

	// We need to identify the mech type so that we can load the proper mech customization. e.g. small, assault, sentinel.
	if(InMechFriendlyName.ToString().Contains("{SMALL}"))
	{
		// Iterate through the small mech data assets and find the mech.
		for(UTitanZeroMechDataAsset* const& ThisMech : SmallMechDataAssets)
		{
			if(ThisMech->FriendlyName == InMechFriendlyName)
			{
				ConfigureMech(EMaochiGamesNetMode::Client, ThisMech);
				break;
			}
		}
	}
	else if(InMechFriendlyName.ToString().Contains("{ASSAULT}"))
	{
		
	}
	else if(InMechFriendlyName.ToString().Contains("{SENTINEL}"))
	{
		
	}
}

void ATitanZeroMech::Server_MechIsTurning_Implementation(const bool bValue)
{
	bIsTurning = bValue;
	OnRep_bIsTurning();
}

void ATitanZeroMech::Server_UpdateTorsoRotation_Implementation(const FQuantizedVector& InRotation)
{
	if(GetWorld()->GetNetMode() == NM_Standalone)
	{
		TorsoPitchRotation = InRotation.QuantizedVector.X;
		TorsoYawRotation = InRotation.QuantizedVector.Y;
	}
	else
	{
		NetMulticast_UpdateTorsoRotation(InRotation);
	}
}

void ATitanZeroMech::NetMulticast_UpdateTorsoRotation_Implementation(const FQuantizedVector& InRotation)
{
	TorsoPitchRotation = InRotation.QuantizedVector.X;
	TorsoYawRotation = InRotation.QuantizedVector.Y;
}

void ATitanZeroMech::Server_StopOverheating_Implementation()
{
	// We set this mech to stop overheating.
	bIsOverheating = false;
	OnRep_bIsOverheating();
}

void ATitanZeroMech::Server_SetMechIsReady_Implementation()
{
	MechState = 1;
	OnRep_MechState();
}

void ATitanZeroMech::Server_AddHeatSink_Implementation(const uint8& InValue)
{
	HeatSinkValue += InValue;
	HeatSinkValue = FMath::Clamp(HeatSinkValue, HEAT_SINK_MIN_VALUE, HEAT_SINK_MAX_VALUE);
	OnRep_HeatSinkValue();

	if(HeatSinkValue >= HEAT_SINK_MAX_VALUE)
	{
		HeatSinkVeryHotMeterCounter += 1;
		HeatSinkVeryHotMeterCounter = FMath::Clamp(HeatSinkVeryHotMeterCounter, HEAT_SINK_MIN_METER_COUNTER, HEAT_SINK_MAX_METER_COUNTER);
		OnRep_HeatSinkVeryHotMeterCounter();

		// // In this case we are going to shutdown.
		if(HeatSinkVeryHotMeterCounter >= HEAT_SINK_MAX_METER_COUNTER)
		{
			// If this is currently overheating, just don't do anything.
			if(bIsOverheating) return;

			// We set this mech to overheating.
			bIsOverheating = true;
			OnRep_bIsOverheating();
		}
	}

	// We delay a little bit before cooling down.
	GetWorld()->GetTimerManager().SetTimer(HeatSinkBufferingBeforeCoolingDown, this, &ATitanZeroMech::OnHeatSinkBufferingBeforeCoolingDownTimerHandle, HeatSinkBufferTimeBeforeCoolDown, false, HeatSinkBufferTimeBeforeCoolDown);
}

void ATitanZeroMech::Server_TakeDamage_Implementation(const uint8& InPartToDamage, const int32& InDamageValue)
{
	switch(InPartToDamage)
	{
	case 1:
		// Left arm.
		if(ATitanZeroPlayerState* TitanZeroPlayerState = Cast<ATitanZeroPlayerState>(GetPlayerState()))
		{
			TitanZeroPlayerState->LeftArmHealth -= InDamageValue;
			TitanZeroPlayerState->LeftArmHealth = FMath::Clamp(TitanZeroPlayerState->LeftArmHealth, 0, 1000);
			TitanZeroPlayerState->OnRep_LeftArmHealth();
		}
		break;
	case 2:
		// Right arm.
		if(ATitanZeroPlayerState* TitanZeroPlayerState = Cast<ATitanZeroPlayerState>(GetPlayerState()))
		{
			TitanZeroPlayerState->RightArmHealth -= InDamageValue;
			TitanZeroPlayerState->RightArmHealth = FMath::Clamp(TitanZeroPlayerState->RightArmHealth, 0, 1000);
			TitanZeroPlayerState->OnRep_RightArmHealth();
		}
		break;
	case 3:
		// Left leg.
		if(ATitanZeroPlayerState* TitanZeroPlayerState = Cast<ATitanZeroPlayerState>(GetPlayerState()))
		{
			TitanZeroPlayerState->LeftLegHealth -= InDamageValue;
			TitanZeroPlayerState->LeftLegHealth = FMath::Clamp(TitanZeroPlayerState->LeftLegHealth, 0, 1000);
			TitanZeroPlayerState->OnRep_LeftLegHealth();
		}
		break;
	case 4:
		// Right leg.
		if(ATitanZeroPlayerState* TitanZeroPlayerState = Cast<ATitanZeroPlayerState>(GetPlayerState()))
		{
			TitanZeroPlayerState->RightLegHealth -= InDamageValue;
			TitanZeroPlayerState->RightLegHealth = FMath::Clamp(TitanZeroPlayerState->RightLegHealth, 0, 1000);
			TitanZeroPlayerState->OnRep_RightLegHealth();
		}
		break;
	default:
		// Default is torso.
		if(ATitanZeroPlayerState* TitanZeroPlayerState = Cast<ATitanZeroPlayerState>(GetPlayerState()))
		{
			TitanZeroPlayerState->TorsoHealth -= InDamageValue;
			TitanZeroPlayerState->TorsoHealth = FMath::Clamp(TitanZeroPlayerState->TorsoHealth, 0, 1000);
			TitanZeroPlayerState->OnRep_TorsoHealth();
		}
		break;
	}
}

void ATitanZeroMech::Server_SetLeftArmWeaponsActivation_Implementation(bool bIsActivationValue)
{
	LeftArmWeaponActivated = bIsActivationValue;
	OnRep_LeftArmWeaponActivated();
}

void ATitanZeroMech::Server_SetRightArmWeaponsActivation_Implementation(bool bIsActivationValue)
{
	RightArmWeaponActivated = bIsActivationValue;
	OnRep_RightArmWeaponActivated();
}

void ATitanZeroMech::Server_SetTorsoWeaponsActivation_Implementation(bool bIsActivationValue)
{
	TorsoWeaponActivated = bIsActivationValue;
	OnRep_TorsoWeaponActivated();
}

void ATitanZeroMech::Server_UpdateTorsoCrosshairLocation_Implementation(const FQuantizedVector& InLocation)
{
	if(GetWorld()->GetNetMode() == NM_Standalone)
	{
		TorsoCrosshairComponent->SetWorldLocation(InLocation.QuantizedVector);
	}
	else
	{
		NetMulticast_UpdateTorsoCrosshairLocation(InLocation);
	}	
}

void ATitanZeroMech::NetMulticast_UpdateTorsoCrosshairLocation_Implementation(const FQuantizedVector& InLocation)
{
	TorsoCrosshairComponent->SetWorldLocation(InLocation.QuantizedVector);
}

USceneComponent* ATitanZeroMech::GetCockpitParentComponent()
{
	return CockpitParentComponent;
}

float ATitanZeroMech::GetMechSpeedClampNormalized()
{
	if(Controller != nullptr)
	{
		if(Controller->IsLocalPlayerController())
		{
			return FMath::Abs(GetVelocity().Length()) / ForwardSpeed;
		}
	}

	return 0.f;
}

float ATitanZeroMech::GetMechSpeedInKilometers()
{
	return UTitanZeroHelperUtility::Exec_ConvertUnrealUnitsToKilometers(GetVelocity().Length());
}

bool ATitanZeroMech::CheckIfLeftArmStillOnline()
{
	if(const ATitanZeroPlayerState* TitanZeroPlayerState = Cast<ATitanZeroPlayerState>(GetPlayerState()))
	{
		if(TitanZeroPlayerState->LeftArmHealth > 0) return true;
	}

	return false;
}

bool ATitanZeroMech::CheckIfRightArmStillOnline()
{
	if(const ATitanZeroPlayerState* TitanZeroPlayerState = Cast<ATitanZeroPlayerState>(GetPlayerState()))
	{
		if(TitanZeroPlayerState->RightArmHealth > 0) return true;
	}

	return false;
}

void ATitanZeroMech::SetAttackingMech(ATitanZeroMech* InAttackingMech)
{
	OnReceivedAttackingMech.Broadcast(InAttackingMech);
	GetWorld()->GetTimerManager().SetTimer(AttackingMechTimerHandle, this, &ATitanZeroMech::OnAttackingMechTimerHandleComplete, GLOBAL_RECEIVED_ATTACKING_MECH_TIME, false, GLOBAL_RECEIVED_ATTACKING_MECH_TIME);
}

bool ATitanZeroMech::Acquire_A_Mech()
{
	// Check if this mech can acquire.
	if(!bCanAcquireMech) return false;
	
	// If the mech controller is disabled, we return.
	if(bIsControllerDisabled) return false;

	// We first grab our own player state. We store this to a variable since we will use this later on.
	if(RemoteTitanZeroPlayerState == nullptr) RemoteTitanZeroPlayerState = Cast<ATitanZeroPlayerState>(GetPlayerState());

	/* We have to do this everytime as sometimes, players disconnect so we keep track of the record. */
	TArray<ATitanZeroMech*> EnemyMechs;
	
	// We loop through each player via reference instead of copy.
	// Hopefully this does not matter as we only have 10 players.
	for(const TObjectPtr<APlayerState>& ThisPlayer : GetWorld()->GetGameState()->PlayerArray)
	{
		if(const ATitanZeroPlayerState* ThisPlayerState = Cast<ATitanZeroPlayerState>(ThisPlayer))
		{
			if(RemoteTitanZeroPlayerState->TeamId != ThisPlayerState->TeamId)
			{
				if(ATitanZeroMech* CurrentMech = Cast<ATitanZeroMech>(ThisPlayerState->GetPawn()))
				{
					EnemyMechs.Add(CurrentMech);
				}
			}
		}
	}

	// Now we need to get the first one as our first reference.
	ATitanZeroMech* CurrentSelectedMech = EnemyMechs[0];

	for(ATitanZeroMech* EnemyMech : EnemyMechs)
	{
		if(const ATitanZeroPlayerState* EnemyMechPlayerState = Cast<ATitanZeroPlayerState>(EnemyMech->GetPlayerState()))
		{
			if(EnemyMechPlayerState->TorsoHealth <= 0.f) continue;
		}
		
		// We compute the distance.
		const float DistanceToTheLocalMech = UTitanZeroHelperUtility::Exec_ActorGetDistanceInKilometers(GetActorLocation(), EnemyMech->GetActorLocation());
		const bool bIsInValidDistance = DistanceToTheLocalMech <= EnemyMech->MaxVisibilityDistance ? true : false;

		if(!bIsInValidDistance) continue;
		
		const FVector SourceForwardVector = CockpitTargetRotationComponent->GetForwardVector();
		const FVector SourceLocation = CockpitTargetRotationComponent->GetComponentLocation();

		const FVector PreviousTargetLocation = CurrentSelectedMech->LineTracerHeadComponent->GetComponentLocation();
		const FVector CurrentTargetLocation = EnemyMech->LineTracerHeadComponent->GetComponentLocation();
		
		const float PreviousGivenDotProduct = UTitanZeroHelperUtility::Exec_GetAngleFromCenterScreen(SourceForwardVector, SourceLocation, PreviousTargetLocation);
		const float CurrentGivenDotProduct = UTitanZeroHelperUtility::Exec_GetAngleFromCenterScreen(SourceForwardVector, SourceLocation, CurrentTargetLocation);

		// We compare if current is closer to the center, always remember the
		// closer to the center has higher value that is why we used greater than.
		if(CurrentGivenDotProduct > PreviousGivenDotProduct) CurrentSelectedMech = EnemyMech;
	}

	if(CurrentSelectedMech != nullptr)
	{
		if(const ATitanZeroPlayerState* EnemyMechPlayerState = Cast<ATitanZeroPlayerState>(CurrentSelectedMech->GetPlayerState()))
		{
			if(EnemyMechPlayerState->TorsoHealth <= 0.f)
			{
				CurrentSelectedMech = nullptr;
				return false;
			}
		}
		
		// We compute the distance.
		const float DistanceToTheLocalMech = UTitanZeroHelperUtility::Exec_ActorGetDistanceInKilometers(GetActorLocation(), CurrentSelectedMech->GetActorLocation());
		const bool bIsInValidDistance = DistanceToTheLocalMech <= CurrentSelectedMech->MaxVisibilityDistance ? true : false;

		if(!bIsInValidDistance) return false;
		
		const FVector SourceForwardVector = CockpitTargetRotationComponent->GetForwardVector();
		const FVector SourceLocation = CockpitTargetRotationComponent->GetComponentLocation();
		const FVector CurrentTargetLocation = CurrentSelectedMech->LineTracerHeadComponent->GetComponentLocation();

		const float CurrentGivenDotProduct = UTitanZeroHelperUtility::Exec_GetAngleFromCenterScreen(SourceForwardVector, SourceLocation, CurrentTargetLocation);

		if(CurrentGivenDotProduct >= MaxViewAngle)
		{
			if(CurrentLockedTargetMech != nullptr)
			{
				if(CurrentLockedTargetMech->GetUniqueID() == CurrentSelectedMech->GetUniqueID()) return false;
			}
			
			if(CurrentSelectedMech->GetClass()->ImplementsInterface(ULockable::StaticClass()))
			{
				if(const ILockable* Lockable = Cast<ILockable>(CurrentSelectedMech))
				{
					Lockable->Execute_OnLockByLocalMech(CurrentSelectedMech);

					if(CurrentLockedTargetMech != nullptr)
					{
						if(CurrentLockedTargetMech->GetClass()->ImplementsInterface(ULockable::StaticClass()))
						{
							if(const ILockable* PreviousLockable = Cast<ILockable>(CurrentLockedTargetMech))
							{
								PreviousLockable->Execute_OnUnlockByLocalMech(CurrentLockedTargetMech);
							}
						}
					}

					// We assign this as our active locked target mech.
					CurrentLockedTargetMech = CurrentSelectedMech;

					OnLocked_A_Mech(CurrentLockedTargetMech);

					// We reset the timer.
					if(UnlockMechTimerHandle.IsValid()) GetWorld()->GetTimerManager().ClearTimer(UnlockMechTimerHandle);

					// We start the timer on acquire hold time.
					GetWorld()->GetTimerManager().SetTimer(UnlockMechTimerHandle, this, &ATitanZeroMech::OnUnlockMechTimerHandle, AcquireHoldTime, false, AcquireHoldTime);
					return true;
				}
			}
		}
	}
	
	// This automatically return false, this is failed.
	return false;
}

void ATitanZeroMech::UpdateTorsoRotation(USceneComponent* InHeadCrosshairComponent)
{
	// If the mech controller is disabled, we return.
	if(bIsControllerDisabled) return;
	
	// If this is running on vr.
	if(bIsUsingStereoRendering)
	{
		// If player is holding down right controller.
		if(bIsPlayerHoldingRightController)
		{
			const FRotator CurrentCockpitRotation = UKismetMathLibrary::FindLookAtRotation(CockpitTargetRotationComponent->GetComponentLocation(), InHeadCrosshairComponent->GetComponentLocation());
			CockpitTargetRotationComponent->SetWorldRotation(FMath::RInterpTo(CockpitTargetRotationComponent->GetComponentRotation(), CurrentCockpitRotation, GetWorld()->GetDeltaSeconds(), TorsoRotationSpeed));

			FRotator TargetRotation = FMath::RInterpTo(CockpitParentComponent->GetRelativeRotation(), CockpitTargetRotationComponent->GetRelativeRotation(), GetWorld()->GetDeltaSeconds(), TorsoRotationSpeed);
			TargetRotation.Pitch = FMath::Clamp(TargetRotation.Pitch, TorsoRotationPitchMinimum, TorsoRotationPitchMaximum);
			TargetRotation.Yaw = FMath::Clamp(TargetRotation.Yaw, TorsoRotationYawMinimum, TorsoRotationYawMaximum);

			CockpitParentComponent->SetRelativeRotation(TargetRotation);

			const float TorsoRotationVelocity = FMath::Abs(TargetRotation.Pitch - LastTorsoRotationPitch) + FMath::Abs(TargetRotation.Yaw - LastTorsoRotationYaw);
			if(TorsoRotationVelocity > 0.1f) OnTorsoRotationUpdate(1.f);
			else OnTorsoRotationUpdate(0.f);

			LastTorsoRotationPitch = TargetRotation.Pitch;
			LastTorsoRotationYaw = TargetRotation.Yaw;

			if(GetWorld()->GetNetMode() == NM_Standalone)
			{
				FQuantizedVector QuantizedVector;
				QuantizedVector.QuantizedVector.X = TargetRotation.Pitch;
				QuantizedVector.QuantizedVector.Y = TargetRotation.Yaw;
				Server_UpdateTorsoRotation(QuantizedVector);
			}
			else
			{
				if(GetLocalRole() == ROLE_AutonomousProxy)
				{
					FQuantizedVector QuantizedVector;
					QuantizedVector.QuantizedVector.X = TargetRotation.Pitch;
					QuantizedVector.QuantizedVector.Y = TargetRotation.Yaw;
					Server_UpdateTorsoRotation(QuantizedVector);
				}
			}
		}
		else
		{
			OnTorsoRotationUpdate(0.f);
		}
	}
	else
	{
		// If player is holding down right controller.
		if(bIsPlayerHoldingRightController)
		{
			const FRotator CurrentCockpitRotation = UKismetMathLibrary::FindLookAtRotation(CockpitTargetRotationComponent->GetComponentLocation(), InHeadCrosshairComponent->GetComponentLocation());
			CockpitTargetRotationComponent->SetWorldRotation(FMath::RInterpTo(CockpitTargetRotationComponent->GetComponentRotation(), CurrentCockpitRotation, GetWorld()->GetDeltaSeconds(), TorsoRotationSpeed));

			FRotator TargetRotation = FMath::RInterpTo(CockpitParentComponent->GetRelativeRotation(), CockpitTargetRotationComponent->GetRelativeRotation(), GetWorld()->GetDeltaSeconds(), TorsoRotationSpeed);
			TargetRotation.Pitch = FMath::Clamp(TargetRotation.Pitch, TorsoRotationPitchMinimum, TorsoRotationPitchMaximum);
			TargetRotation.Yaw = FMath::Clamp(TargetRotation.Yaw, TorsoRotationYawMinimum, TorsoRotationYawMaximum);

			CockpitParentComponent->SetRelativeRotation(TargetRotation);

			const float TorsoRotationVelocity = FMath::Abs(TargetRotation.Pitch - LastTorsoRotationPitch) + FMath::Abs(TargetRotation.Yaw - LastTorsoRotationYaw);
			
			if(TorsoRotationVelocity > 0.1f) OnTorsoRotationUpdate(1.f);
			else OnTorsoRotationUpdate(0.f);

			LastTorsoRotationPitch = TargetRotation.Pitch;
			LastTorsoRotationYaw = TargetRotation.Yaw;

			// We send this to the network for actual mech torso rotation.
			if(GetWorld()->GetNetMode() == NM_Standalone)
			{
				FQuantizedVector QuantizedVector;
				QuantizedVector.QuantizedVector.X = TargetRotation.Pitch;
				QuantizedVector.QuantizedVector.Y = TargetRotation.Yaw;
				Server_UpdateTorsoRotation(QuantizedVector);
			}
			else
			{
				if(GetLocalRole() == ROLE_AutonomousProxy)
				{
					FQuantizedVector QuantizedVector;
					QuantizedVector.QuantizedVector.X = TargetRotation.Pitch;
					QuantizedVector.QuantizedVector.Y = TargetRotation.Yaw;
					Server_UpdateTorsoRotation(QuantizedVector);
				}
			}
		}
		else
		{
			OnTorsoRotationUpdate(0.f);
		}
	}
}

void ATitanZeroMech::ResetMech()
{
	// Since this is shared skeleton, all mechs have these bone names.
	GetMesh()->HideBoneByName("Bone_RArmWreck", PBO_None);
	GetMesh()->HideBoneByName("Bone_LArmWreck", PBO_None);
}

void ATitanZeroMech::SetupMinimapTaggedActor(float BelowMechDistance)
{
	// Spawn cockpit here.
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = this;

	const FVector TargetLocation = FVector(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z - BelowMechDistance);
	AMinimapTaggedActor* ThisMinimapTaggedActor = GetWorld()->SpawnActor<AMinimapTaggedActor>(MinimapTaggedActor, TargetLocation, GetActorRotation(), SpawnParameters);
	ThisMinimapTaggedActor->InitializeTaggedActor(this);
}

UUserWidget* ATitanZeroMech::SetupAcquireIcon()
{
	MechInfoWidgetComponent->SetWidgetClass(MechInfoWidgetReference);
	return MechInfoWidgetComponent->GetWidget();
}

UUserWidget* ATitanZeroMech::SetupHeadIcon()
{
	MechHeadIndicatorWidgetComponent->SetWidgetClass(MechHeadIndicatorReference);
	return MechHeadIndicatorWidgetComponent->GetWidget();
}

void ATitanZeroMech::DisableMech()
{
	bIsControllerDisabled = true;
	GetCharacterMovement()->Velocity = FVector::ZeroVector;
	// Based on test, 22 is the perfect height.
	PlayerOffsetComponent->SetRelativeLocation(FVector(0.f, 0.f, 22.f));
}

void ATitanZeroMech::Server_ToggleJumpJet_Implementation(bool bIsInValue)
{
	bIsJumpJetActivated = bIsInValue;
	OnRep_bIsJumpJetActivated();
}

void ATitanZeroMech::Server_InitializePassiveAbilities_Implementation()
{
	if(ATitanZeroPlayerState* TitanZeroPlayerState = Cast<ATitanZeroPlayerState>(GetPlayerState()))
	{
		TitanZeroPlayerState->Server_InitializeHealth(MaxTorsoHealth, MaxLeftArmHealth, MaxRightArmHealth, MaxLeftLegHealth, MaxRightLegHealth);
	}
}

void ATitanZeroMech::ConfigureMech(const EMaochiGamesNetMode InMaochiGamesNetMode, UTitanZeroMechDataAsset* const& InMechData)
{
	if(InMaochiGamesNetMode == EMaochiGamesNetMode::StandAlone)
	{
		// Configure components.
		GetMesh()->SetSkeletalMesh(InMechData->BaseSkeletalMesh, true);
		GetMesh()->SetAnimInstanceClass(InMechData->BaseSkeletalMeshAnimationClass);
		GetMesh()->SetWorldScale3D(InMechData->BaseSkeletalMeshScale);
		GetMesh()->SetVisibility(true);
		GetMesh()->SetOwnerNoSee(true);
		GetMesh()->SetOnlyOwnerSee(false);
		
		CockpitComponent->SetRelativeLocation(PlayerOffsetComponent->GetRelativeLocation());
		CockpitComponent->SetRelativeRotation(PlayerOffsetComponent->GetRelativeRotation());

		CockpitTargetRotationComponent->SetRelativeLocation(FVector::ZeroVector);
		CockpitTargetRotationComponent->SetRelativeRotation(FRotator::ZeroRotator);
		CockpitParentComponent->SetRelativeLocation(FVector::ZeroVector);
		CockpitParentComponent->SetRelativeRotation(FRotator::ZeroRotator);

		TargetMechCockpit = InMechData->MechCockpit;
		CockpitLocation = InMechData->CockpitLocation;
		CockpitRotation = InMechData->CockpitRotation;
		CockpitMeshScale = InMechData->CockpitMeshScale;
		CockpitDebugMeshScale = InMechData->CockpitDebugMeshScale;
		CockpitDebugLocation = InMechData->CockpitDebugLocation;
		CockpitDebugRotation = InMechData->CockpitDebugRotation;
		
		// Configure properties.
		ForwardSpeed = InMechData->ForwardSpeed;
		BackwardSpeed = InMechData->BackwardSpeed;
		DamageForwardSpeed = InMechData->DamagedForwardSpeed;
		DamageBackwardSpeed = InMechData->DamagedBackwardSpeed;
		TurnSpeed = InMechData->TurnSpeed;
		TorsoRotationSpeed = InMechData->TorsoRotationSpeed;
		TorsoRotationPitchMinimum = InMechData->TorsoRotationPitchMinimum;
		TorsoRotationPitchMaximum = InMechData->TorsoRotationPitchMaximum;
		TorsoRotationYawMinimum = InMechData->TorsoRotationYawMinimum;
		TorsoRotationYawMaximum = InMechData->TorsoRotationYawMaximum;
		bHasJumpJet = InMechData->bHasJumpJet;
		JumpJetLiftDelay = InMechData->JumpJetLiftDelay;
		JumpJetMaxFuel = InMechData->JumpJetMaxFuel;
		JumpJetCurrentFuel = InMechData->JumpJetMaxFuel;
		HeatSinkBufferTimeBeforeCoolDown = InMechData->HeatSinkBufferTimeBeforeCoolDown;
		MechJumpZVelocity = InMechData->JumpJetZVelocity;
		MechAirControl = InMechData->JumpJetAirControl;
		MechBrakingDecelerationFalling = InMechData->JumpJetBrakingDecelerationFalling;
		
		MaxVisibilityDistance = InMechData->MaxVisibilityDistance;
		MaxViewAngle = InMechData->MaxViewAngle;
		MaxDelayBeforeIconDisappears = InMechData->MaxDelayBeforeIconDisappears;

		GetCapsuleComponent()->SetCapsuleRadius(InMechData->CapsuleRadius);
		GetCapsuleComponent()->SetCapsuleHalfHeight(InMechData->CapsuleHalfHeight);
		GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, InMechData->UpPosition));
		VrOriginComponent->SetRelativeLocation(FVector(0.f, 0.f, InMechData->UpPosition));
		VrOriginOffsetComponent->SetRelativeLocation(FVector(InMechData->OffsetForwardPosition, 0.f, InMechData->OffsetUpPosition));
		LineTracerHeadComponent->SetRelativeLocation(FVector(0.f, 0.f, InMechData->UpLineTracePosition));

		MinimapTaggedActor = InMechData->MinimapTaggedActor;
		
		MechInfoWidgetReference = InMechData->MechInfo;
		MechInfoWidgetComponent->SetRelativeLocation(InMechData->MechInfoPosition);
		MechInfoWidgetComponent->SetRelativeScale3D(InMechData->MechInfoInitialScale);
		MechInfoInitialScale = InMechData->MechInfoInitialScale;
		MechInfoMinimumScale = InMechData->MechInfoMinimumScale;
		MechInfoMaximumScale = InMechData->MechInfoMaximumScale;

		MechHeadIndicatorReference = InMechData->MechHeadIndicator;
		MechHeadIndicatorWidgetComponent->SetRelativeLocation(InMechData->MechHeadIndicatorPosition);
		MechHeadIndicatorWidgetComponent->SetRelativeScale3D(InMechData->MechHeadIndicatorInitialScale);
		MechHeadIndicatorInitialScale = InMechData->MechHeadIndicatorInitialScale;
		MechHeadIndicatorMinimumScale = InMechData->MechHeadIndicatorMinimumScale;
		MechHeadIndicatorMaximumScale = InMechData->MechHeadIndicatorMaximumScale;

		// We just store this for the meantime here then later on, we pass this to the player state.
		MaxTorsoHealth = InMechData->MaxTorsoHealth;
		MaxLeftArmHealth = InMechData->MaxLeftArmHealth;
		MaxRightArmHealth = InMechData->MaxRightArmHealth;
		MaxLeftLegHealth = InMechData->MaxLeftLegHealth;
		MaxRightLegHealth = InMechData->MaxRightLegHealth;

		bCanAcquireMech = InMechData->bCanAcquireMech;
		AcquireHoldTime = InMechData->AcquireHoldTime;

		DestroyedMaterial = InMechData->DestroyedMaterial;

		MechExplosionOffset = InMechData->MechExplosionOffset;
		MechLeftArmExplosionOffset = InMechData->MechLeftArmExplosionOffset;
		MechRightArmExplosionOffset = InMechData->MechRightArmExplosionOffset;
		MechLeftLegExplosionOffset = InMechData->MechLeftLegExplosionOffset;
		MechRightLegExplosionOffset = InMechData->MechRightLegExplosionOffset;

		CockpitLeftArmExplosionOffset = InMechData->CockpitLeftArmExplosionOffset;
		CockpitRightArmExplosionOffset = InMechData->CockpitRightArmExplosionOffset;
		CockpitLeftLegExplosionOffset = InMechData->CockpitLeftLegExplosionOffset;
		CockpitRightLegExplosionOffset = InMechData->CockpitRightLegExplosionOffset;

		LeftLegMaterialIndex = InMechData->LeftLegMaterialIndex;
		RightLegMaterialIndex = InMechData->RightLegMaterialIndex;

		if(InMechData->MechFootStepsAudio == nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("ATitanZeroMech::ConfigureMech - MechFootStepsAudio null, game will crash"));
			UE_LOG(LogTemp, Error, TEXT("ATitanZeroMech::ConfigureMech - MechFootStepsAudio null, game will crash"));
		}
		else
		{
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParameters.Owner = this;
			SpawnParameters.Instigator = this;

			AAudio3dManagedObject* SpawnedMechFootStepsAudio = GetWorld()->SpawnActor<AAudio3dManagedObject>(InMechData->MechFootStepsAudio, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
			SpawnedMechFootStepsAudio->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, NAME_None);
		
			MechFootStepsAudio = SpawnedMechFootStepsAudio;
		}
		
		if(InMechData->MechJumpJetAudio == nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("ATitanZeroMech::ConfigureMech - MechJumpJetAudio null, game will crash"));
			UE_LOG(LogTemp, Error, TEXT("ATitanZeroMech::ConfigureMech - MechJumpJetAudio null, game will crash"));
		}
		else
		{
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParameters.Owner = this;
			SpawnParameters.Instigator = this;

			AAudio3dManagedObject* SpawnedMechJumpJetAudio = GetWorld()->SpawnActor<AAudio3dManagedObject>(InMechData->MechJumpJetAudio, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
			SpawnedMechJumpJetAudio->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, NAME_None);
			
			MechJumpJetAudio = SpawnedMechJumpJetAudio;
		}

		if(InMechData->CockpitFootStepsAudio == nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("ATitanZeroMech::ConfigureMech - CockpitFootStepsAudio null, game will crash"));
			UE_LOG(LogTemp, Error, TEXT("ATitanZeroMech::ConfigureMech - CockpitFootStepsAudio null, game will crash"));
		}
		else
		{
			CockpitFootStepsAudio = InMechData->CockpitFootStepsAudio;
		}

		if(InMechData->CockpitEngineAudio == nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("ATitanZeroMech::ConfigureMech - CockpitEngineAudio null, game will crash"));
			UE_LOG(LogTemp, Error, TEXT("ATitanZeroMech::ConfigureMech - CockpitEngineAudio null, game will crash"));
		}
		else
		{
			CockpitEngineAudio = InMechData->CockpitEngineAudio;
		}

		if(InMechData->CockpitRotationAudio == nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("ATitanZeroMech::ConfigureMech - CockpitRotationAudio null, game will crash"));
			UE_LOG(LogTemp, Error, TEXT("ATitanZeroMech::ConfigureMech - CockpitRotationAudio null, game will crash"));
		}
		else
		{
			CockpitRotationAudio = InMechData->CockpitRotationAudio;
		}

		if(InMechData->CockpitJumpJetAudio == nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("ATitanZeroMech::ConfigureMech - CockpitJumpJetAudio null, game will crash"));
			UE_LOG(LogTemp, Error, TEXT("ATitanZeroMech::ConfigureMech - CockpitJumpJetAudio null, game will crash"));
		}
		else
		{
			CockpitJumpJetAudio = InMechData->CockpitJumpJetAudio;
		}

		if(InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData != nullptr)
		{
			if(UTitanZeroWeaponDataAsset* RightArmWeaponDataAsset = InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData)
			{
				RightArmWeaponComponent = Cast<UTitanZeroWeaponComponent>(AddComponentByClass(RightArmWeaponDataAsset->WeaponComponent, true, GetTransform(), false));
				RightArmWeaponComponent->SetIsReplicated(false);
				RightArmWeaponComponent->Exec_InitializeWeapon(
												    this,
													           EWeaponAttachmentLocation::RightArm,
													           InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponSocketNames,
													           InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->WeaponType,
													           InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->Variant,
													           InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->AmmunitionType,
													           InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->MaximumAmmunition,
													           InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->AmmunitionPerTrigger,
													           InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->Damage,
													           InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->DamageMaxDistance,
													           InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->DamageFallOffDistance,
													           InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->HeatSinkDamage,
													           InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->HeatSinkCost,
													           InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->CoolDownTime,
													           InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->WeaponAdditionalParameters);
			}
		}

		if(InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData != nullptr)
		{
			if(UTitanZeroWeaponDataAsset* LeftArmWeaponDataAsset = InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData)
			{
				LeftArmWeaponComponent = Cast<UTitanZeroWeaponComponent>(AddComponentByClass(LeftArmWeaponDataAsset->WeaponComponent, true, GetTransform(), false));
				LeftArmWeaponComponent->SetIsReplicated(false);
				LeftArmWeaponComponent->Exec_InitializeWeapon(
												   this,
															  EWeaponAttachmentLocation::LeftArm,
															  InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponSocketNames,
															  InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->WeaponType,
															  InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->Variant,
															  InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->AmmunitionType,
															  InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->MaximumAmmunition,
															  InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->AmmunitionPerTrigger,
															  InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->Damage,
															  InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->DamageMaxDistance,
															  InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->DamageFallOffDistance,
															  InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->HeatSinkDamage,
															  InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->HeatSinkCost,
															  InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->CoolDownTime,
															  InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->WeaponAdditionalParameters);
			}
		}

		if(InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData != nullptr)
		{
			if(UTitanZeroWeaponDataAsset* TorsoWeaponDataAsset = InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData)
			{
				TorsoWeaponComponent = Cast<UTitanZeroWeaponComponent>(AddComponentByClass(TorsoWeaponDataAsset->WeaponComponent, true, GetTransform(), false));
				TorsoWeaponComponent->SetIsReplicated(false);
				TorsoWeaponComponent->Exec_InitializeWeapon(
															this,
														    EWeaponAttachmentLocation::Torso,
														    InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponSocketNames,
														    InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->WeaponType,
														    InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->Variant,
														    InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->AmmunitionType,
														    InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->MaximumAmmunition,
														    InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->AmmunitionPerTrigger,
														    InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->Damage,
														    InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->DamageMaxDistance,
														    InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->DamageFallOffDistance,
														    InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->HeatSinkDamage,
															InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->HeatSinkCost,
															InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->CoolDownTime,
														    InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->WeaponAdditionalParameters);
			}
		}
	}
	else if(InMaochiGamesNetMode == EMaochiGamesNetMode::DedicatedServer)
	{
		// Configure components.
		GetMesh()->SetSkeletalMesh(InMechData->BaseSkeletalMesh, true);
		GetMesh()->SetAnimInstanceClass(InMechData->BaseSkeletalMeshAnimationClass);
		GetMesh()->SetWorldScale3D(InMechData->BaseSkeletalMeshScale);
		GetMesh()->SetVisibility(false);
		
		CockpitComponent->SetRelativeLocation(PlayerOffsetComponent->GetRelativeLocation());
		CockpitComponent->SetRelativeRotation(PlayerOffsetComponent->GetRelativeRotation());

		CockpitTargetRotationComponent->SetRelativeLocation(FVector::ZeroVector);
		CockpitTargetRotationComponent->SetRelativeRotation(FRotator::ZeroRotator);
		CockpitParentComponent->SetRelativeLocation(FVector::ZeroVector);
		CockpitParentComponent->SetRelativeRotation(FRotator::ZeroRotator);

		// Configure properties.
		ForwardSpeed = InMechData->ForwardSpeed;
		BackwardSpeed = InMechData->BackwardSpeed;
		DamageForwardSpeed = InMechData->DamagedForwardSpeed;
		DamageBackwardSpeed = InMechData->DamagedBackwardSpeed;
		TurnSpeed = InMechData->TurnSpeed;
		TorsoRotationSpeed = InMechData->TorsoRotationSpeed;
		TorsoRotationPitchMinimum = InMechData->TorsoRotationPitchMinimum;
		TorsoRotationPitchMaximum = InMechData->TorsoRotationPitchMaximum;
		TorsoRotationYawMinimum = InMechData->TorsoRotationYawMinimum;
		TorsoRotationYawMaximum = InMechData->TorsoRotationYawMaximum;
		bHasJumpJet = InMechData->bHasJumpJet;
		JumpJetLiftDelay = InMechData->JumpJetLiftDelay;
		JumpJetMaxFuel = InMechData->JumpJetMaxFuel;
		JumpJetCurrentFuel = InMechData->JumpJetMaxFuel;
		HeatSinkBufferTimeBeforeCoolDown = InMechData->HeatSinkBufferTimeBeforeCoolDown;
		MechJumpZVelocity = InMechData->JumpJetZVelocity;
		MechAirControl = InMechData->JumpJetAirControl;
		MechBrakingDecelerationFalling = InMechData->JumpJetBrakingDecelerationFalling;

		MaxVisibilityDistance = InMechData->MaxVisibilityDistance;
		MaxViewAngle = InMechData->MaxViewAngle;
		MaxDelayBeforeIconDisappears = InMechData->MaxDelayBeforeIconDisappears;

		GetCapsuleComponent()->SetCapsuleRadius(InMechData->CapsuleRadius);
		GetCapsuleComponent()->SetCapsuleHalfHeight(InMechData->CapsuleHalfHeight);
		GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, InMechData->UpPosition));
		VrOriginComponent->SetRelativeLocation(FVector(0.f, 0.f, InMechData->UpPosition));
		VrOriginOffsetComponent->SetRelativeLocation(FVector(InMechData->OffsetForwardPosition, 0.f, InMechData->OffsetUpPosition));
		LineTracerHeadComponent->SetRelativeLocation(FVector(0.f, 0.f, InMechData->UpLineTracePosition));

		// We just store this for the meantime here then later on, we pass this to the player state.
		MaxTorsoHealth = InMechData->MaxTorsoHealth;
		MaxLeftArmHealth = InMechData->MaxLeftArmHealth;
		MaxRightArmHealth = InMechData->MaxRightArmHealth;
		MaxLeftLegHealth = InMechData->MaxLeftLegHealth;
		MaxRightLegHealth = InMechData->MaxRightLegHealth;

		if(InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData != nullptr)
		{
			if(UTitanZeroWeaponDataAsset* RightArmWeaponDataAsset = InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData)
			{
				RightArmWeaponComponent = Cast<UTitanZeroWeaponComponent>(AddComponentByClass(RightArmWeaponDataAsset->WeaponComponent, true, GetTransform(), false));
				RightArmWeaponComponent->SetIsReplicated(false);
				RightArmWeaponComponent->Exec_InitializeWeapon(
															   this,
															   EWeaponAttachmentLocation::RightArm,
															   InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponSocketNames,
															   InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->WeaponType,
															   InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->Variant,
															   InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->AmmunitionType,
															   InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->MaximumAmmunition,
															   InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->AmmunitionPerTrigger,
															   InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->Damage,
															   InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->DamageMaxDistance,
															   InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->DamageFallOffDistance,
															   InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->HeatSinkDamage,
															   InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->HeatSinkCost,
															   InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->CoolDownTime,
															   InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->WeaponAdditionalParameters);
			}
		}

		if(InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData != nullptr)
		{
			if(UTitanZeroWeaponDataAsset* LeftArmWeaponDataAsset = InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData)
			{
				LeftArmWeaponComponent = Cast<UTitanZeroWeaponComponent>(AddComponentByClass(LeftArmWeaponDataAsset->WeaponComponent, true, GetTransform(), false));
				LeftArmWeaponComponent->SetIsReplicated(false);
				LeftArmWeaponComponent->Exec_InitializeWeapon(
														      this,
														      EWeaponAttachmentLocation::LeftArm,
														      InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponSocketNames,
														      InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->WeaponType,
														      InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->Variant,
														      InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->AmmunitionType,
														      InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->MaximumAmmunition,
														      InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->AmmunitionPerTrigger,
														      InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->Damage,
														      InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->DamageMaxDistance,
														      InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->DamageFallOffDistance,
														      InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->HeatSinkDamage,
															  InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->HeatSinkCost,
															  InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->CoolDownTime,
														      InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->WeaponAdditionalParameters);
			}
		}

		if(InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData != nullptr)
		{
			if(UTitanZeroWeaponDataAsset* TorsoWeaponDataAsset = InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData)
			{
				TorsoWeaponComponent = Cast<UTitanZeroWeaponComponent>(AddComponentByClass(TorsoWeaponDataAsset->WeaponComponent, true, GetTransform(), false));
				TorsoWeaponComponent->SetIsReplicated(false);
				TorsoWeaponComponent->Exec_InitializeWeapon(
														    this,
														    EWeaponAttachmentLocation::Torso,
														    InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponSocketNames,
														    InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->WeaponType,
														    InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->Variant,
														    InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->AmmunitionType,
														    InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->MaximumAmmunition,
														    InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->AmmunitionPerTrigger,
														    InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->Damage,
														    InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->DamageMaxDistance,
														    InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->DamageFallOffDistance,
														    InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->HeatSinkDamage,
															InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->HeatSinkCost,
															InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->CoolDownTime,
														    InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->WeaponAdditionalParameters);
			}
		}
	}
	else if(InMaochiGamesNetMode == EMaochiGamesNetMode::Client)
	{
		// Configure components.
		GetMesh()->SetSkeletalMesh(InMechData->BaseSkeletalMesh, true);
		GetMesh()->SetAnimInstanceClass(InMechData->BaseSkeletalMeshAnimationClass);
		GetMesh()->SetWorldScale3D(InMechData->BaseSkeletalMeshScale);
		GetMesh()->SetVisibility(true);
		GetMesh()->SetOwnerNoSee(true);
		GetMesh()->SetOnlyOwnerSee(false);
		
		CockpitComponent->SetRelativeLocation(PlayerOffsetComponent->GetRelativeLocation());
		CockpitComponent->SetRelativeRotation(PlayerOffsetComponent->GetRelativeRotation());

		CockpitTargetRotationComponent->SetRelativeLocation(FVector::ZeroVector);
		CockpitTargetRotationComponent->SetRelativeRotation(FRotator::ZeroRotator);
		CockpitParentComponent->SetRelativeLocation(FVector::ZeroVector);
		CockpitParentComponent->SetRelativeRotation(FRotator::ZeroRotator);

		TargetMechCockpit = InMechData->MechCockpit;
		CockpitLocation = InMechData->CockpitLocation;
		CockpitRotation = InMechData->CockpitRotation;
		CockpitMeshScale = InMechData->CockpitMeshScale;
		CockpitDebugMeshScale = InMechData->CockpitDebugMeshScale;
		CockpitDebugLocation = InMechData->CockpitDebugLocation;
		CockpitDebugRotation = InMechData->CockpitDebugRotation;
		
		// Configure properties.
		ForwardSpeed = InMechData->ForwardSpeed;
		BackwardSpeed = InMechData->BackwardSpeed;
		DamageForwardSpeed = InMechData->DamagedForwardSpeed;
		DamageBackwardSpeed = InMechData->DamagedBackwardSpeed;
		TurnSpeed = InMechData->TurnSpeed;
		TorsoRotationSpeed = InMechData->TorsoRotationSpeed;
		TorsoRotationPitchMinimum = InMechData->TorsoRotationPitchMinimum;
		TorsoRotationPitchMaximum = InMechData->TorsoRotationPitchMaximum;
		TorsoRotationYawMinimum = InMechData->TorsoRotationYawMinimum;
		TorsoRotationYawMaximum = InMechData->TorsoRotationYawMaximum;
		bHasJumpJet = InMechData->bHasJumpJet;
		JumpJetLiftDelay = InMechData->JumpJetLiftDelay;
		JumpJetMaxFuel = InMechData->JumpJetMaxFuel;
		JumpJetCurrentFuel = InMechData->JumpJetMaxFuel;
		HeatSinkBufferTimeBeforeCoolDown = InMechData->HeatSinkBufferTimeBeforeCoolDown;
		MechJumpZVelocity = InMechData->JumpJetZVelocity;
		MechAirControl = InMechData->JumpJetAirControl;
		MechBrakingDecelerationFalling = InMechData->JumpJetBrakingDecelerationFalling;

		MaxVisibilityDistance = InMechData->MaxVisibilityDistance;
		MaxViewAngle = InMechData->MaxViewAngle;
		MaxDelayBeforeIconDisappears = InMechData->MaxDelayBeforeIconDisappears;
		
		GetCapsuleComponent()->SetCapsuleRadius(InMechData->CapsuleRadius);
		GetCapsuleComponent()->SetCapsuleHalfHeight(InMechData->CapsuleHalfHeight);
		GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, InMechData->UpPosition));
		VrOriginComponent->SetRelativeLocation(FVector(0.f, 0.f, InMechData->UpPosition));
		VrOriginOffsetComponent->SetRelativeLocation(FVector(InMechData->OffsetForwardPosition, 0.f, InMechData->OffsetUpPosition));
		LineTracerHeadComponent->SetRelativeLocation(FVector(0.f, 0.f, InMechData->UpLineTracePosition));

		MinimapTaggedActor = InMechData->MinimapTaggedActor;
		
		MechInfoWidgetReference = InMechData->MechInfo;
		MechInfoWidgetComponent->SetRelativeLocation(InMechData->MechInfoPosition);
		MechInfoWidgetComponent->SetRelativeScale3D(InMechData->MechInfoInitialScale);
		MechInfoInitialScale = InMechData->MechInfoInitialScale;
		MechInfoMinimumScale = InMechData->MechInfoMinimumScale;
		MechInfoMaximumScale = InMechData->MechInfoMaximumScale;

		MechHeadIndicatorReference = InMechData->MechHeadIndicator;
		MechHeadIndicatorWidgetComponent->SetRelativeLocation(InMechData->MechHeadIndicatorPosition);
		MechHeadIndicatorWidgetComponent->SetRelativeScale3D(InMechData->MechHeadIndicatorInitialScale);
		MechHeadIndicatorInitialScale = InMechData->MechHeadIndicatorInitialScale;
		MechHeadIndicatorMinimumScale = InMechData->MechHeadIndicatorMinimumScale;
		MechHeadIndicatorMaximumScale = InMechData->MechHeadIndicatorMaximumScale;
		
		bCanAcquireMech = InMechData->bCanAcquireMech;
		AcquireHoldTime = InMechData->AcquireHoldTime;

		DestroyedMaterial = InMechData->DestroyedMaterial;

		MechExplosionOffset = InMechData->MechExplosionOffset;
		MechLeftArmExplosionOffset = InMechData->MechLeftArmExplosionOffset;
		MechRightArmExplosionOffset = InMechData->MechRightArmExplosionOffset;
		MechLeftLegExplosionOffset = InMechData->MechLeftLegExplosionOffset;
		MechRightLegExplosionOffset = InMechData->MechRightLegExplosionOffset;

		CockpitLeftArmExplosionOffset = InMechData->CockpitLeftArmExplosionOffset;
		CockpitRightArmExplosionOffset = InMechData->CockpitRightArmExplosionOffset;
		CockpitLeftLegExplosionOffset = InMechData->CockpitLeftLegExplosionOffset;
		CockpitRightLegExplosionOffset = InMechData->CockpitRightLegExplosionOffset;
		
		LeftLegMaterialIndex = InMechData->LeftLegMaterialIndex;
		RightLegMaterialIndex = InMechData->RightLegMaterialIndex;

		if(InMechData->MechFootStepsAudio == nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("ATitanZeroMech::ConfigureMech - MechFootStepsAudio null, game will crash"));
			UE_LOG(LogTemp, Error, TEXT("ATitanZeroMech::ConfigureMech - MechFootStepsAudio null, game will crash"));
		}
		else
		{
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParameters.Owner = this;
			SpawnParameters.Instigator = this;

			AAudio3dManagedObject* SpawnedMechFootStepsAudio = GetWorld()->SpawnActor<AAudio3dManagedObject>(InMechData->MechFootStepsAudio, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
			SpawnedMechFootStepsAudio->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, NAME_None);
		
			MechFootStepsAudio = SpawnedMechFootStepsAudio;
		}

		if(InMechData->MechJumpJetAudio == nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("ATitanZeroMech::ConfigureMech - MechJumpJetAudio null, game will crash"));
			UE_LOG(LogTemp, Error, TEXT("ATitanZeroMech::ConfigureMech - MechJumpJetAudio null, game will crash"));
		}
		else
		{
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParameters.Owner = this;
			SpawnParameters.Instigator = this;

			AAudio3dManagedObject* SpawnedMechJumpJetAudio = GetWorld()->SpawnActor<AAudio3dManagedObject>(InMechData->MechJumpJetAudio, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
			SpawnedMechJumpJetAudio->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, NAME_None);
			
			MechJumpJetAudio = SpawnedMechJumpJetAudio;
		}

		if(InMechData->CockpitFootStepsAudio == nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("ATitanZeroMech::ConfigureMech - CockpitFootStepsAudio null, game will crash"));
			UE_LOG(LogTemp, Error, TEXT("ATitanZeroMech::ConfigureMech - CockpitFootStepsAudio null, game will crash"));
		}
		else
		{
			CockpitFootStepsAudio = InMechData->CockpitFootStepsAudio;
		}
		
		if(InMechData->CockpitEngineAudio == nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("ATitanZeroMech::ConfigureMech - CockpitEngineAudio null, game will crash"));
			UE_LOG(LogTemp, Error, TEXT("ATitanZeroMech::ConfigureMech - CockpitEngineAudio null, game will crash"));
		}
		else
		{
			CockpitEngineAudio = InMechData->CockpitEngineAudio;
		}

		if(InMechData->CockpitRotationAudio == nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("ATitanZeroMech::ConfigureMech - CockpitRotationAudio null, game will crash"));
			UE_LOG(LogTemp, Error, TEXT("ATitanZeroMech::ConfigureMech - CockpitRotationAudio null, game will crash"));
		}
		else
		{
			CockpitRotationAudio = InMechData->CockpitRotationAudio;
		}
		
		if(InMechData->CockpitJumpJetAudio == nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("ATitanZeroMech::ConfigureMech - CockpitJumpJetAudio null, game will crash"));
			UE_LOG(LogTemp, Error, TEXT("ATitanZeroMech::ConfigureMech - CockpitJumpJetAudio null, game will crash"));
		}
		else
		{
			CockpitJumpJetAudio = InMechData->CockpitJumpJetAudio;
		}

		if(InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData != nullptr)
		{
			if(UTitanZeroWeaponDataAsset* RightArmWeaponDataAsset = InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData)
			{
				RightArmWeaponComponent = Cast<UTitanZeroWeaponComponent>(AddComponentByClass(RightArmWeaponDataAsset->WeaponComponent, true, GetTransform(), false));
				RightArmWeaponComponent->SetIsReplicated(false);
				RightArmWeaponComponent->Exec_InitializeWeapon(
															   this,
															   EWeaponAttachmentLocation::RightArm,
															   InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponSocketNames,
															   InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->WeaponType,
															   InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->Variant,
															   InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->AmmunitionType,
															   InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->MaximumAmmunition,
															   InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->AmmunitionPerTrigger,
															   InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->Damage,
															   InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->DamageMaxDistance,
															   InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->DamageFallOffDistance,
															   InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->HeatSinkDamage,
															   InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->HeatSinkCost,
															   InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->CoolDownTime,
															   InMechData->RightArmWeaponAttachmentConfigurationContext.WeaponData->WeaponAdditionalParameters);
			}
		}

		if(InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData != nullptr)
		{
			if(UTitanZeroWeaponDataAsset* LeftArmWeaponDataAsset = InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData)
			{
				LeftArmWeaponComponent = Cast<UTitanZeroWeaponComponent>(AddComponentByClass(LeftArmWeaponDataAsset->WeaponComponent, true, GetTransform(), false));
				LeftArmWeaponComponent->SetIsReplicated(false);
				LeftArmWeaponComponent->Exec_InitializeWeapon(
															  this,
															  EWeaponAttachmentLocation::LeftArm,
															  InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponSocketNames,
															  InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->WeaponType,
															  InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->Variant,
															  InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->AmmunitionType,
															  InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->MaximumAmmunition,
															  InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->AmmunitionPerTrigger,
															  InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->Damage,
															  InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->DamageMaxDistance,
															  InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->DamageFallOffDistance,
															  InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->HeatSinkDamage,
															  InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->HeatSinkCost,
															  InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->CoolDownTime,
															  InMechData->LeftArmWeaponAttachmentConfigurationContext.WeaponData->WeaponAdditionalParameters);
			}
		}

		if(InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData != nullptr)
		{
			if(UTitanZeroWeaponDataAsset* TorsoWeaponDataAsset = InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData)
			{
				TorsoWeaponComponent = Cast<UTitanZeroWeaponComponent>(AddComponentByClass(TorsoWeaponDataAsset->WeaponComponent, true, GetTransform(), false));
				TorsoWeaponComponent->SetIsReplicated(false);
				TorsoWeaponComponent->Exec_InitializeWeapon(
															this,
															EWeaponAttachmentLocation::Torso,
															InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponSocketNames,
															InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->WeaponType,
															InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->Variant,
															InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->AmmunitionType,
															InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->MaximumAmmunition,
															InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->AmmunitionPerTrigger,
															InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->Damage,
															InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->DamageMaxDistance,
															InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->DamageFallOffDistance,
															InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->HeatSinkDamage,
															InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->HeatSinkCost,
															InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->CoolDownTime,
															InMechData->TorsoWeaponAttachmentConfigurationContext.WeaponData->WeaponAdditionalParameters);
			}
		}
	}
}

void ATitanZeroMech::OnLeftGripTriggered(const FInputActionValue& Value)
{
	const float InputValue = Value.Get<float>();
	OnLeftGrip(InputValue);
}

void ATitanZeroMech::OnLeftGripCompleted(const FInputActionValue& Value)
{
	const float InputValue = Value.Get<float>();
	OnLeftGrip(InputValue);
}

void ATitanZeroMech::OnLeftThumbstickYAxisTriggered(const FInputActionValue& Value)
{
	const float InputValue = Value.Get<float>();
	OnLeftThumbstickYAxis(InputValue);
}

void ATitanZeroMech::OnLeftThumbstickYAxisCompleted(const FInputActionValue& Value)
{
	const float InputValue = Value.Get<float>();
	OnLeftThumbstickYAxis(InputValue);
}

void ATitanZeroMech::OnLeftThumbstickXAxisTriggered(const FInputActionValue& Value)
{
	const float InputValue = Value.Get<float>();
	OnLeftThumbstickXAxis(InputValue);
}

void ATitanZeroMech::OnLeftThumbstickXAxisCompleted(const FInputActionValue& Value)
{
	const float InputValue = Value.Get<float>();
	OnLeftThumbstickXAxis(InputValue);
}

void ATitanZeroMech::OnRightGripTriggered(const FInputActionValue& Value)
{
	const float InputValue = Value.Get<float>();
	OnRightGrip(InputValue);
}

void ATitanZeroMech::OnRightGripCompleted(const FInputActionValue& Value)
{
	const float InputValue = Value.Get<float>();
	OnRightGrip(InputValue);
}

void ATitanZeroMech::OnRightThumbstickYAxisTriggered(const FInputActionValue& Value)
{
	const float InputValue = Value.Get<float>();
	OnRightThumbstickYAxis(InputValue);
}

void ATitanZeroMech::OnRightThumbstickYAxisCompleted(const FInputActionValue& Value)
{
	const float InputValue = Value.Get<float>();
	OnRightThumbstickYAxis(InputValue);
}

void ATitanZeroMech::OnRightThumbstickXAxisTriggered(const FInputActionValue& Value)
{
	const float InputValue = Value.Get<float>();
	OnRightThumbstickXAxis(InputValue);
}

void ATitanZeroMech::OnRightThumbstickXAxisCompleted(const FInputActionValue& Value)
{
	const float InputValue = Value.Get<float>();
	OnRightThumbstickXAxis(InputValue);
}

void ATitanZeroMech::OnLookRotationTriggered(const FInputActionValue& Value)
{
	const FVector2D InputValue = Value.Get<FVector2D>();
	OnLookRotation(InputValue);
}

void ATitanZeroMech::OnLookRotationCompleted(const FInputActionValue& Value)
{
	const FVector2D InputValue = Value.Get<FVector2D>();
	OnLookRotation(InputValue);
}

void ATitanZeroMech::MechVerticalAxis(float Value)
{
	// If the mech controller is disabled, we return.
	if(bIsControllerDisabled) return;

	// If we are not using a stereo rendering, we just turn this on all the time.
	if(!bIsUsingStereoRendering) bIsPlayerHoldingLeftController = true;
	
	// If the player is not holding the left controller, we return.
	if(!bIsPlayerHoldingLeftController) return;
	
	float OutMechVerticalAxis = 0.f;

	if(Value > 0.8f) OutMechVerticalAxis = 1.f;
	else if(Value < -0.8f) OutMechVerticalAxis = -1.f;
	else OutMechVerticalAxis = 0.f;

	if(PreviousSpeed != OutMechVerticalAxis)
	{
		if(OutMechVerticalAxis != 0.f)
		{
			OnSpeedUpdated(1.f);
		}
		else
		{
			OnSpeedUpdated(0.f);
		}
		
		PreviousSpeed = OutMechVerticalAxis;
	}

	// If this is running on vr.
	if(bIsUsingStereoRendering)
	{
		// If player release virtual left controller.
		if(!bIsPlayerHoldingLeftController) OutMechVerticalAxis = 0.f;
	}
	
	if(const ATitanZeroPlayerController* TitanZeroPlayerController = Cast<ATitanZeroPlayerController>(Controller))
	{
		if(TitanZeroPlayerController->bIsOverheating) OutMechVerticalAxis = 0.f;
	}

	if(bIsTurning && OutMechVerticalAxis > 0.f) OnLeaningWhileMovingForward(RotationDirection);
	else OnLeaningWhileMovingForward(ERotationDirection::None);

	// We set the mech vertical velocity here.
	if(OutMechVerticalAxis == 1.f)
	{
		MechVerticalVelocity += GetWorld()->GetDeltaSeconds() * 0.5f;
		MechVerticalVelocityTimeElapsed = 0.f;
	}
	else if(OutMechVerticalAxis == -1.f)
	{
		MechVerticalVelocity -= GetWorld()->GetDeltaSeconds() * 0.5f;
		MechVerticalVelocityTimeElapsed = 0.f;
	}
	else
	{
		if(MechVerticalVelocity > 0.01f || MechVerticalVelocity < -0.01f)
		{
			if(MechVerticalVelocityTimeElapsed < MECH_VERTICAL_VELOCITY_LERP_DURATION)
			{
				MechVerticalVelocity = FMath::Lerp(MechVerticalVelocity, 0.f, MechVerticalVelocityTimeElapsed / MECH_VERTICAL_VELOCITY_LERP_DURATION);
				MechVerticalVelocityTimeElapsed += GetWorld()->GetDeltaSeconds() * 0.5f;
			}
		}
		else
		{
			// We force it to become 0 or else the velocity will still maintain small value that will still make the mech somewhat moving and animating.
			MechVerticalVelocity = 0.f;
		}
	}

	// We clamp it between -1 and 1.
	MechVerticalVelocity = FMath::Clamp(MechVerticalVelocity, -1.f, 1.f);

	// We set the speed based on direction.
	if(MechVerticalVelocity > 0.f)
	{
		GetCharacterMovement()->MaxWalkSpeed = ForwardSpeed;

		// We override if one or both of the legs are damaged.
		if(bIsOrAreLegsDamaged) GetCharacterMovement()->MaxWalkSpeed = DamageForwardSpeed;
	}
	else if(MechVerticalVelocity < 0.f)
	{
		GetCharacterMovement()->MaxWalkSpeed = BackwardSpeed;
		
		// We override if one or both of the legs are damaged.
		if(bIsOrAreLegsDamaged) GetCharacterMovement()->MaxWalkSpeed = DamageBackwardSpeed;
	}

	const FVector MechVerticalDirection = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::X);
	AddMovementInput(MechVerticalDirection, MechVerticalVelocity);
}

void ATitanZeroMech::MechTurnAxis(float Value)
{
	// If the mech controller is disabled, we return.
	if(bIsControllerDisabled) return;

	// If we are not using a stereo rendering, we just turn this on all the time.
	if(!bIsUsingStereoRendering) bIsPlayerHoldingLeftController = true;

	// If the player is not holding the left controller, we return.
	if(!bIsPlayerHoldingLeftController) return;
	
	float OutMechTurnAxis = 0.f;

	if(Value > 0.3f) OutMechTurnAxis = 1.f;
	else if(Value < -0.3f) OutMechTurnAxis = -1.f;
	else OutMechTurnAxis = 0.f;

	// If this is running on vr.
	if(bIsUsingStereoRendering)
	{
		// If player release virtual left controller.
		if(!bIsPlayerHoldingLeftController) OutMechTurnAxis = 0.f;
	}
	
	if(const ATitanZeroPlayerController* TitanZeroPlayerController = Cast<ATitanZeroPlayerController>(Controller))
	{
		if(TitanZeroPlayerController->bIsOverheating) OutMechTurnAxis = 0.f;
	}

	if(OutMechTurnAxis != 0.f)
	{
		Server_MechIsTurning(true);

		if(OutMechTurnAxis > 0.f) RotationDirection = ERotationDirection::Right;
		else if(OutMechTurnAxis < 0.f) RotationDirection = ERotationDirection::Left;
	}
	else
	{
		Server_MechIsTurning(false);
		RotationDirection = ERotationDirection::None;
	}

	const float TurnRate = OutMechTurnAxis / TurnSpeed;
	AddControllerYawInput(TurnRate);	
}

void ATitanZeroMech::MechJumpPressed()
{
	// If mech has no jump jet, we return.
	if(!bHasJumpJet) return;

	// If the mech controller is disabled, we return.
	if(bIsControllerDisabled) return;

	// if mech has no fuel, we return.
	if(JumpJetCurrentFuel <= 0.f) return;

	// We check if cool down is happening then we stop it to prevent consuming fuel while being regenerated.
	if(JumpJetCoolDownLoopTimerHandle.IsValid()) GetWorld()->GetTimerManager().ClearTimer(JumpJetCoolDownLoopTimerHandle);

	// Adjust character movement.
	GetCharacterMovement()->JumpZVelocity = MechJumpZVelocity;
	GetCharacterMovement()->AirControl = MechAirControl;
	GetCharacterMovement()->BrakingDecelerationFalling = MechBrakingDecelerationFalling;

	// We call this for blueprint to do logic like booster effect trigger, etc.
	OnJumpJetStart();

	// We start the jump jet fuel loop timer.
	GetWorld()->GetTimerManager().SetTimer(JumpJetLoopTimerHandle, this, &ATitanZeroMech::OnJumpJetLoopTimerHandle, 0.1f, true, 0.1f);

	// Is the jump jet in progress?
	if(bIsJumpJetInProgress)
	{
		Jump();
	}
	else
	{
		// We start the jump jet left delay.
		GetWorld()->GetTimerManager().SetTimer(JumpJetLiftDelayTimerHandle, this, &ATitanZeroMech::OnJumpJetLiftDelayCompleted, JumpJetLiftDelay, false, JumpJetLiftDelay);
	}
}

void ATitanZeroMech::MechJumpReleased()
{
	if(!bHasJumpJet) return;

	// We clear the jump jet loop timer.
	if(JumpJetLoopTimerHandle.IsValid()) GetWorld()->GetTimerManager().ClearTimer(JumpJetLoopTimerHandle);
	// We clear the lift delay timer.
	if(JumpJetLiftDelayTimerHandle.IsValid()) GetWorld()->GetTimerManager().ClearTimer(JumpJetLiftDelayTimerHandle);

	OnJumpJetStop();
	// We stop the jump regardless.
	StopJumping();
}

void ATitanZeroMech::OnToggleLockLookForVr()
{
	// If the player is not gripping
	if(!bIsPlayerGrippingRightController) return;
	bIsPlayerHoldingRightController = !bIsPlayerHoldingRightController;
}

void ATitanZeroMech::OnToggleLockLookForPc()
{
	bIsPlayerHoldingRightController = !bIsPlayerHoldingRightController;
}

void ATitanZeroMech::OnJumpJetLiftDelayCompleted()
{
	Jump();
	bIsJumpJetInProgress = true;
}

void ATitanZeroMech::OnJumpJetLoopTimerHandle()
{
	// If mech controller is disabled while mid air, we force this to stop.
	if(bIsControllerDisabled)
	{
		OnJumpJetStop();
		StopJumping();

		// We stop the jump jet loop timer update.
		GetWorld()->GetTimerManager().ClearTimer(JumpJetLoopTimerHandle);
	}
	
	JumpJetCurrentFuel -= GetWorld()->GetDeltaSeconds();
	JumpJetCurrentFuel = FMath::Clamp(JumpJetCurrentFuel, 0.f, JumpJetMaxFuel);

	// We also update the normalized value.
	JumpJetCurrentFuelNormalized = GetJumpJetFuelNormalizedValue();

	if(JumpJetCurrentFuel <= 0.f)
	{
		OnJumpJetStop();
		StopJumping();

		// We stop the jump jet loop timer update.
		GetWorld()->GetTimerManager().ClearTimer(JumpJetLoopTimerHandle);
	}
}

void ATitanZeroMech::OnJumpJetCoolDownLoopTimerHandle()
{
	JumpJetCurrentFuel += GetWorld()->GetDeltaSeconds();
	JumpJetCurrentFuel = FMath::Clamp(JumpJetCurrentFuel, 0.f, JumpJetMaxFuel);
	
	// We also update the normalized value.
	JumpJetCurrentFuelNormalized = GetJumpJetFuelNormalizedValue();

	// Adjust character movement.
	GetCharacterMovement()->JumpZVelocity = MechDefaultJumpZVelocity;
	GetCharacterMovement()->AirControl = MechDefaultAirControl;
	GetCharacterMovement()->BrakingDecelerationFalling = MechDefaultBrakingDecelerationFalling;

	if(JumpJetCurrentFuel >= JumpJetMaxFuel)
	{
		// We stop the cool down update timer.
		GetWorld()->GetTimerManager().ClearTimer(JumpJetCoolDownLoopTimerHandle);
	}
}

void ATitanZeroMech::OnHeatSinkBufferingBeforeCoolingDownTimerHandle()
{
	GetWorld()->GetTimerManager().ClearTimer(HeatSinkBufferingBeforeCoolingDown);
	GetWorld()->GetTimerManager().SetTimer(HeatSinkCoolingDownTimerHandle, this, &ATitanZeroMech::OnHeatSinkCoolingDownTimerHandle, 0.1f, true, 0.1f);
}

void ATitanZeroMech::OnHeatSinkCoolingDownTimerHandle()
{
	HeatSinkValue -= 5;
	bIsMechOverheatCoolingDown = true;

	// We clamp this between 0 - 100.
	HeatSinkValue = FMath::Clamp(HeatSinkValue, 0, 100);

	if(bIsOverheating)
	{
		if(HeatSinkValue >= 70)
		{
			bIsImmuneFromDamage = true;
		}
		else
		{
			HeatSinkVeryHotMeterCounter = HEAT_SINK_MIN_METER_COUNTER;			
			bIsImmuneFromDamage = false;
			Server_StopOverheating();
		}
	}
	
	if(HeatSinkValue <= 0)
	{
		bIsMechOverheatCoolingDown = false;
		GetWorld()->GetTimerManager().ClearTimer(HeatSinkCoolingDownTimerHandle);
	}
}

void ATitanZeroMech::OnControllerDisabledTimerHandle()
{
	GetWorld()->GetTimerManager().ClearTimer(ControllerDisabledTimerHandle);
	bIsControllerDisabled = false;
}

void ATitanZeroMech::OnWaitForLocalMechTimerHandle()
{
	if(const ATitanZeroGameState* TitanZeroGameState = GetWorld()->GetGameState<ATitanZeroGameState>())
	{
		bool bIsLocalMechExisting = false;

		if(GetWorld()->GetNetMode() == NM_Standalone)
		{
			for(const TObjectPtr<APlayerState>& ThisPlayer : TitanZeroGameState->PlayerArray)
			{
				if(!ThisPlayer->IsABot())
				{
					bIsLocalMechExisting = true;
					break;
				}
			}

			if(bIsLocalMechExisting)
			{
				if(WaitForLocalMechTimerHandle.IsValid()) GetWorld()->GetTimerManager().ClearTimer(WaitForLocalMechTimerHandle);

				// Here we start performing bot for single player or simulated proxy logic.
				FindLocalMechAndDetermineIfOpponentOrATeammate();
			}
		}
		else
		{
			// Let us iterate, wait, and find the local mech.
			for(const TObjectPtr<APlayerState>& ThisPlayer : TitanZeroGameState->PlayerArray)
			{
				if(ThisPlayer->GetPawn() == nullptr) continue;
				
				if(ThisPlayer->GetPawn()->GetLocalRole() == ROLE_AutonomousProxy)
				{
					bIsLocalMechExisting = true;
					break;
				}
			}

			if(bIsLocalMechExisting)
			{
				if(WaitForLocalMechTimerHandle.IsValid()) GetWorld()->GetTimerManager().ClearTimer(WaitForLocalMechTimerHandle);

				// Here we start performing bot for single player or simulated proxy logic.
				FindLocalMechAndDetermineIfOpponentOrATeammate();
			}
		}
	}
}

void ATitanZeroMech::FindLocalMechAndDetermineIfOpponentOrATeammate()
{
	if(const ATitanZeroGameState* TitanZeroGameState = GetWorld()->GetGameState<ATitanZeroGameState>())
	{
		if(GetWorld()->GetNetMode() == NM_Standalone)
		{
			// We first grab our own player state. We store this to a variable since we will use this later on.
			RemoteTitanZeroPlayerState = Cast<ATitanZeroPlayerState>(GetPlayerState());
		
			if(RemoteTitanZeroPlayerState != nullptr)
			{
				uint8 LocalMechTeamId = 0;
			
				// Let us iterate and find the local mech.
				for(const TObjectPtr<APlayerState>& ThisPlayer : TitanZeroGameState->PlayerArray)
				{
					if(!ThisPlayer->IsABot())
					{
						if(const ATitanZeroPlayerState* LocalMechPlayerState = Cast<ATitanZeroPlayerState>(ThisPlayer))
						{
							LocalMechTeamId = LocalMechPlayerState->TeamId;
							RemoteTitanZeroPlayerState->LocalMechReference = Cast<ATitanZeroMech>(ThisPlayer->GetPawn());
							break;
						}
					}
				}

				// Let us define that we are an opponent if we do not have the same team id as the local mech.
				if(LocalMechTeamId != RemoteTitanZeroPlayerState->TeamId) RemoteTitanZeroPlayerState->bIsOpponentOfLocalMech = true;
			}
		}
		else
		{
			// We first grab our own player state. We store this to a variable since we will use this later on.
			RemoteTitanZeroPlayerState = Cast<ATitanZeroPlayerState>(GetPlayerState());
		
			if(RemoteTitanZeroPlayerState != nullptr)
			{
				uint8 LocalMechTeamId = 0;
			
				// Let us iterate and find the local mech.
				for(const TObjectPtr<APlayerState>& ThisPlayer : TitanZeroGameState->PlayerArray)
				{
					if(ThisPlayer->GetPawn() == nullptr) continue;
					
					if(ThisPlayer->GetPawn()->GetLocalRole() == ROLE_AutonomousProxy)
					{
						if(const ATitanZeroPlayerState* LocalMechPlayerState = Cast<ATitanZeroPlayerState>(ThisPlayer))
						{
							LocalMechTeamId = LocalMechPlayerState->TeamId;
							RemoteTitanZeroPlayerState->LocalMechReference = Cast<ATitanZeroMech>(ThisPlayer->GetPawn());
							break;
						}
					}
				}

				// Let us define that we are an opponent if we do not have the same team id as the local mech.
				if(LocalMechTeamId != RemoteTitanZeroPlayerState->TeamId) RemoteTitanZeroPlayerState->bIsOpponentOfLocalMech = true;
			}
		}
	}
}

void ATitanZeroMech::OnUnlockMechTimerHandle()
{
	// We clear the timer.
	if(UnlockMechTimerHandle.IsValid()) GetWorld()->GetTimerManager().ClearTimer(UnlockMechTimerHandle);

	if(CurrentLockedTargetMech != nullptr)
	{
		if(CurrentLockedTargetMech->GetClass()->ImplementsInterface(ULockable::StaticClass()))
		{
			if(const ILockable* Lockable = Cast<ILockable>(CurrentLockedTargetMech)) Lockable->Execute_OnUnlockByLocalMech(CurrentLockedTargetMech);
		}
		// Then we clear the locked mech.
		CurrentLockedTargetMech = nullptr;
	}
	
	OnUnlocked_A_Mech();
}

void ATitanZeroMech::OnAttackingMechTimerHandleComplete()
{
	if(AttackingMechTimerHandle.IsValid()) GetWorld()->GetTimerManager().ClearTimer(AttackingMechTimerHandle);
	OnReceivedAttackingMech.Broadcast(nullptr);
}

bool ATitanZeroMech::IsRemoteMechVisibleToLocalMech()
{
	// @TODO: We must not forget to get the torso health from (If StandAlone or Autonomous, Player Controller) / (If Simulated, PlayerState) 
	if(GetWorld()->GetNetMode() == NM_Standalone)
	{
		// Since we know there is only 1 player, our approach is different.
		if(RemoteTitanZeroPlayerState != nullptr)
		{
			// This usually happens when we are a bot, so we keep searching if this player state is not a bot.
			if(RemoteTitanZeroPlayerState->IsABot())
			{
				if(RemoteTitanZeroPlayerState->LocalMechReference == nullptr) FindLocalMechAndDetermineIfOpponentOrATeammate();
			}

			if(RemoteTitanZeroPlayerState->LocalMechReference != nullptr)
			{
				FCollisionQueryParams CurrentParams = FCollisionQueryParams();
				CurrentParams.bTraceComplex = false;
				CurrentParams.AddIgnoredActor(this);
				CurrentParams.AddIgnoredActor(RemoteTitanZeroPlayerState->LocalMechReference);
				
				FHitResult CurrentHitResult(ForceInit);
				bool bIsHittingAnObject = false;
				
				// We have to check if something is blocking between this remote mech and the local mech.
				if(GetWorld()->LineTraceSingleByChannel(CurrentHitResult, LineTracerHeadComponent->GetComponentLocation(), RemoteTitanZeroPlayerState->LocalMechReference->GetActorLocation(), ECC_Visibility, CurrentParams)) bIsHittingAnObject = true;

				// ------------- This is for debugging only -------------
				if(!bIsHittingAnObject)
				{
					if(const UMaochiGamesSettings* MaochiGamesSettings = GetDefault<UMaochiGamesSettings>())
					{
						if(MaochiGamesSettings->bShowLineTracing) DrawDebugLine(GetWorld(), LineTracerHeadComponent->GetComponentLocation(), RemoteTitanZeroPlayerState->LocalMechReference->GetActorLocation(), FColor(0, 255, 0), false, -1, 0, MaochiGamesSettings->LineTraceThickness);
					}
				}
				else
				{
					if(const UMaochiGamesSettings* MaochiGamesSettings = GetDefault<UMaochiGamesSettings>())
					{
						if(MaochiGamesSettings->bShowLineTracing) DrawDebugLine(GetWorld(), LineTracerHeadComponent->GetComponentLocation(), RemoteTitanZeroPlayerState->LocalMechReference->GetActorLocation(), FColor(255, 0, 0), false, -1, 0, MaochiGamesSettings->LineTraceThickness);
					}
				}
				// ------------- End of debugging -------------
				
				// If there are no blocking object in the view.
				if(!bIsHittingAnObject)
				{
					// We compute the distance.
					const float DistanceToTheLocalMech = UTitanZeroHelperUtility::Exec_ActorGetDistanceInKilometers(RemoteTitanZeroPlayerState->LocalMechReference->GetActorLocation(), GetActorLocation());
					const bool bIsInValidDistance = DistanceToTheLocalMech <= MaxVisibilityDistance ? true : false;
				
					// Get the dot product of two vectors.
					const FVector SourceForwardVector = RemoteTitanZeroPlayerState->LocalMechReference->CockpitTargetRotationComponent->GetForwardVector();
					const FVector SourceLocation = RemoteTitanZeroPlayerState->LocalMechReference->CockpitTargetRotationComponent->GetComponentLocation();
					const FVector TargetLocation = LineTracerHeadComponent->GetComponentLocation();

					const float GivenDotProduct = UTitanZeroHelperUtility::Exec_GetAngleFromCenterScreen(SourceForwardVector, SourceLocation, TargetLocation);
				
					// Note: Screen angle is a normalized value between 0 and 1. Since center is 1, we use the dprod to calculate how far this remote mech is on the local mechs screen.
					//       I find 0.94 - 1.0 as a good value.
					if(GivenDotProduct >= RemoteTitanZeroPlayerState->LocalMechReference->MaxViewAngle && bIsInValidDistance)
					{
						//GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Green, FString::Printf(TEXT("Angle: %f, Distance: %f, Is In Valid Distance? %s"), DProd, DistanceToTheLocalMech, bIsInValidDistance ? TEXT("True"): TEXT("False")));
						return true;
					}
					
					return false;
				}
				
				return false;
			}

			return false;
		}
	}
	else
	{
		if(GetLocalRole() == ROLE_SimulatedProxy)
		{
			if(RemoteTitanZeroPlayerState != nullptr)
			{
				// This usually happens when we are a bot, so we keep searching for the autonomous player.
				if(RemoteTitanZeroPlayerState->IsABot())
				{
					if(RemoteTitanZeroPlayerState->LocalMechReference == nullptr) FindLocalMechAndDetermineIfOpponentOrATeammate();
				}

				if(RemoteTitanZeroPlayerState->LocalMechReference != nullptr)
				{
					if(RemoteTitanZeroPlayerState->LocalMechReference->GetPlayerState() != nullptr)
					{
						if(ATitanZeroPlayerState* LocalMechPlayerState = Cast<ATitanZeroPlayerState>(RemoteTitanZeroPlayerState->LocalMechReference->GetPlayerState()))
						{
							// If the local mech has been destroyed, we stop.
							if(LocalMechPlayerState->TorsoHealth <= 0) return false;
						}
					}
					
					FCollisionQueryParams CurrentParams = FCollisionQueryParams();
					CurrentParams.bTraceComplex = false;
					CurrentParams.AddIgnoredActor(this);
					CurrentParams.AddIgnoredActor(RemoteTitanZeroPlayerState->LocalMechReference);

					FHitResult CurrentHitResult(ForceInit);
					bool bIsHittingAnObject = false;

					// We have to check if something is blocking between this remote mech and the local mech.
					if(GetWorld()->LineTraceSingleByChannel(CurrentHitResult, LineTracerHeadComponent->GetComponentLocation(), RemoteTitanZeroPlayerState->LocalMechReference->GetActorLocation(), ECC_Visibility, CurrentParams)) bIsHittingAnObject = true;

					// ------------- This is for debugging only -------------
					if(!bIsHittingAnObject)
					{
						if(const UMaochiGamesSettings* MaochiGamesSettings = GetDefault<UMaochiGamesSettings>())
						{
							if(MaochiGamesSettings->bShowLineTracing) DrawDebugLine(GetWorld(), LineTracerHeadComponent->GetComponentLocation(), RemoteTitanZeroPlayerState->LocalMechReference->GetActorLocation(), FColor(0, 255, 0), false, -1, 0, MaochiGamesSettings->LineTraceThickness);
						}
					}
					else
					{
						if(const UMaochiGamesSettings* MaochiGamesSettings = GetDefault<UMaochiGamesSettings>())
						{
							if(MaochiGamesSettings->bShowLineTracing) DrawDebugLine(GetWorld(), LineTracerHeadComponent->GetComponentLocation(), RemoteTitanZeroPlayerState->LocalMechReference->GetActorLocation(), FColor(255, 0, 0), false, -1, 0, MaochiGamesSettings->LineTraceThickness);
						}
					}
					// ------------- End of debugging -------------
					
					// If there are no blocking object in the view.
					if(!bIsHittingAnObject)
					{
						// We compute the distance.
						const float DistanceToTheLocalMech = UTitanZeroHelperUtility::Exec_ActorGetDistanceInKilometers(RemoteTitanZeroPlayerState->LocalMechReference->GetActorLocation(), GetActorLocation());
						const bool bIsInValidDistance = DistanceToTheLocalMech <= MaxVisibilityDistance ? true : false;

						// Get the dot product of two vectors.
						const FVector SourceForwardVector = RemoteTitanZeroPlayerState->LocalMechReference->CockpitTargetRotationComponent->GetForwardVector();
						const FVector SourceLocation = RemoteTitanZeroPlayerState->LocalMechReference->CockpitTargetRotationComponent->GetComponentLocation();
						const FVector TargetLocation = LineTracerHeadComponent->GetComponentLocation();
						
						const float GivenDotProduct = UTitanZeroHelperUtility::Exec_GetAngleFromCenterScreen(SourceForwardVector, SourceLocation, TargetLocation);

						// Note: Screen angle is a normalized value between 0 and 1. Since center is 1, we use the dot product to calculate how far this remote mech is on the local mechs screen.
						//       I find 0.94 - 1.0 as a good value.
						if(GivenDotProduct >= RemoteTitanZeroPlayerState->LocalMechReference->MaxViewAngle && bIsInValidDistance)
						{
							//GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Green, FString::Printf(TEXT("Angle: %f, Distance: %f, Is In Valid Distance? %s"), DProd, DistanceToTheLocalMech, bIsInValidDistance ? TEXT("True"): TEXT("False")));
							return true;
						}
						
						return false;
					}

					return false;
				}

				return false;
			}
		}
	}

	return false;
}

float ATitanZeroMech::GetJumpJetFuelNormalizedValue()
{
	if(Controller != nullptr)
	{
		if(Controller->IsLocalPlayerController()) return JumpJetCurrentFuel / JumpJetMaxFuel;
	}

	return 0.f;
}

void ATitanZeroMech::OnRep_bIsTurning()
{
	//UE_LOG(LogTemp, Log, TEXT("ATitanZeroMech::OnRep_bIsTurning - bIsTurning updated: %s"), bIsTurning ? TEXT("True") : TEXT("False"));
}

void ATitanZeroMech::OnRep_bIsOverheating()
{
	//UE_LOG(LogTemp, Log, TEXT("ATitanZeroMech::OnRep_bIsOverheating - bIsOverheating updated: %s"), bIsOverheating ? TEXT("True") : TEXT("False"));

	if(bIsOverheating)
	{
		if(GetWorld()->GetNetMode() == NM_Standalone)
		{
			bIsControllerDisabled = true;
		}
		else
		{
			if(GetLocalRole() == ROLE_AutonomousProxy)
			{
				bIsControllerDisabled = true;
			}
		}
		
		OnStartOverheating();
	}
	else
	{
		if(GetWorld()->GetNetMode() == NM_Standalone)
		{
			GetWorld()->GetTimerManager().SetTimer(ControllerDisabledTimerHandle, this, &ATitanZeroMech::OnControllerDisabledTimerHandle, 4.5f, false, 4.5f);
		}
		else
		{
			if(GetLocalRole() == ROLE_AutonomousProxy)
			{
				GetWorld()->GetTimerManager().SetTimer(ControllerDisabledTimerHandle, this, &ATitanZeroMech::OnControllerDisabledTimerHandle, 4.5f, false, 4.5f);
			}
		}
		
		OnStopOverheating();
	}
}

void ATitanZeroMech::OnRep_HeatSinkValue()
{
	UE_LOG(LogTemp, Log, TEXT("ATitanZeroMech::OnRep_HeatSinkValue - Heatsink updated: %d"), HeatSinkValue);
}

void ATitanZeroMech::OnRep_HeatSinkVeryHotMeterCounter()
{
	UE_LOG(LogTemp, Log, TEXT("ATitanZeroMech::OnRep_HeatSinkVeryHotMeterCounter - Heatsink hot meter counter updated: %d"), HeatSinkVeryHotMeterCounter);
}

void ATitanZeroMech::OnRep_MechState()
{
	UE_LOG(LogTemp, Log, TEXT("ATitanZeroMech::OnRep_MechState - MechState updated: %d"), MechState);
	OnMechIsReady();
}

void ATitanZeroMech::OnRep_bIsJumpJetActivated()
{
	UE_LOG(LogTemp, Log, TEXT("ATitanZeroMech::OnRep_bIsJumpJetActivated - bIsJumpJetActivated updated: %s"), bIsJumpJetActivated ? TEXT("True") : TEXT("False"));
	OnIsJumpJetActivated(bIsJumpJetActivated);
}

void ATitanZeroMech::OnRep_LeftArmWeaponActivated()
{
	OnLeftArmWeaponsActivatedDispatcher.Broadcast(LeftArmWeaponActivated);
}

void ATitanZeroMech::OnRep_RightArmWeaponActivated()
{
	OnRightArmWeaponsActivatedDispatcher.Broadcast(RightArmWeaponActivated);
}

void ATitanZeroMech::OnRep_TorsoWeaponActivated()
{
	OnTorsoWeaponsActivatedDispatcher.Broadcast(TorsoWeaponActivated);
}
