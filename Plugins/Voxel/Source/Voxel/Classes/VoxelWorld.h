// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "IntBox.h"
#include "VoxelSave.h"
#include "VoxelAsset.h"
#include "VoxelGrassSpawner.h"
#include "VoxelWorldGenerator.h"
#include "GameFramework/Actor.h"
#include "Templates/SubclassOf.h"
#include "VoxelActorSpawner.h"
#include "VoxelRenderFactory.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "VoxelWorld.generated.h"

class IVoxelRender;
class FVoxelData;
class FVoxelActorOctree;
class UVoxelInvokerComponent;
class AVoxelWorldEditorInterface;
class AVoxelActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnClientConnection);

/**
 * Voxel World actor class
 */
UCLASS()
class VOXEL_API AVoxelWorld : public AActor
{
	GENERATED_BODY()

public:
	// Used to get a ref to AVoxelWorldEditor::StaticClass()
	UClass* VoxelWorldEditorClass;

	UPROPERTY(BlueprintAssignable)
	FOnClientConnection OnClientConnection;
		
	AVoxelWorld();
	~AVoxelWorld();
	
	void AddActor(AVoxelActor* Actor);

	void NotifyActorsAreCreated(const FIntVector& ChunkPositon);
	bool HasActorsBeenCreated(const FIntVector& ChunkPosition) const;

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void GetActorsInBox(const FIntBox& Box, TArray<AVoxelActor*>& Actors);

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void RemoveActorFromOctree(AVoxelActor* Actor);

	
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void CreateWorld();

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void DestroyWorld();

	/**
	 * Create the world when in editor (not PIE)
	 */
	void CreateInEditor();
	/**
	 * Destroy the world when in editor (not PIE)
	 */
	void DestroyInEditor();

	/**
	 * Add a VoxelInvoker
	 */
	void AddInvoker(TWeakObjectPtr<UVoxelInvokerComponent> Invoker);

	// Getters
	FORCEINLINE AVoxelWorldEditorInterface* GetVoxelWorldEditor() const;
	FORCEINLINE FVoxelData* GetData() const;
	FORCEINLINE FVoxelWorldGeneratorInstance* GetWorldGenerator() const;
	FORCEINLINE const FVoxelGrassSpawner_ThreadSafe& GetGrassSpawner() const;
	FORCEINLINE const FVoxelActorSpawner_ThreadSafe& GetActorSpawner() const;
	FORCEINLINE int GetMaxCollisionsLOD() const;
	FORCEINLINE bool GetDebugCollisions() const;
	FORCEINLINE float GetCollisionsUpdateRate() const;
	FORCEINLINE float GetLODUpdateRate() const;
	FORCEINLINE float GetChunksFadeDuration() const;
	FORCEINLINE int GetCollisionsThreadCount() const;
	FORCEINLINE int GetMeshThreadCount() const;
	FORCEINLINE FQueuedThreadPool* GetAsyncTasksThreadPool() const;
	FORCEINLINE bool GetCreateAdditionalVerticesForMaterialsTransitions() const;
	UMaterialInstanceDynamic* GetVoxelMaterialDynamicInstance();
	FORCEINLINE UMaterialInterface* GetVoxelMaterial() const;
	FORCEINLINE bool GetEnableNormals() const;

	
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	int GetSeed() const;

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void SetSeed(int Seed);

	/**
	 * Set the voxel material
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void SetVoxelMaterial(UMaterialInterface* NewMaterial);

	/**
	 * Set the world LOD. The world must not be created
	 */	
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void SetLOD(uint8 NewLOD);
	
