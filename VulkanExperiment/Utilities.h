#pragma once

// Indices (locations) of Queue Families (if they exist at all)
struct QueueFamilyIndices {
	int graphicsFamily = -1;	// Location of Graphics Que Family

	//Check if queue families are Valid
	bool isValid()
	{
		return graphicsFamily >= 0;
	}
};