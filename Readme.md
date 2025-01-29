# Tag Game

A third-person tag game implemented in Unreal Engine 5 using C++. Players compete against AI-controlled characters to collect balls scattered around the arena.

## Game Overview

In this game, players navigate a 3D environment competing against AI opponents to collect balls. The game emphasizes quick movement, strategic thinking, and competitive gameplay.

### Core Mechanics

- Balls spawn at random target points in the level
- AI opponents actively seek and collect balls
- The round resets when all balls are collected by either the player or AI

## Technical Implementation

### Character System

The game uses UE5's character system with enhanced input for modern control handling. 

### AI Implementation

The AI system uses a custom state machine for sophisticated behavior control. The state machine is implemented using modern C++ features including lambdas and shared pointers:

```cpp
// EnemyAIController.h
struct FAIVState : public TSharedFromThis<FAIVState>
{
    TFunction<void(AAIController*)> Enter;
    TFunction<void(AAIController*)> Exit;
    TFunction<TSharedPtr<FAIVState>(AAIController*, const float)> Tick;

    // State transition handling
    TSharedPtr<FAIVState> CallTick(AAIController* AIController, const float DeltaTime)
    {
        if (Tick)
        {
            TSharedPtr<FAIVState> NewState = Tick(AIController, DeltaTime);
            if (NewState != nullptr && NewState != AsShared())
            {
                CallExit(AIController);
                NewState->CallEnter(AIController);
                return NewState;
            }
        }
        return AsShared();
    }
};
```

The AI controller implements different states for complex behavior:

```cpp
// EnemyAIController.cpp
void AEnemyAIController::BeginPlay()
{
      Super::BeginPlay();

    //Set up Blackboard
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
    // SearchForBall state implementation
    SearchForBall = MakeShared<FAIVState>(
        // Enter function
        [this](AAIController* AIController) {
            GetNearestBall(AIController);
        },
        nullptr, // Exit function
        // Tick function
        [this](AAIController* AIController, const float DeltaTime) -> TSharedPtr<FAIVState> {
            return BestBall ? GoToBall : SearchForBall;
        }
    );

    // Ball collection state
    GrabBall = MakeShared<FAIVState>(
        [this](AAIController* AIController) {
            if (BestBall->GetAttachParentActor()) {
                BestBall = nullptr;
            }
        },
        nullptr,
        [this](AAIController* AIController, const float DeltaTime) -> TSharedPtr<FAIVState> {
            if (!BestBall) return SearchForBall;

            BestBall->AttachToActor(AIController->GetPawn(), 
                FAttachmentTransformRules::KeepRelativeTransform);
            return GoToPlayer;
        }
    );
    ResetStateMachine();
}
```

### Game Mode Implementation

The game mode manages the core game loop, including ball spawning and victory conditions:

```cpp
// TagGameGameMode.cpp
void ATagGameGameMode::ResetMatch()
{
    // Clear existing game state
    TargetPoints.Empty();
    GameBalls.Empty();

    // Collect all target points and balls
    for (TActorIterator<ATargetPoint> It(GetWorld()); It; ++It)
    {
        TargetPoints.Add(*It);
    }

    // Reset ball attachments and positions
    for (TActorIterator<ABall> It(GetWorld()); It; ++It)
    {
        if (It->GetAttachParentActor())
        {
            It->DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);
        }
        GameBalls.Add(*It);
    }

    // Randomly distribute balls to target points
    TArray<ATargetPoint*> RandomTargetPoints = TargetPoints;
    for (int32 i = 0; i < GameBalls.Num(); i++)
    {
        const int32 RandomTargetIndex = FMath::RandRange(0, RandomTargetPoints.Num() - 1);
        GameBalls[i]->SetActorLocation(RandomTargetPoints[RandomTargetIndex]->GetActorLocation());
        RandomTargetPoints.RemoveAt(RandomTargetIndex);
    }
}
```

### Key Implementation Points

1. **State Machine Pattern**: The AI controller implements a flexible state machine using modern C++ features:
   
   - States are implemented as lambdas for clean, encapsulated behavior
   - Shared pointers manage state lifetime
   - Each state has Enter, Exit, and Tick functions

2. **Ball Management System**: 
   
   - Balls can be attached to both AI and player characters
   - Random distribution system ensures fair ball placement
   - Automatic round reset when all balls are collected
