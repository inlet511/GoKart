// Fill out your copyright notice in the Description page of Project Settings.


#include "VehicleMovement.h"
#include "Components/ActorComponent.h"
#include "GameFramework/GameStateBase.h"

// Sets default values for this component's properties
UVehicleMovement::UVehicleMovement()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UVehicleMovement::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UVehicleMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetOwnerRole() == ROLE_AutonomousProxy || (GetOwnerRole() == ROLE_Authority&& Cast<APawn>(GetOwner())->IsLocallyControlled()))
	{	
		LastMove =CreateMove(DeltaTime);
		SimulateMove(LastMove);
	}
}

FVehiclePawnMove UVehicleMovement::CreateMove(const float DeltaTime)
{
	FVehiclePawnMove Move;
	Move.DeltaTime = DeltaTime;
	Move.SteeringValue = SteeringValue;
	Move.Throttle = Throttle;
	Move.Time = GetOwner()->GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	return Move;
}


void UVehicleMovement::SimulateMove(const FVehiclePawnMove& Move)
{
	// Driving Force
	FVector Force = GetOwner()->GetActorForwardVector() * Move.Throttle * MaxDrivingForce;


	// Air Drag
	FVector AirDrag = -Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient;
	Force += AirDrag;

	// Rolling Drag
	if (Velocity.Size() > 0.01f)
	{
		float g = -GetOwner()->GetWorld()->GetGravityZ() * 0.01;
		FVector RollingDrag = -Velocity.GetSafeNormal() * RollingDragCoefficient * Mass * g;
		Force += RollingDrag;
	}

	// Acceleration
	FVector Acceleration = Force / Mass;

	Velocity = Velocity + Acceleration * Move.DeltaTime;

	UpdateRotation(Move.DeltaTime, Move.SteeringValue);

	UpdateLocationFromVelocity(Move.DeltaTime);
}

void UVehicleMovement::UpdateRotation(float DeltaTime, float Steering)
{
	auto sign = FMath::Sign(FVector::DotProduct(Velocity, GetOwner()->GetActorForwardVector()));
	FQuat RotationDelta(GetOwner()->GetActorUpVector(), sign* Velocity.Size() * DeltaTime * Steering / TurningRadius);

	Velocity = RotationDelta.RotateVector(Velocity);

	GetOwner()->AddActorWorldRotation(RotationDelta);
}

void UVehicleMovement::UpdateLocationFromVelocity(float DeltaTime)
{
	FVector Translation = Velocity * DeltaTime * 100;

	FHitResult Hit;
	GetOwner()->AddActorWorldOffset(Translation, true, &Hit);
	if (Hit.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
}