	/**
	 * Replace the World Generator. Can be called at runtime
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void SetWorldGenerator(UVoxelWorldGenerator* NewGenerator);
	
	/**
	 * Swap the world generator, keeping a sphere unmodified. Need to call UpdateAll after this
	 * @param	NewGenerator	The new generator outside the sphere
	 * @param	Center			The center of the sphere, in voxels
	 * @param	Radius			The radius of the sphere, in voxels
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	 void SwapWorldGeneratorSphere(UVoxelWorldGenerator* NewGenerator, const FIntVector& Center, float Radius);

	/**
	 * Is this world created?
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	bool IsCreated() const;

	/**
	 * Get the render chunks LOD at Position
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	int GetLODAt(const FIntVector& Position) const;
	
	/**
	 * Get the world LOD
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	uint8 GetLOD() const;

	/**
	 *  Size of a voxel in unit (cm)
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	float GetVoxelSize() const;

	/**
	 * Bounds of this world
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	FIntBox GetBounds() const;

	/**
	 * Convert position from world space to voxel space
	 * @param	Position	Position in world space
	 * @return	Position in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	FIntVector GlobalToLocal(const FVector& Position) const;
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	FVector GlobalToLocalFloat(const FVector& Position) const;

	/**
	 * Convert position from voxel space to world space
	 * @param	Position	Position in voxel space
	 * @return	Position in world space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	FVector LocalToGlobal(const FIntVector& Position) const;
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	FVector LocalToGlobalFloat(const FVector& Position) const;

	/**
	 * Get the 8 neighbors in voxel space of GlobalPosition
	 * @param	GlobalPosition	The position in world space
	 * @return	The 8 neighbors in voxel space 
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	TArray<FIntVector> GetNeighboringPositions(const FVector& GlobalPosition) const;

	/**
	 * Add chunks at position to update queue
	 * @param	Position	Position in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void UpdateChunksAtPosition(const FIntVector& Position);

	/**
	 * Add chunks overlapping box to update queue
	 * @param	Box			Box
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void UpdateChunksOverlappingBox(const FIntBox& Box);

	/**
	 * Update all the chunks
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void UpdateAll();

	/**
	 * Is position in this world?
	 * @param	Position	Position in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	bool IsInWorld(const FIntVector& Position) const;

	/**
	 * Get the intersection using voxel data. Doesn't depend on LOD. Useful for short distances, but costful for big ones
	 * @param	Start				The start of the raycast. The start and the end must have only one coordinate not in common
	 * @param	End					The end of the raycast. The start and the end must have only one coordinate not in common
	 * @return	GlobalPosition		The world position of the intersection if found
	 * @return	VoxelPosition		The voxel position of the intersection if found
	 * @return	Has intersected?
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (DisplayName = "Get Intersection"))
	bool GetIntersectionBP(const FIntVector& Start, const FIntVector& End, FVector& GlobalPosition, FIntVector& VoxelPosition); // BP Function can't be const for performance (pure are called for each output)
	bool GetIntersection(const FIntVector& Start, const FIntVector& End, FVector& GlobalPosition, FIntVector& VoxelPosition) const;

	/**
	 * Get the normal at the voxel position Position using gradient. May differ from the mesh normal
	 * @param	Position	Position in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	FVector GetNormal(const FIntVector& Position) const;

	/**
	 * Get value at position
	 * @param	Position	Position in voxel space
	 * @return	Value at position
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	float GetValue(const FIntVector& Position) const;
	/**
	 * Get material at position
	 * @param	Position	Position in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	FVoxelMaterial GetMaterial(const FIntVector& Position) const;

	/**
	 * Set value at position
	 * @param	Position	Position in voxel space
	 * @param	Value		Value to set
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void SetValue(const FIntVector& Position, float Value);
	/**
	 * Set material at position
	 * @param	Position	Position in voxel space
	 * @param	Material	FVoxelMaterial
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void SetMaterial(const FIntVector& Position, const FVoxelMaterial& Material);
	
	/**
	 * Add an asset to the world at Position
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void AddAsset(UVoxelAsset* Asset, FIntVector Position);

	
	/**
	 * Is the position inside the meshes? Not accurate
	 * @param	Position	Position in global space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	bool IsInside(const FVector& Position);

	/**
	 * Get the world save
	 * @return	SaveArray
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void GetSave(FVoxelWorldSave& OutSave) const;
	/**
	 * Load world from save
	 * @param	Save	Save to load from
	 * @param	bReset	Reset existing world? Set to false only if current world is unmodified
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "1"))
	void LoadFromSave(const FVoxelWorldSave& Save, bool bReset = true);
	
	/**
	 * Use this world as a multiplayer server
	 * @param	Ip		The ip of the server. Most of the time its ip on its local network (not 127.0.0.1)
	 * @param	Port	The port to listen on
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void StartServer(const FString& Ip, int32 Port);
	/**
	 * Use this world as a multiplayer client
	 * @param	Ip		The ip of the server
	 * @param	Port	The port of the server
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void ConnectClient(const FString& Ip, int32 Port);

protected:
	//~ Begin AActor Interface
	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void Tick(float DeltaTime) override;
#if WITH_EDITOR
	bool ShouldTickIfViewportsOnly() const override;
	bool CanEditChange(const UProperty* InProperty) const override;
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent);
#endif
	//~ End AActor Interface

private:
	// Size = CHUNK_SIZE * 2^LOD. Has little impact on performance
	UPROPERTY(EditAnywhere, Category = "Voxel|General", meta = (ClampMin = "1", ClampMax = "19", UIMin = "1", UIMax = "19"))
	int LOD;

	UPROPERTY(VisibleAnywhere, Category = "Voxel|General")
	int WorldSizeInVoxel;

	// Size of a voxel in cm
	UPROPERTY(EditAnywhere, Category = "Voxel|General")
	float VoxelSize;

	// Generator of this world
	UPROPERTY(EditAnywhere, Category = "Voxel|General")
	FVoxelWorldGeneratorPicker WorldGenerator;

	// The seed of this world. For now only used for grass
	UPROPERTY(EditAnywhere, Category = "Voxel|General", meta = (ClampMin = "1", UIMin = "1"))
	int32 Seed;

	UPROPERTY(EditAnywhere, Category = "Voxel|General")
	bool bCreateWorldAutomatically;



	UPROPERTY(EditAnywhere, Category = "Voxel|Rendering")
	EVoxelRenderType RenderType;

	// The material of the world
	UPROPERTY(EditAnywhere, Category = "Voxel|Rendering")
	UMaterialInterface* VoxelMaterial;
	
	UPROPERTY(EditAnywhere, Category = "Voxel|Rendering")
	UVoxelGrassSpawner* GrassSpawner;
	
	UPROPERTY(EditAnywhere, Category = "Voxel|Rendering")
	UVoxelActorSpawner* ActorSpawner;
		
	UPROPERTY(EditAnywhere, Category = "Voxel|Rendering")
	float MaxVoxelActorsRenderDistance;
	
	UPROPERTY(EditAnywhere, Category = "Voxel|Rendering", AdvancedDisplay)
	bool bEnableNormals;

	UPROPERTY(EditAnywhere, Category = "Voxel|Rendering", meta = (ClampMin = "0", UIMin = "0"), AdvancedDisplay)
	float ChunksFadeDuration;

	// Add vertices to reduce materials transitions glitches, however it can make holes in the grounds if tessellation is enabled.
	UPROPERTY(EditAnywhere, Category = "Voxel|Rendering", AdvancedDisplay)
	bool bCreateAdditionalVerticesForMaterialsTransitions;



	// Max LOD to compute collisions on. Inclusive. Collisions around player are always computed
	UPROPERTY(EditAnywhere, Category = "Voxel|Collisions", AdvancedDisplay, meta = (ClampMin = "-1", UIMin = "-1"))
	int MaxCollisionsLOD;

	// Should the collisions meshes around the player be rendered?
	UPROPERTY(EditAnywhere, Category = "Voxel|Collisions", AdvancedDisplay)
	bool bDebugCollisions;
	


	// Number of collision update per second
	UPROPERTY(EditAnywhere, Category = "Voxel|Performance", AdvancedDisplay, meta = (ClampMin = "0.001", UIMin = "0.001"))
	float CollisionsUpdateRate;

	// Number of LOD update per second
	UPROPERTY(EditAnywhere, Category = "Voxel|Performance", AdvancedDisplay, meta = (ClampMin = "0.001", UIMin = "0.001"), DisplayName = "LOD Update Rate")
	float LODUpdateRate;

	// Number of threads allocated for the mesh processing. Setting it too high may impact performance
	UPROPERTY(EditAnywhere, Category = "Voxel|Performance", meta = (ClampMin = "1", UIMin = "1"), AdvancedDisplay)
	int MeshThreadCount;

	// Number of threads allocated for the collisions meshes processing. Setting it too low may impact performance
	UPROPERTY(EditAnywhere, Category = "Voxel|Performance", meta = (ClampMin = "1", UIMin = "1"), AdvancedDisplay)
	int CollisionsThreadCount;


	// Is this world multiplayer?
	UPROPERTY(EditAnywhere, Category = "Voxel|Multiplayer")
	bool bMultiplayer;

	// Number of sync per second
	UPROPERTY(EditAnywhere, Category = "Voxel|Multiplayer", meta = (EditCondition = "bMultiplayer"))
	float MultiplayerSyncRate;

	// Debug multiplayer syncs?
	UPROPERTY(EditAnywhere, Category = "Voxel|Multiplayer", meta = (EditCondition = "bMultiplayer"))
	bool bDebugMultiplayer;



	TSharedPtr<FVoxelWorldGeneratorInstance> InstancedWorldGenerator;

	UPROPERTY()
	AVoxelWorldEditorInterface* VoxelWorldEditor;

	UPROPERTY()
	class AVoxelChunksOwner* ChunksOwner;

	UPROPERTY()
	UMaterialInstanceDynamic* VoxelMaterialInstance;
	
	FVoxelGrassSpawner_ThreadSafe GrassConfig_ThreadSafe;

	FVoxelActorSpawner_ThreadSafe ActorSpawnerConfig_ThreadSafe;

	FQueuedThreadPool* AsyncTasksThreadPool;

	TSharedPtr<class FVoxelTcpServer> TcpServer;
	TSharedPtr<class FVoxelTcpClient> TcpClient;

	TSharedPtr<FVoxelData> Data;
	TSharedPtr<IVoxelRender> Render;

	FThreadSafeCounter OnClientConnectionTrigger;

	bool bIsCreated;

	float TimeSinceSync;

	float TimeSinceActorOctreeUpdate;

	TSet<FIntVector> ChunksWithCreatedActors;
	TSharedPtr<FVoxelActorOctree> ActorOctree;

	TArray<TWeakObjectPtr<UVoxelInvokerComponent>> Invokers;

	// Create the world
	void CreateWorldInternal(AActor* ChunksOwner = nullptr);
	// Destroy the world
	void DestroyWorldInternal();

	// Receive data from server
	void ReceiveData();
	// Send data to clients
	void SendData();

	void TriggerOnClientConnection();
};
