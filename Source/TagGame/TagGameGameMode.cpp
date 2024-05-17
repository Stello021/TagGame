// Copyright Epic Games, Inc. All Rights Reserved.

#include "TagGameGameMode.h"
#include "TagGameCharacter.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"
#include "EnemyAIController.h"


ATagGameGameMode::ATagGameGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	PrimaryActorTick.bCanEverTick = true;
}

void ATagGameGameMode::BeginPlay()
{
	Super::BeginPlay();

	ResetMatch();

}

void ATagGameGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	for (int32 i = 0; i < GameBalls.Num(); i++)
	{
		if (GameBalls[i]->GetAttachParentActor() != GetWorld()->GetFirstPlayerController()->GetPawn())
		{
			return;
		}
	}
	ResetMatch();

}

void ATagGameGameMode::ResetMatch()
{
	TargetPoints.Empty();
	GameBalls.Empty();

	for (TActorIterator<ATargetPoint> It(GetWorld()); It; ++It)
	{
		TargetPoints.Add(*It);
	}

	for (TActorIterator<ABall> It(GetWorld()); It; ++It)
	{
		if (It->GetAttachParentActor())
		{
			It->SetActorRelativeLocation(FVector::Zero());
			It->DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);
		}

		GameBalls.Add(*It);
	}

	TArray<ATargetPoint*> RandomTargetPoints = TargetPoints;

	for (int32 i = 0; i < GameBalls.Num(); i++)
	{
		const int32 RandomTargetIndex = FMath::RandRange(0, RandomTargetPoints.Num() - 1);
		GameBalls[i]->SetActorLocation(RandomTargetPoints[RandomTargetIndex]->GetActorLocation());
		RandomTargetPoints.RemoveAt(RandomTargetIndex);

	}

	for (TActorIterator<AEnemyAIController> It(GetWorld()); It; ++It)
	{
		It->BeginPlay();
	}
}

const TArray<ABall*>& ATagGameGameMode::GetBalls() const
{
	return GameBalls;
}


