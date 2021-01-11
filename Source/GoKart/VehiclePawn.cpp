// Fill out your copyright notice in the Description page of Project Settings.


#include "VehiclePawn.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AVehiclePawn::AVehiclePawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	NetUpdateFrequency = 1.0f;
}

// Called when the game starts or when spawned
void AVehiclePawn::BeginPlay()
{
	Super::BeginPlay();
	
}

void AVehiclePawn::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AVehiclePawn, ReplicatedLocation);
	DOREPLIFETIME(AVehiclePawn, ReplicatedRotation);
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

	if (HasAuthority())
	{
		ReplicatedLocation = GetActorLocation();
		ReplicatedRotation = GetActorRotation();
	}
	else
	{
		SetActorLocation(ReplicatedLocation);
		SetActorRotation(ReplicatedRotation);
	}

	DrawDebugString(GetWorld(), FVector(0, 0, 100), GetEnumText(Role), this, FColor::White, DeltaTime);

}

// Called to bind functionality to input
void AVehiclePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &AVehiclePawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AVehiclePawn::MoveRight);
}



void AVehiclePawn::OnRep_Location()
{
	UE_LOG(LogTemp, Warning, TEXT("Location Updated"));
}

void AVehiclePawn::MoveForward(float Value)
{
	Throttle = Value;
	Server_MoveForward(Value);
}

void AVehiclePawn::Server_MoveForward_Implementation(float Value)
{
	Throttle = Value;
}

bool AVehiclePawn::Server_MoveForward_Validate(float Value)
{
	return FMath::Abs(Value) <= 1.0f;
}

void AVehiclePawn::MoveRight(float Value)
{
	SteeringValue = Value;
	Server_MoveRight(Value);
}

void AVehiclePawn::Server_MoveRight_Implementation(float Value)
{
	SteeringValue = Value;
}

bool AVehiclePawn::Server_MoveRight_Validate(float Value)
{
	return FMath::Abs(Value) <= 1.0f;
}

void AVehiclePawn::UpdateRotation(float DeltaTime)
{
	auto sign = FMath::Sign(FVector::DotProduct(Velocity, GetActorForwardVector()));
	FQuat RotationDelta(GetActorUpVector(), sign* Velocity.Size() * DeltaTime * SteeringValue/TurningRadius);

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