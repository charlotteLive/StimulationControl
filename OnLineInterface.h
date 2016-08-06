typedef struct tagSAFEARRAYBOUND {
	unsigned long cElements;
	long lLbound;
} SAFEARRAYBOUND;

typedef struct tagSAFEARRAY {
	unsigned short cDims;		// Count of dimensions in this array.
	unsigned short fFeatures;	// Flags used by the SafeArray
	unsigned long cbElements;	// Size of an element of the array. Does not include size of pointed-to data.
	unsigned long cLocks;		// Number of times the array has been locked without corresponding unlock.
	void * pvData;				// Pointer to the data.
	SAFEARRAYBOUND rgsabound;	// One bound for each dimension.
} SAFEARRAY;
  
//extern "C" __declspec(dllexport) long OnLineGetData(int ch, int sizeMs, SAFEARRAY **pData, int *pActualSamples);
//extern "C" __declspec(dllexport) long OnLineGetData(int ch, int sizeMs, SafeArray **pData, int *pActualSamples);
//extern "C" __declspec(dllexport) long OnLineGetData(int ch, int sizeMs, mxArray *pData, int *pActualSamples);

extern "C" __declspec(dllexport) long OnLineGetData(int ch, int sizeMs, SAFEARRAY **pData, int *pActualSamples);

/*extern "C" __declspec(dllexport) long OnLineGetData(
			[in] long channel, 
			[in] long sizeMsToRead, 
			[out] SAFEARRAY(short) *DataArray, 
			[out] long *pActualSamples
			); */
			
extern "C" __declspec(dllexport) long OnLineStatus(int ch, int statusType, int *pStatus);
/* above is the same as: int __stdcall OnLineStatus(long channel, long statusType, long *pStatus); */
