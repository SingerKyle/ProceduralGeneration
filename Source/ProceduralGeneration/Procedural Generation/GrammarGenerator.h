#pragma once

#include "Floor.h"               
#include "LevelGenerator.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrammarGenerator.generated.h"

struct GrammarRule
{
    TArray<EPlatformPlacementCategory> Expansions;
};

struct FPlatformCalculations
{
    FVector MinBounds;
    FVector MaxBounds;
    FVector PlatformStart;
    FVector PlatformSize;
    float PlatformWidth;
    float PlatformDepth;
    bool bSpawnAlongX;
};

UENUM(BlueprintType)
enum class EPlacementDirection : uint8
{
    Forward   UMETA(DisplayName = "Forward"),
    Backward  UMETA(DisplayName = "Backward"),
    Left      UMETA(DisplayName = "Left"),
    Right     UMETA(DisplayName = "Right")

    
};

UENUM(BlueprintType)
enum class EPlatformPlacementCategory : uint8
{
    HorizontalForward  UMETA(DisplayName = "Beside Forward"),
    HorizontalLeft    UMETA(DisplayName = "Beside Left"),
    HorizontalRight   UMETA(DisplayName = "Beside Right"),
    HorizontalBack    UMETA(DisplayName = "Beside Back"),
    
    SmallJumpForward UMETA(DisplayName = "Small Jump Forward"),
    SmallJumpLeft    UMETA(DisplayName = "Small Jump Left"),
    SmallJumpRight   UMETA(DisplayName = "Small Jump Right"),
    SmallJumpBack    UMETA(DisplayName = "Small Jump Back"),

    LongJumpForward UMETA(DisplayName = "Long Jump Forward"),
    LongJumpLeft    UMETA(DisplayName = "Long Jump Left"),
    LongJumpRight   UMETA(DisplayName = "Long Jump Right"),
    LongJumpBack    UMETA(DisplayName = "Long Jump Back"),

    AboveForward UMETA(DisplayName = "Above Forward"),
    AboveLeft    UMETA(DisplayName = "Above Left"),
    AboveRight   UMETA(DisplayName = "Above Right"),
    AboveBack    UMETA(DisplayName = "Above Back"),

    BelowForward UMETA(DisplayName = "Below Forward"),
    BelowLeft    UMETA(DisplayName = "Below Left"),
    BelowRight   UMETA(DisplayName = "Below Right"),
    BelowBack    UMETA(DisplayName = "Below Back"),

    VeryHighPoint UMETA(DisplayName = "Very High Point"),
    VeryLowPoint  UMETA(DisplayName = "Very Low Point")
};

// Struct to store the edges of a platform
struct FPlatformEdges
{
    // Red Sphere In Level
    FVector TopLeftCoord;
    
    // Orange Sphere In Level
    FVector BottomLeftCoord;
    
    // Yellow Sphere In Level
    FVector TopRightCoord;
    
    // Blue Sphere In Level
    FVector BottomRightCoord;
    
    FVector Centre;

    FPlatformEdges()
        : TopLeftCoord(FVector::ZeroVector)
        , BottomLeftCoord(FVector::ZeroVector)
        , TopRightCoord(FVector::ZeroVector)
        , BottomRightCoord(FVector::ZeroVector)
        , Centre(FVector::ZeroVector)
    {}

    FPlatformEdges(const FVector& InTopLeftFront, const FVector& InTopRightFront, const FVector& InTopLeftBack, const FVector& InTopRightBack, const FVector& Centre)
        : BottomLeftCoord(InTopLeftBack)
        , TopLeftCoord(InTopRightBack)
        , TopRightCoord(InTopRightFront)
        , BottomRightCoord(InTopLeftFront)
        , Centre(Centre)
    {}
    
};

USTRUCT(BlueprintType)
struct FDecorateLevelRules
{
    GENERATED_BODY()
    
    // How many building Rows
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NumLayers = 3;
    
    // Distance between each layer
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LayerSpacing = 7500.f; 

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BufferDistance = 8000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpawnHeight = 5000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2f SpawnScale = FVector2f(5,15);
    
};

USTRUCT(BlueprintType)
struct FGrammarRules
{
    GENERATED_BODY()

    // Grammar Spawning Settings - Platforms
    // Total number of platforms to generate.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform Parameters")
    int32 NumPlatforms = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform Parameters", meta = (DisplayName = "Platform Size Minimum")) FVector PlatformScale = FVector(10.f,65.f, 60.f);

    // Grid unit (each 1x1 cell = 1000 units)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform Parameters")
    int32 GridUnit = 50.0f;

    // The Mesh to spawn for each platform.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform Parameters")
    TArray<UStaticMesh*> PlatformMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform Parameters")
    UMaterialInterface* StartPlatformMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform Parameters")
    UMaterialInterface* FinishPlatformMaterial = nullptr;

