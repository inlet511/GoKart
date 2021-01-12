// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "VehiclePawn.generated.h"

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

USTRUCT()
struct FVehiclePawnState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FVehiclePawnMove LastMove;

	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	FVector Velocity;
};


UCLASS()
class GOKART_API AVehiclePawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AVehiclePawn();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FVehiclePawnMove CreateMove(const float DeltaTime);

	void UpdateRotation( float DeltaTime,float Steering);	

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;



private:

	void SimulateMove(const FVehiclePawnMove& Move);

	UPROPERTY(ReplicatedUsing= OnRep_ServerState)
	FVehiclePawnState ServerState;

	UFUNCTION()
	void OnRep_ServerState();

	FVector Velocity;

	float Throttle;
	float SteeringValue;

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


	void MoveForward(float Value);
	void MoveRight(float Value);

	UFUNCTION(Server,Reliable,WithValidation)
	void Server_SendMove(FVehiclePawnMove Move);


	void UpdateLocationFromVelocity(float DeltaTime);

	TArray<FVehiclePawnMove> UnacknowledgedMoves;

	void ClearMoves(const FVehiclePawnMove& Move);
};
