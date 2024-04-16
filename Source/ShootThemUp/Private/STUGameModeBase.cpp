// Shoot Them Up Game. All Rights Reserved.

#include "STUGameModeBase.h"
#include "Player/STUBaseCharacter.h"
#include "Player/STUPlayerController.h"
#include "UI/STUGameHUD.h"

ASTUGameModeBase::ASTUGameModeBase()
{
    DefaultPawnClass = ASTUBaseCharacter::StaticClass();
    PlayerControllerClass = ASTUPlayerController::StaticClass();
    HUDClass = ASTUGameHUD::StaticClass();
}

void ASTUGameModeBase::BeginPlay()
{
    int32 Variable = 0;
    if (Variable < 10)
    {
        for (int32 i = 0; i < 10; ++i)
        {
            UE_LOG(LogTemp, Display, TEXT("index: %i"), i);
        }
    }
}
