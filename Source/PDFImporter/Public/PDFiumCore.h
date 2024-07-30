#pragma once

#include "CoreMinimal.h"
#include "PDFImporter.h"
#include <fpdfview.h>

class PDFIMPORTER_API FPDFiumCore
{
private:
	// PDFium module
	void* PDFiumModule;

	TSharedPtr<class IImageWrapper> ImageWrapper;

public:
	// The path to the directory where the page's texture assets are located
	static const FString PagesDirectoryPath;

public:
	// Convert PDF to PDF asset
	class UPDF* ConvertPdfToPdfAsset(const FString& InputPath, int Dpi, int FirstPage, int LastPage, bool bIsImportIntoEditor = false);

	bool PDFiumLib_State();
private:
	// Init PDFium Library
	bool PDFiumLib_Init();

	// Close PDFium Library
	bool PDFiumLib_Close();

	// Convert PDF to multiple jpeg images using PDFium
	bool ConvertPdfToJpeg(const FString& InputPath, const FString& OutputPath, int Dpi, int FirstPage, int LastPage);

	// Create UTexture2D from image files in directory
	bool LoadTexture2DFromFile(const FString& FilePath, class UTexture2D*& LoadedTexture);

	// Get Text from PDF files using PDFium
	bool LoadTextFromFile(const FString& FilePath, TArray<FString>& Out_Texts);

#if WITH_EDITORONLY_DATA
	// Create texture asset from image files in directory
	bool CreateTextureAssetFromFile(const FString& FilePath, class UTexture2D*& LoadedTexture);
#endif

private:
	// Only PDFImporterModule can create instances
	friend FPDFImporterModule;

	FPDFiumCore();

public:
	// Destructor is open because it is managed by TSharedPtr
	~FPDFiumCore();
};
