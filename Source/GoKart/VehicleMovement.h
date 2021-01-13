// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VehicleMovement.generated.h"

USTRUCT()
struct FVehiclePawnMove
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	float Throttle;

	UPROPERTY()
	float SteeringValue;

	UPROPERTY()
	float DeltaTime;

	UPROPERTY()
	float Time;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GOKART_API UVehicleMovement : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UVehicleMovement();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:

	void SimulateMove(const FVehiclePawnMove& Move);

	FVehiclePawnMove CreateMove(const float DeltaTime);

	FVector GetVelocity() { return Velocity; }

	void SetVelocity(FVector Value) { Velocity = Value; }

	void SetThrottle(float Value) { Throttle = Value; }

	void SetSteering(float Value) { SteeringValue = Value; }

private:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void UpdateRotation(float DeltaTime, float Steering);

	UPROPERTY(EditAnywhere)
	float Mass = 1000;

	UPROPERTY(EditAnywhere)
	float MaxDrivingForce = 10000;

	UPROPERTY(EditAnywhere)
	float DragCoefficient = 16;

	UPROPERTY(EditAnywhere)
	float RollingDragCoefficient = 0.1;

	UPROPERTY(EditAnywhere)
	float TurningRadius = 10.0f;

	void UpdateLocationFromVelocity(float DeltaTime);

	FVector Velocity;

	float Throttle;
	float SteeringValue;
};