    // Grammar Spawning Settings - Obstacles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Parameters")
    UStaticMesh* WallRunMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Parameters")
    UMaterialInterface* ObstacleMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Parameters", meta = (DisplayName = "Wallrun Wall Height"))
    int WallRunHeight = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Parameters")
    UStaticMesh* MantleMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Parameters")
    TSubclassOf<AActor> VaultMesh = nullptr;
    
    // Grammar Spawning settings - Distances

    // Above
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Parameters", meta = (DisplayName = "Above Gap Minimum")) int AboveGapMinimum = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Parameters", meta = (DisplayName = "Above Gap Maximum")) int AboveGapMaximum = 100;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Parameters", meta = (DisplayName = "Above Jump Height Minimum")) int AboveHeightMin = 120;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Parameters", meta = (DisplayName = "Above Jump Height Maximum")) int AboveHeightMax = 220;
    // Below                                                                                                
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Parameters", meta = (DisplayName = "Below Gap Minimum")) int BelowGapMinimum = 200;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Parameters", meta = (DisplayName = "Below Gap Minimum")) int BelowGapMaximum = 450;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Parameters", meta = (DisplayName = "Below Jump Height Minimum")) int BelowHeightMin = 70;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Parameters", meta = (DisplayName = "Below Jump Height Maximum")) int BelowHeightMax = 150;
    // Short Jumps                     
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Parameters", meta = (DisplayName = "Small Jump Minimum Value")) int SmallJumpMinimum = 200;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Parameters", meta = (DisplayName = "Small Jump Maximum Value")) int SmallJumpMaximum = 450;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Parameters", meta = (DisplayName = "Small Jump Height"))        int SmallJumpHeight = 100;
    // Long Jumps                 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Parameters", meta = (DisplayName = "Long Jump Minimum Value")) int LongJumpMinimum = 800;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Parameters", meta = (DisplayName = "Long Jump Maximum Value")) int LongJumpMaximum = 1500;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Parameters", meta = (DisplayName = "Long Jump Height"))        int LongJumpHeight = 100;
};

UCLASS()
class PROCEDURALGENERATION_API AGrammarGenerator : public AActor
{
    GENERATED_BODY()

public:
    AGrammarGenerator();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    void ClearLevel();
    // Called in-editor or at runtime to generate the level.
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Level Generation")
    void GenerateLevel();

    EPlacementDirection GetOppositeDirection(EPlacementDirection Dir);

    FPlatformCalculations CalculatePlatformProperties(const FPlatformEdges& PlatformEdges);

    inline TArray<AActor*> GetPlacedPlatforms() { return PlacedPlatforms; };

protected:
    // Grammar-based functions

    /** Spawns the first platform and then, using grammar rules, places subsequent platforms. */
    void GeneratePlatformChain();
    void PopulateWorld();

    // Functions for spawning in decorations
    void SpawnBuilding(const FVector& Location);

    // Chooses the next grammar rule depending on available options
    FString PickNextRule(EPlatformPlacementCategory Category);
    
    // recursively calls this function to spawn platforms according to the next rule
    void ExpandRule(const FString& Rule, int32 RemainingPlatforms);
    
    void CalculateClosestEdges(const FPlatformEdges& OldEdges, const FPlatformEdges& NewEdges, EPlatformPlacementCategory Category, FVector&
                               OutOldEdgeStart, FVector& OutOldEdgeEnd, FVector& OutNewEdgeStart, FVector& OutNewEdgeEnd);
    
    FVector CalculateWallRunLocation(const FPlatformEdges& OldEdges, const FPlatformEdges& NewEdges, EPlatformPlacementCategory Category);
    
    FRotator CalculateWallRunRotation(const FPlatformEdges& OldEdges, const FPlatformEdges& NewEdges, EPlatformPlacementCategory Category);
    
    void SpawnWallRunObstacle(const FVector& Vector, const FRotator& Rotator, const float& Distance);
    
    float CalculateWallRunDistance(const FPlatformEdges& OldEdges, const FPlatformEdges& NewEdges, EPlatformPlacementCategory Category);
    
    void SpawnMantleObstacle(const FVector& Vector, const FRotator& Rotator);
    
    void SpawnMantleWalls(const FPlatformEdges& PlatformEdges);
    
    void SpawnVaultObstacle(const FVector& Vector, const FRotator& Rotator, const float& Distance);
    
    void SpawnVaultObstacles(const FPlatformEdges& PlatformEdges);
    
    void SpawnMantleStaircase(const FPlatformEdges& OldEdges, const FPlatformEdges& NewEdges, EPlatformPlacementCategory Category, int32 StepCount);
    
    int32 HandleMantleSpawns(const FPlatformEdges& OldEdges, const FPlatformEdges& NewEdges);
    
