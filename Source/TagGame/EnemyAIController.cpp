// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "TagGameGameMode.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/BlackboardComponent.h"


void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();

	BlackboardAsset = NewObject<UBlackboardData>();
	BlackboardAsset->UpdatePersistentKey<UBlackboardKeyType_Object>(TEXT("NearestBall"));

	UseBlackboard(BlackboardAsset, BlackboardComponent);

	GoToPlayer = MakeShared<FAIVState>(
		//Enter
		[](AAIController* AIController) {
			AIController->MoveToActor(AIController->GetWorld()->GetFirstPlayerController()->GetPawn(), 100.0f);
		},
		nullptr, //Exit
		//Tick
		[this](AAIController* AIController, const float DeltaTime) -> TSharedPtr<FAIVState> {
			EPathFollowingStatus::Type State = AIController->GetMoveStatus();

			if (State == EPathFollowingStatus::Moving)
			{

				return nullptr;
			}
			if (BestBall)
			{
				BestBall->SetActorRelativeLocation(FVector::Zero());
				BestBall->AttachToActor(AIController->GetWorld()->GetFirstPlayerController()->GetPawn(), FAttachmentTransformRules::KeepRelativeTransform);

				BestBall = nullptr;
			}
			return SearchForBall;
		}
	);

	SearchForBall = MakeShared<FAIVState>(
		//Enter
		[this](AAIController* AIController) {
			GetNearestBall(AIController);
		},
		nullptr, //Exit
		//Tick
		[this](AAIController* AIController, const float DeltaTime) -> TSharedPtr<FAIVState> {
			if (BestBall)
			{
				return GoToBall;
			}
			else
			{
				return SearchForBall;
			}
		}
	);

	GoToBall = MakeShared<FAIVState>(
		//Enter
		[this](AAIController* AIController) {
			AIController->MoveToActor(BestBall, 100.0f);
		},
		nullptr, //Exit
		//Tick
		[this](AAIController* AIController, const float DeltaTime) -> TSharedPtr<FAIVState> {
			EPathFollowingStatus::Type State = AIController->GetMoveStatus();

			if (State == EPathFollowingStatus::Moving)
			{
				return nullptr;
			}

			return GrabBall;
		}
	);

	GrabBall = MakeShared<FAIVState>(
		//Enter
		[this](AAIController* AIController)
		{
			if (BestBall->GetAttachParentActor())
			{
				BestBall = nullptr;
			}
		},
		nullptr, //Exit
		//Tick
		[this](AAIController* AIController, const float DeltaTime) -> TSharedPtr<FAIVState> {

			if (!BestBall)
			{
				return SearchForBall;
			}

			BestBall->SetActorRelativeLocation(FVector::Zero());
			BestBall->AttachToActor(AIController->GetPawn(), FAttachmentTransformRules::KeepRelativeTransform);

			return GoToPlayer;
		}
	);

	ResetStateMachine();

}

void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState)
	{
		CurrentState = CurrentState->CallTick(this, DeltaTime);

	}

}

void AEnemyAIController::GetNearestBall(AAIController* AIController)
{
	AGameModeBase* GameMode = AIController->GetWorld()->GetAuthGameMode();
	ATagGameGameMode* AIGameMode = Cast<ATagGameGameMode>(GameMode);
	const TArray<ABall*>& BallsList = AIGameMode->GetBalls();

	ABall* NearestBall = nullptr;

	for (int32 i = 0; i < BallsList.Num(); i++)
	{
		if ((!BallsList[i]->GetAttachParentActor()) &&
			(!NearestBall ||
				FVector::Distance(AIController->GetPawn()->GetActorLocation(), BallsList[i]->GetActorLocation()) <
				FVector::Distance(AIController->GetPawn()->GetActorLocation(), NearestBall->GetActorLocation())))
		{
			NearestBall = BallsList[i];
		}
	}
	BlackboardComponent->SetValueAsObject("NearestBall", NearestBall);
	BestBall = Cast<ABall>(BlackboardComponent->GetValueAsObject("NearestBall"));
}

void AEnemyAIController::ResetStateMachine()
{
	CurrentState = SearchForBall;
	CurrentState->CallEnter(this);
}
