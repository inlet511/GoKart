// Fill out your copyright notice in the Description page of Project Settings.


#include "VehiclePawn.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"

// Sets default values
AVehiclePawn::AVehiclePawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AVehiclePawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AVehiclePawn::Tick(float DeltaTime)
{

	Super::Tick(DeltaTime);	

	// Driving Force
	FVector Force = GetActorForwardVector() * Throttle * MaxDrivingForce;


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

	Velocity = Velocity + Acceleration * DeltaTime;

	

	UpdateRotation(DeltaTime);

	UpdateLocationFromVelocity(DeltaTime);

}

// Called to bind functionality to input
void AVehiclePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &AVehiclePawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AVehiclePawn::MoveRight);
}

void AVehiclePawn::MoveForward(float Value)
{
	Throttle = Value;
}

void AVehiclePawn::MoveRight(float Value)
{
	SteeringValue = Value;
}

void AVehiclePawn::UpdateRotation(float DeltaTime)
{
	FQuat RotationDelta(GetActorUpVector(), FMath::DegreesToRadians(DeltaTime * MaxRotateSpeed * SteeringValue));

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