    void SpawnObstaclesForCategory(EPlatformPlacementCategory NextCategory, const FPlatformEdges& OldEdges, const FPlatformEdges& NewEdges);
    
    EPlacementDirection ConvertToPlacementDirection(EPlatformPlacementCategory Category);
    
    auto CalculateOffsetForDirection(EPlacementDirection Direction, const FVector& CurrentScale,
                                   const FVector& NewScale) const -> FVector;
    
    FRotator CalculateRotationForDirection(EPlacementDirection Direction) const;

    /** Calculate a relative offset (in world units) based on the chosen category. */
    FVector CalculateOffsetForCategory(EPlatformPlacementCategory Category) const;

    /** Helper to spawn a platform at a given location. */
    void SpawnPlatform(const FVector& Location, const FVector& Scale, const FRotator& Rotation, const FString& PlatformName);
    
    FVector SnapToGrid(const FVector& Location) const;

    /** For debugging, draws a text label at a location. */
    void DrawDebugLabel(const FString& Text, const FVector& Location) const;

    FPlatformEdges CalculatePlatformEdges(const FVector& Location, const FVector& Scale, const FRotator& Rotation) const;

    TMap<FString, GrammarRule> GrammarRules = {
    { "StartRule", {
        {
            // Forward
            EPlatformPlacementCategory::SmallJumpForward,
            EPlatformPlacementCategory::HorizontalForward,
            EPlatformPlacementCategory::AboveForward,
            EPlatformPlacementCategory::BelowForward,

            EPlatformPlacementCategory::SmallJumpLeft,
            EPlatformPlacementCategory::HorizontalLeft,
            EPlatformPlacementCategory::AboveLeft,
            EPlatformPlacementCategory::BelowLeft,

            EPlatformPlacementCategory::SmallJumpRight,
            EPlatformPlacementCategory::HorizontalRight,
            EPlatformPlacementCategory::AboveRight,
            EPlatformPlacementCategory::BelowRight,
            EPlatformPlacementCategory::VeryHighPoint
        } } },
    { "BesideRule", {
        {
            // Forward
            EPlatformPlacementCategory::HorizontalForward,
            // Left
            EPlatformPlacementCategory::HorizontalLeft,
            // Right
            EPlatformPlacementCategory::HorizontalRight
        } } },
    { "AboveRule", { {
        // Forward
        EPlatformPlacementCategory::AboveForward,
        EPlatformPlacementCategory::SmallJumpForward,
        // Left
        EPlatformPlacementCategory::AboveLeft,
        EPlatformPlacementCategory::SmallJumpLeft,
        // Right
        EPlatformPlacementCategory::AboveRight,
        EPlatformPlacementCategory::SmallJumpRight
    } } },
    { "BelowRule", { {
        // Forward
        EPlatformPlacementCategory::BelowForward,
        // Left
        EPlatformPlacementCategory::BelowLeft,
        // Right
        EPlatformPlacementCategory::BelowRight, 
    } } },
    { "FarRule", { {
        EPlatformPlacementCategory::LongJumpForward,
        EPlatformPlacementCategory::LongJumpLeft,
        EPlatformPlacementCategory::LongJumpRight, 
    } } },
    { "SmallJumpRule", { {
        EPlatformPlacementCategory::SmallJumpForward,
        EPlatformPlacementCategory::SmallJumpLeft,
        EPlatformPlacementCategory::SmallJumpRight, 
    } } },
    { "VeryHighRule", {
        {
            EPlatformPlacementCategory::VeryHighPoint
        } } },
    { "VeryLowRule", {
        {
            EPlatformPlacementCategory::VeryLowPoint
        } } }
};
    
private:
    // Track the last platform's location and scale (starting with the initial platform).
    FVector LastPlatformLocation;
    FVector LastPlatformScale;
    FRotator LastPlatformRotation;
    EPlacementDirection LastPlacementDirection;

    // Helper function to check if a new platform intersects with existing platforms.
    bool IsLocationValid(const FVector& Location, const FVector& Scale) const;
    FBox CalculatePlatformBoundingBox(const FVector& Location, const FVector& Scale) const;

    // Store placed platforms' locations and scales.
    TArray<FVector> PlacedLocations;
    TArray<FVector> PlacedScales;
    // array for platforms
    UPROPERTY() TArray<AActor*> PlacedPlatforms;
    // array for obstacles
    UPROPERTY() TArray<AActor*> PlacedObstacles;
    // array for Surrounding Buildings
    UPROPERTY() TArray<AActor*> PlacedBuildings;

    TSet<FIntPoint> OccupiedCells;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true, DisplayName = "Spawn Parameters")) FGrammarRules FSpawnParams;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true, DisplayName = "Decoration Parameters")) FDecorateLevelRules FDecorateRules;
};