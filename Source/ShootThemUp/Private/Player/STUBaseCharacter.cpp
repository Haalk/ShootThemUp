// Shoot Them Up Game. All Rights Reserved.


#include "Player/STUBaseCharacter.h"
#include <Camera/CameraComponent.h>
#include <Components/InputComponent.h>
#include <GameFramework/SpringArmComponent.h>
#include <Components/STUCharacterMovementComponent.h>
#include <Components/STUHealthComponent.h>
#include <Components/TextRenderComponent.h>
#include <Components/STUWeaponComponent.h>
#include <GameFramework/Controller.h>

DEFINE_LOG_CATEGORY_STATIC(BaseCharacterLog, All, All);

// Sets default values
ASTUBaseCharacter::ASTUBaseCharacter(const FObjectInitializer& ObjInit)
    : Super(ObjInit.SetDefaultSubobjectClass<USTUCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArmComponent");
    SpringArmComponent->SetupAttachment(GetRootComponent());
    SpringArmComponent->bUsePawnControlRotation = true;
    SpringArmComponent->SocketOffset = FVector(0.0f, 100.0f, 80.0f);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
    CameraComponent->SetupAttachment(SpringArmComponent);

    HealthComponent = CreateDefaultSubobject<USTUHealthComponent>("HealthComponent");

    HealthTextComponent = CreateDefaultSubobject<UTextRenderComponent>("HealthTextComponent");
    HealthTextComponent->SetupAttachment(GetRootComponent());
    HealthTextComponent->SetOwnerNoSee(true);

    WeaponComponent = CreateDefaultSubobject<USTUWeaponComponent>("WeaponComponent");
}

// Called when the game starts or when spawned
void ASTUBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
    check(HealthComponent);
    check(HealthTextComponent);
    check(GetCharacterMovement());

    OnHealthChanged(HealthComponent->GetHealth());
    HealthComponent->OnDeadth.AddUObject(this, &ASTUBaseCharacter::OnDeath);
    HealthComponent->OnHealthChanged.AddUObject(this, &ASTUBaseCharacter::OnHealthChanged);

    LandedDelegate.AddDynamic(this, &ASTUBaseCharacter::OnGroundLanded);
}

// Called every frame
void ASTUBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ASTUBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
    check(PlayerInputComponent);
    check(WeaponComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASTUBaseCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ASTUBaseCharacter::MoveRight);
    PlayerInputComponent->BindAxis("LookUp", this, &ASTUBaseCharacter::AddControllerPitchInput);
    PlayerInputComponent->BindAxis("TurnAround", this, &ASTUBaseCharacter::AddControllerYawInput);
    PlayerInputComponent->BindAction("Jump", IE_Pressed,  this, &ASTUBaseCharacter::Jump);

    PlayerInputComponent->BindAction("Run", IE_Pressed, this, &ASTUBaseCharacter::OnStartRunning);
    PlayerInputComponent->BindAction("Run", IE_Released, this, &ASTUBaseCharacter::OnStopRunning);

    PlayerInputComponent->BindAction("Fire", IE_Pressed, WeaponComponent, &USTUWeaponComponent::Fire);
}

bool ASTUBaseCharacter::IsRunning() const
{
    return WantsToRun && IsMovingForward && !GetVelocity().IsZero();
}

float ASTUBaseCharacter::GetMovementDirection() const
{
    if (GetVelocity().IsZero())
    {
        return 0.0f;
    }

    const auto VelocityNormal = GetVelocity().GetSafeNormal();
    const auto AngleBetween = FMath::Acos(FVector::DotProduct(GetActorForwardVector(), VelocityNormal));
    const auto CrossProduct = FVector::CrossProduct(GetActorForwardVector(), VelocityNormal);

    const auto Degress = FMath::RadiansToDegrees(AngleBetween);

    return CrossProduct.IsZero() ? Degress : Degress * FMath::Sign(CrossProduct.Z);

    return 0.0f;
}

void ASTUBaseCharacter::MoveForward(float Amount) 
{
    IsMovingForward = Amount > 0.f;

    if (Amount == 0.0f)
    {
        return;
    }

    AddMovementInput(GetActorForwardVector(), Amount);
}

void ASTUBaseCharacter::MoveRight(float Amount) 
{
    if (Amount == 0.0f)
    {
        return;
    }

    AddMovementInput(GetActorRightVector(), Amount);
}

void ASTUBaseCharacter::OnStartRunning()
{
    WantsToRun = true;
    //GetCharacterMovement()->MaxWalkSpeed = 2300;
}

void ASTUBaseCharacter::OnStopRunning()
{
    WantsToRun = false;
    //GetCharacterMovement()->MaxWalkSpeed = 600;
}

void ASTUBaseCharacter::OnDeath() 
{
    UE_LOG(BaseCharacterLog, Display, TEXT("Player %s is dead"), *GetName());

    PlayAnimMontage(DeathAnimMontage);

    GetCharacterMovement()->DisableMovement();
    SetLifeSpan(LifeSpanOnDeath);

    if (Controller)
    {
        Controller->ChangeState(NAME_Spectating);
    }
}

void ASTUBaseCharacter::OnHealthChanged(float Health) 
{
    HealthTextComponent->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), Health)));
}

void ASTUBaseCharacter::OnGroundLanded(const FHitResult& Hit) 
{
    const auto FallVelocityZ = -GetVelocity().Z;
    UE_LOG(BaseCharacterLog, Display, TEXT("FallVelocity %f"), FallVelocityZ);

    if (FallVelocityZ < LandedDamageVelocity.X)
    {
        return;
    }

    const auto FinalDamage = FMath::GetMappedRangeValueClamped(LandedDamageVelocity, LandedDamage, FallVelocityZ);
    UE_LOG(BaseCharacterLog, Display, TEXT("FinalDamage %f"), FinalDamage);
    TakeDamage(FinalDamage, FDamageEvent{}, nullptr, nullptr);
}
