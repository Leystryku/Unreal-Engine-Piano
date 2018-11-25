// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class MusicSuite : ModuleRules
{
	private string ModulePath
    {
        get { return Path.GetDirectoryName( RulesCompiler.GetModuleFilename( this.GetType().Name ) ); }
    }
 
    private string ThirdPartyPath
    {
        get { return Path.GetFullPath( Path.Combine( ModulePath, "../../ThirdParty/" ) ); }
    }
	
	public MusicSuite(TargetInfo Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "UMG", "Slate", "SlateCore", "OnlineSubsystem", "OnlineSubsystemUtils" });
		
		DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
		
		LoadThirdPartyLibrary(Target, "rtmidi");
	}
	
	public bool LoadThirdPartyLibrary(TargetInfo Target, string libraryname)
    {
        bool isLibrarySupported = false;
 
        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
        {
            isLibrarySupported = true;
 
            string PlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "x64" : "x86";
            string LibrariesPath = Path.Combine(ThirdPartyPath, libraryname, "Libraries");
 
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, libraryname + "." + PlatformString + ".lib")); 
        }
 
        if (isLibrarySupported)
        {
            // Include path
            PublicIncludePaths.Add( Path.Combine( ThirdPartyPath, libraryname, "Includes" ) );
        }
 
        Definitions.Add(string.Format( libraryname + "={0}", isLibrarySupported ? 1 : 0 ) );
 
        return isLibrarySupported;
    }
 
}
