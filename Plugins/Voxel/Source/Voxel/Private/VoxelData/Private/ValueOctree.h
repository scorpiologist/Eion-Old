// Copyright 2018 Phyronnaz

#pragma once

#include <shared_mutex>

#include "CoreMinimal.h"
#include "Octree.h"
#include "VoxelSave.h"
#include "VoxelDiff.h"
#include "ThreadSafeBool.h"
#include "VoxelGlobals.h"

class FVoxelWorldGeneratorInstance;
class FVoxelAssetInstance;

/**
 * Octree that holds modified values & materials
 */
class FValueOctree : public TVoxelOctree<FValueOctree, DATA_CHUNK_SIZE>
{
public:
	FValueOctree(TSharedRef<FVoxelWorldGeneratorInstance> WorldGenerator, uint8 LOD, bool bMultiplayer);
	FValueOctree(FValueOctree* Parent, uint8 ChildIndex);
	~FValueOctree();

	// Is the game multiplayer?
	const bool bMultiplayer;

	// Generator for this world
	TSharedRef<FVoxelWorldGeneratorInstance> WorldGenerator;

	/**
	 * Does this chunk have been modified?
	 */
	bool IsDirty() const;
	
	bool IsEmpty(const FIntVector& Start, const int Step, const FIntVector& Size) const;

	/**
	 * Get values and materials
	 * @see FVoxelWorldGeneratorInstance
	 */
	void GetValuesAndMaterials(float Values[], FVoxelMaterial Materials[], const FIntVector& Start, const FIntVector& StartIndex, const int Step, const FIntVector& Size, const FIntVector& ArraySize) const;

	/**
	 * Set value and material at (X,Y,Z)
	 * @param	X,Y,Z			Position in global space
	 * @param	Value			The value to set
	 * @param	Material		The material to set
	 * @param	bSetValue		Should the value be set?
	 * @param	bSetMaterial	Should the material be set?
	 */
	// TODO: poor design?
	void SetValueAndMaterial(int X, int Y, int Z, float Value, FVoxelMaterial Material, bool bSetValue, bool bSetMaterial);

	/**
	 * Remove dirty flag if set
	 */
	void SetAsNotDirty();
	
	/**
	 * Add a voxel asset
	 */
	// TODO: Discard chunks entirely inside the asset
	void AddAsset(TSharedRef<FVoxelAssetInstance> Asset);

	/**
	 * Remove all the assets of this chunk
	 */
	void RemoveAssets();

	/**
	 * Add dirty chunks to SaveList
	 * @param	SaveList		List to save chunks into. Sorted by increasing Id
	 */
	void AddDirtyChunksToSaveQueue(TArray<FVoxelChunkSave>& SaveQueue);
	/**
	 * Load chunks from Save list
	 * @param	SaveQueue				Queue to load chunks from. Sorted by decreasing Id (top is lowest Id)
	 * @return	OutModifiedPositions	The modified positions
	 */
	void LoadFromSaveQueueAndGetModifiedPositions(TArray<FVoxelChunkSave>& SaveQueue, TArray<FIntVector>& OutModifiedPositions);
	
	/**
	 * Add values that have changed since last network sync to diff arrays
	 * @param	OutValueDiffQueue		Values diff array; sorted by increasing Id
	 * @param	OutColorDiffQueue		Colors diff array; sorted by increasing Id
	 */
	void AddChunksToDiffQueues(TArray<FVoxelValueDiff>& OutValueDiffQueue, TArray<FVoxelMaterialDiff>& OutColorDiffQueue);

	void LoadValueDiff(FVoxelValueDiff& Diff, TArray<FIntVector>& OutModifiedPositions);
	void LoadMaterialDiff(FVoxelMaterialDiff& Diff, TArray<FIntVector>& OutModifiedPositions);
	
	/**
	 * Get the positions to update for this chunk. Must be called only if LOD == 0
	 */
	void GetPositionsToUpdate(TArray<FIntVector>& OutPositions);

	/**
	 * Set the new world generator
	 */
	void SetWorldGenerator(TSharedRef<FVoxelWorldGeneratorInstance> NewGenerator);

	/**
	 * P(bounds) = -1: discard
	 * P(bounds) = 0 : call with smaller bounds
	 * P(bounds) = 1 : keep
	 */
	void DiscardValuesByPredicate(const std::function<int(const FIntBox&)>& P);

	/**
	 * Recursively set chunk as not dirty
	 */
	void SetEntireChunkAsNotDirty();
	
	// sorted by increasing Id
	void BeginSet(const FIntBox& Box, TArray<uint64>& OutIds);
	// sorted by decreasing Id
	void EndSet(TArray<uint64>& Ids);
	
	// sorted by increasing Id
	void BeginGet(const FIntBox& Box, TArray<uint64>& OutIds);
	// sorted by decreasing Id
	void EndGet(TArray<uint64>& Ids);

	void LockTransactions();

private:
	// Values if dirty
	float* Values;
	// Materials if dirty
	FVoxelMaterial* Materials;
	
	// Voxel assets
	TArray<TSharedRef<FVoxelAssetInstance>> Assets;

	// Is the chunk dirty? Undefined behaviour if LOD != 0
	bool bIsDirty;

	// For multiplayer
	TSet<uint32> DirtyValues;
	TSet<uint32> DirtyMaterials;

	// Has the chunk changed since last sync?
	bool bIsNetworkDirty;

	std::shared_mutex MainLock;
	std::mutex TransactionLock;

	FThreadSafeCounter GetCounter;
	FThreadSafeCounter SetCounter;

	FThreadSafeBool bIsLocked;


	/**
	 * Create childs of this octree
	 */
	virtual void CreateChilds() override;

	/**
	 * Init arrays
	 */
	void SetAsDirtyAndSetDefaultValues();

	/**
	 * Get the arrays index corresponding to (X, Y, Z)
	 */
	FORCEINLINE uint32 IndexFromCoordinates(int X, int Y, int Z) const;

	/**
	 * Get the (X, Y, Z) coordinates corresponding to the array index
	 */
	FORCEINLINE void CoordinatesFromIndex(uint32 Index, int& OutX, int& OutY, int& OutZ) const;
};