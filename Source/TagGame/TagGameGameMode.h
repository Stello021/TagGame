// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/TargetPoint.h"
#include "Ball.h"
#include "TagGameGameMode.generated.h"

UCLASS(minimalapi)
class ATagGameGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATagGameGameMode();
	void BeginPlay() override;
	void Tick(float DeltaTime) override;
	void ResetMatch();
	const TArray<ABall*>& GetBalls() const;

protected:
	TArray<ATargetPoint*> TargetPoints;
	TArray<ABall*> GameBalls;

};



