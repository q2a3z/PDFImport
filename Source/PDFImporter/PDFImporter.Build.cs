// Some copyright should be here...

using UnrealBuildTool;
using System;
using System.IO;

public class PDFImporter : ModuleRules
{
	public PDFImporter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "SlateCore",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "Projects",
				// ... add private dependencies that you statically link with here ...	
			}
			);

        string GhostscriptPath = Path.Combine(ModuleDirectory, "..", "..", "ThirdParty");
        string PDFiumPath = Path.Combine(ModuleDirectory, "..", "..", "ThirdParty");

        string Platform = string.Empty;
	
        if(Target.Platform == UnrealTargetPlatform.Win64)
        {
            Platform = "Win64";
        }
#if UE_5_0_OR_LATER
#else
        else if(Target.Platform == UnrealTargetPlatform.Win32)
        {
            Platform = "Win32";
        }
#endif
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            //if (Target.Version.MajorVersion >= 5 && Target.Version.MinorVersion >= 1)
            Platform = "OSX";
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            Platform = "Linux";
        }
        else
        {
            throw new Exception(string.Format("Unsupported platform {0}", Target.Platform.ToString()));
        }
	
        GhostscriptPath = Path.Combine(GhostscriptPath, Platform, "gsdll.dll");

        if(!File.Exists(GhostscriptPath))
        {
            throw new Exception(string.Format("File not found {0}", GhostscriptPath));
        }

        RuntimeDependencies.Add(GhostscriptPath);
        
        PDFiumPath = Path.Combine(PDFiumPath, Platform, "pdfium.dll");
        
        if(!File.Exists(PDFiumPath))
        {
            throw new Exception(string.Format("File not found {0}", PDFiumPath));
        }
        RuntimeDependencies.Add(PDFiumPath);
        //string Location_PDFium = "../../ThirdParty";
        string Location_PDFium = Path.Combine(ModuleDirectory, "..", "..", "ThirdParty","include");
        PrivateIncludePaths.Add(Location_PDFium);

        PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "..", "..", "ThirdParty",Platform,"pdfium.dll.lib"));
        PublicDelayLoadDLLs.Add("pdfium.dll");        
    }
}
