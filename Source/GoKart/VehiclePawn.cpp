// Fill out your copyright notice in the Description page of Project Settings.


#include "VehiclePawn.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameStateBase.h"

// Sets default values
AVehiclePawn::AVehiclePawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
}

// Called when the game starts or when spawned
void AVehiclePawn::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		NetUpdateFrequency = 1.0f;
	}
}

void AVehiclePawn::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AVehiclePawn, ServerState);
}

FString GetEnumText(ENetRole Role)
{
	switch (Role)
	{
	case ROLE_None:
		return "None";		
	case ROLE_SimulatedProxy:
		return "Simulated";
	case ROLE_AutonomousProxy:
		return "AutonomousProxy";
	case ROLE_Authority:
		return "Authority";
	default:
		return "Error";
	}
}

// Called every frame
void AVehiclePawn::Tick(float DeltaTime)
{

	Super::Tick(DeltaTime);	

	if (IsLocallyControlled()) {
		FVehiclePawnMove Move = CreateMove(DeltaTime);
		if (!HasAuthority())
		{
			UnacknowledgedMoves.Add(Move);
			UE_LOG(LogTemp, Warning, TEXT("Queue Lenght:%d"), UnacknowledgedMoves.Num());
			Server_SendMove(Move);
			SimulateMove(Move);
		}
	}

	

	DrawDebugString(GetWorld(), FVector(0, 0, 100), GetEnumText(GetLocalRole()), this, FColor::White, DeltaTime);

}

FVehiclePawnMove AVehiclePawn::CreateMove(const float DeltaTime)
{
	FVehiclePawnMove Move;
	Move.DeltaTime = DeltaTime;
	Move.SteeringValue = SteeringValue;
	Move.Throttle = Throttle;
	Move.Time = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	return Move;
}

// Called to bind functionality to input
void AVehiclePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &AVehiclePawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AVehiclePawn::MoveRight);
}



void AVehiclePawn::SimulateMove(const FVehiclePawnMove& Move)
{
	// Driving Force
	FVector Force = GetActorForwardVector() * Move.Throttle * MaxDrivingForce;


	// Air Drag
	FVector AirDrag = -Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient;
	Force += AirDrag;

	// Rolling Drag
	if (Velocity.Size() > 0.01f)
	{
		float g = -GetWorld()->GetGravityZ() * 0.01;
		FVector RollingDrag = -Velocity.GetSafeNormal() * RollingDragCoefficient * Mass * g;
		Force += RollingDrag;
	}

	// Acceleration
	FVector Acceleration = Force / Mass;

	Velocity = Velocity + Acceleration * Move.DeltaTime;

	UpdateRotation(Move.DeltaTime,Move.SteeringValue);

	UpdateLocationFromVelocity(Move.DeltaTime);
}

void AVehiclePawn::OnRep_ServerState()
{
	SetActorTransform(ServerState.Transform);
	Velocity = ServerState.Velocity;
	ClearMoves(ServerState.LastMove);
	for (const FVehiclePawnMove& Move : UnacknowledgedMoves)
	{
		SimulateMove(Move);
	}
}



void AVehiclePawn::MoveForward(float Value)
{
	Throttle = Value;	
}

void AVehiclePawn::MoveRight(float Value)
{
	SteeringValue = Value;
}

void AVehiclePawn::Server_SendMove_Implementation(FVehiclePawnMove Move)
{
	SimulateMove(Move);
	ServerState.LastMove = Move;
	ServerState.Transform = GetActorTransform();
	ServerState.Velocity = Velocity;
}

bool AVehiclePawn::Server_SendMove_Validate(FVehiclePawnMove Move)
{
	return true; // TODO 
}


void AVehiclePawn::UpdateRotation(float DeltaTime, float Steering)
{
	auto sign = FMath::Sign(FVector::DotProduct(Velocity, GetActorForwardVector()));
	FQuat RotationDelta(GetActorUpVector(), sign* Velocity.Size() * DeltaTime * Steering /TurningRadius);

	Velocity = RotationDelta.RotateVector(Velocity);

	AddActorWorldRotation(RotationDelta);
}

void AVehiclePawn::UpdateLocationFromVelocity(float DeltaTime)
{
	FVector Translation = Velocity * DeltaTime * 100;

	FHitResult Hit;
	AddActorWorldOffset(Translation, true, &Hit);
	if (Hit.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
}

void AVehiclePawn::ClearMoves(const FVehiclePawnMove& Move)
{
	TArray<FVehiclePawnMove> NewMoves;

	for (const FVehiclePawnMove& item : UnacknowledgedMoves)
	{
		if (item.Time > Move.Time)
		{
			NewMoves.Add(item);
		}
	}

	UnacknowledgedMoves = NewMoves;
}
