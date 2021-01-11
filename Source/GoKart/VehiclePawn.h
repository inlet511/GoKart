// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "VehiclePawn.generated.h"

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

	void UpdateRotation( float DeltaTime);	

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;



private:
	FVector Velocity;

	UPROPERTY(ReplicatedUsing=OnRep_Location)
	FVector ReplicatedLocation;

	UFUNCTION()
	void OnRep_Location();

	UPROPERTY(Replicated)
	FRotator ReplicatedRotation;

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

	UFUNCTION(Server,Reliable,WithValidation)
	void Server_MoveForward(float Value);


	void MoveRight(float Value);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_MoveRight(float Value);

	void UpdateLocationFromVelocity(float DeltaTime);
};
