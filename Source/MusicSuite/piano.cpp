// Fill out your copyright notice in the Description page of Project Settings.

#include "MusicSuite.h"
#include "piano.h"
#include <string>
#include <sstream>
#include "MusicSuiteCharacter.h"
#include "Runtime/Engine/Classes/Engine/UserInterfaceSettings.h"
#include "Runtime/Engine/Classes/Engine/RendererSettings.h"
#include "Runtime/Engine/Public/CanvasItem.h"
#include "Runtime/Engine/Classes/Engine/Canvas.h"

std::string Apiano::TranslateToNoteName(uint8 key)
{

	int octave = (key / 12) - 1;

	std::string notes = "C DbD EbE F GbG AbA BbB ";
	std::string note = "";
	note = notes;
	note = note.substr(((key % 12) * 2), ((key % 12) * 2 + 2) - ((key % 12) * 2));

	std::ostringstream ostr;
	ostr << octave;

	if (note[note.length() - 1] == ' ')
	{
		note.erase(note.length() - 1);
	}

	note.append(ostr.str());


	return note;
}

// Sets default values
Apiano::Apiano()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ConstructorHelpers::FObjectFinder<UFont> FontObject(TEXT("/Game/Font/njnaruto"));
	if (FontObject.Object)
	{
		font = FontObject.Object;
		font->SetFontScalingFactor(0.005);
	}


	// Create and position a mesh component so we can see where our sphere is

	USkeletalMeshComponent *collisions = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Collisions"));

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> pianoasset(TEXT("/Game/Geometry/Props/piano"));
	if (!pianoasset.Succeeded())
		return;



	RootComponent = collisions;
	collisions->SetRelativeLocation(FVector(0.0f, 0.0f, -40.0f));
	collisions->SetWorldScale3D(FVector(0.22f));

	collisions->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	collisions->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	collisions->SetNotifyRigidBodyCollision(true);
	collisions->SetCollisionProfileName("Pawn");
	
	collisions->SetSkeletalMesh(pianoasset.Object);
	collisions->Activate();
	collisions->SetVisibility(false);



	UPoseableMeshComponent* poseroot = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("VisualRepresentation"));
	poseroot->AttachParent = collisions;
	poseroot->SetSkeletalMesh(pianoasset.Object);
	//poseroot->SetRelativeLocation(FVector(0.0f, 0.0f, -40.0f));
	poseroot->SetWorldScale3D(FVector(0.22f));

	poseroot->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	poseroot->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	poseroot->SetNotifyRigidBodyCollision(true);
	poseroot->SetCollisionProfileName("Pawn");

	poseroot->Activate();
	poseroot->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::AlwaysTickPoseAndRefreshBones; // update animation even when mesh is not visible
	
	bReplicates = true;
}

// Called when the game starts or when spawned
void Apiano::BeginPlay()
{

	Super::BeginPlay();
	UPoseableMeshComponent* poseroot = (UPoseableMeshComponent*)RootComponent->GetChildComponent(0);

	if (!poseroot)
	{
		UE_LOG(LogTemp, Warning, TEXT("couldn't get bone child!\n"));

		return;
	}

//	poseroot->SetBoneRotationByName("Bone_MiddleLid", poseroot->GetBoneRotationByName("Bone_MiddleLid", EBoneSpaces::ComponentSpace) + FRotator(0, 0, 10), EBoneSpaces::ComponentSpace);

	keypresses.SetNumUninitialized(0xFF);
	keypresses_velocity.SetNumUninitialized(0xFF);
	keypresses_state.SetNumUninitialized(0xFF);



	for (int i = 0; i < 0xFF; i++)
	{
		keypresses[i] = 0;
		keypresses_velocity[i] = 0;
		keypresses_state[i] = 0;
	}

	for (int i = 0; i < 255; i++)
	{

		std::string name = TranslateToNoteName(i);
		std::string sndpath = "/Game/Sound/pianokeys/" + name;
		USoundWave*snd = (USoundWave*)StaticLoadObject(USoundWave::StaticClass(), 0, *FString(sndpath.c_str()));

		if (!snd)
		{
			pianokeys.Add(0);
			pianokeyscomp.Add(0);
			continue;
		}

		UAudioComponent* comp = NewObject<UAudioComponent>(this, *FString(sndpath.c_str()));
		comp->SetSound(snd);
		comp->bStopWhenOwnerDestroyed = true;

		UE_LOG(LogTemp, Warning, TEXT("RECEIVED KEY: %i!\n"), i);

		//comp->AddToRoot();
		//snd->AddToRoot();//dont you dare gcing this

		pianokeys.Add(snd);
		pianokeyscomp.Add(comp);
	}



}

void Apiano::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);


	keypresses.Empty();
	keypresses_velocity.Empty();
	keypresses_state.Empty();
	

}
// Called every frame
void Apiano::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	UPoseableMeshComponent *mesh = (UPoseableMeshComponent*)GetRootComponent()->GetChildComponent(0);

	if (!mesh)
		return;

	FRotator ay = mesh->GetBoneRotationByName("Bone_MiddleLid", EBoneSpaces::ComponentSpace);

	static int cock = 1;

	if (GetAsyncKeyState(VK_F9))
	{
		if (cock == 1)
			ay.Pitch -= 0.5;

		if (cock == 2)
			ay.Yaw -= 0.5;

		if (cock == 3)
			ay.Roll -= 0.5;
	}

	if (GetAsyncKeyState(VK_F8))
	{
		if (cock == 1)
			ay.Pitch += 0.5;

		if (cock == 2)
			ay.Yaw += 0.5;

		if (cock == 3)
			ay.Roll += 0.5;
	}
	
	if (GetAsyncKeyState(VK_F10)&1)
	{
		cock = cock + 1;
		if (cock == 3)
			cock = 1;
	}

	mesh->SetBoneRotationByName("Bone_MiddleLid", ay, EBoneSpaces::ComponentSpace);


	bool logonce = false;

	for (int i = 0; i < 88; i++)
	{
		if (!keypresses_state[i])
			continue;

		std::string bonename = "Bone";
		std::stringstream tonum;
		tonum << i;
		
		if (i != 0)
		{
			if (tonum.str().length() == 1)
			{
				bonename.append("_00");
				bonename.append(tonum.str());
			}
			else {
				bonename.append("_0");
				bonename.append(tonum.str());
			}
		}

		//UE_LOG(LogTemp, Warning, TEXT("LOL: %s\n"), *fbonename.ToString());


		FRotator currotate = mesh->GetBoneRotationByName(bonename.c_str(), EBoneSpaces::ComponentSpace);

		if (!logonce)
		{
		//	UE_LOG(LogTemp, Warning, TEXT("\n%s __ ROTAADATE:  %f\n"), *FString(bonename.c_str()), currotate.Roll);
			logonce = true;
		}

		float factor = 1.2;

		if (keypresses_state[i]==1)
		{

			currotate.Roll += factor;//go down
			//UE_LOG(LogTemp, Warning, TEXT("A"));

			if (((int)floor(currotate.Roll)) == -173|| ((int)floor(currotate.Roll)) == -174|| ((int)floor(currotate.Roll)) == -175|| ((int)floor(currotate.Roll)) == -176)//-173 to -176 = max roll for down
			{
				keypresses_state[i] = 2;
				continue;
			}

			mesh->SetBoneRotationByName(bonename.c_str(), currotate, EBoneSpaces::ComponentSpace);
			mesh->RefreshBoneTransforms();
		
			continue;
		}

	//	UE_LOG(LogTemp, Warning, TEXT("B"));

		if (keypresses_state[i] == 3)
		{
			currotate.Roll -= factor;//go up

			mesh->SetBoneRotationByName(bonename.c_str(), currotate, EBoneSpaces::ComponentSpace);
			mesh->RefreshBoneTransforms();

			if (abs(currotate.Roll) == currotate.Roll)
			{
				keypresses_velocity[i] = 0;
				keypresses[i] = 0;
				keypresses_state[i] = 0;

				mesh->SetBoneRotationByName(bonename.c_str(), currotate, EBoneSpaces::ComponentSpace);
				mesh->RefreshBoneTransforms();
				continue;
			}
		}

	}


}

uint8 Apiano::ConvertToModelKey(uint8 keynum)
{
	//keynum = keynum - 21;//midi keys start at 21, models at 0
	int octave = int(keynum / 12)-1;
	int octave_key = (keynum % 12);

	bool is_black = false;

	if (octave_key == 1 || octave_key == 3 || octave_key == 6 || octave_key == 8 || octave_key == 10)
	{
		is_black = true;
	}

	int keynum_thistype = 0;


	for (int i=21; i < keynum; i++)
	{
		int checkkey = i % 12;

		if (is_black)
		{
			if (checkkey == 1 || checkkey == 3 || checkkey == 6 || checkkey == 8 || checkkey == 10)
			{
			//	UE_LOG(LogTemp, Warning, TEXT("BLACK: %i\n"), i);

				keynum_thistype++;
			}

		}
		else {
			if (!(checkkey == 1 || checkkey == 3 || checkkey == 6 || checkkey == 8 || checkkey == 10))
			{
				//UE_LOG(LogTemp, Warning, TEXT("WHITE: %i\n"), i);

				keynum_thistype++;
			}
		}


	}

	if (!is_black)
	{
		keynum_thistype += 36;
	}

	//UE_LOG(LogTemp, Warning, TEXT("CONVERT: OC: %d TYPE: %d NUM: %d P:%d\n"), octave, (int)is_black, octave_key, keynum_thistype);

	return keynum_thistype;
}

void Apiano::PressKey(uint8 keynum, uint8 velocity)
{

	//FIX THIS STUPID CRASH

	UAudioComponent* comp = pianokeyscomp[keynum];
	if (comp)
	{
		comp->Play();
	}

	keynum = ConvertToModelKey(keynum);


	UE_LOG(LogTemp, Warning, TEXT("LOL PRESSED: %i\n"), keynum);

	keypresses[keynum] = 0;
	//keypresses_velocity[keynum] = velocity;
	keypresses_velocity[keynum] = 0;
	keypresses_state[keynum] = 1;
}

void Apiano::ReleaseKey(uint8 keynum, uint8 velocity)
{


	keynum = ConvertToModelKey(keynum);

	UE_LOG(LogTemp, Warning, TEXT("LOL RELEASED: %i\n"), keynum);

	//keypresses[keynum] = 0;
	//keypresses_velocity[keynum] = velocity;

	if(keypresses_state[keynum])
		keypresses_state[keynum] = 3;

}


void Apiano::OnUsed(APawn*user)
{
	PlayMIDIFile(L"C://Users//Leystryku//Desktop//parting.mid");

	AMusicSuiteCharacter *player = (AMusicSuiteCharacter*)user;
	player->health = 1337;

	if (!player->usingpiano)
	{
		UE_LOG(LogTemp, Warning, TEXT("USE ME BABY <3"));
		player->usingpiano = this;
		return;
	}

	if (player->usingpiano != this)
		return;

	player->usingpiano = nullptr;
	UE_LOG(LogTemp, Warning, TEXT("WOW YOU STOPPED PLAYING ME RUDE"));

	for (int i = 0; i < 0xFF; i++)
		ReleaseKey(i, 0);

}




void Apiano::DrawHUD(UCanvas *Canvas, APawn *user)
{
	AMusicSuiteCharacter *player = (AMusicSuiteCharacter*)user;

	FText SomeText = FText::FromString("Use Me Baby x3..");
	if (player->usingpiano)
	{

		if (player->usingpiano == this)
		{
			SomeText = FText::FromString("im being used by u ok");
		}
		else {
			SomeText = FText::FromString("u betrayed me </3");
		}
	}

	FLinearColor TheFontColour = FLinearColor(1.0f, 1.0f, 1.0f);

	FCanvasTextItem NewText(FVector2D(1.0, 1.0), SomeText, font, TheFontColour);

	//Text Scale
	NewText.Scale.Set(1, 1);

	//Draw
	Canvas->DrawItem(NewText);

}

#define midiheader_t_bytes  4 + 4 + 2 + 2 + 2
#define miditrack_t_bytes 4+4

struct midiheader_t
{
	uint32 header;//must be 'MThd'
	uint32 headersize;//must be 6

	uint16 fileformat; // must be 0=single-track, 1=multiple & synch, 2 = asynch
	uint16 numtracks;
	uint16 deltatimeticksperquarternote; // wtf?


};	

struct miditrack_t
{
	uint32 header;//must be 'MTrk'
	uint32 tracklength;

};

uint32 from_seq(uint8 *in, uint8& tries)
{
	uint32 r = 0;

	do {
		r = (r << 7) | (uint32)(*in & 127);
		tries++;
	} while (*in++ & 128);

	return r;
}

bool Apiano::PlayMIDIFile(TCHAR *filename)
{
	IFileHandle *filehandle = FPlatformFileManager::Get().GetPlatformFile().OpenRead(filename);

	if (filehandle == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("NO FILE!\n"));
		return false;
	}
	filehandle->SeekFromEnd();

	midibuf_size = filehandle->Tell();

	if (midibuf_size < sizeof(midiheader_t)) { //too small to be a midi file
		midibuf_size = 0;
		UE_LOG(LogTemp, Warning, TEXT("TOO SMALL FILE: %llu!\n"), midibuf_size);
		return false;
	}


	filehandle->Seek(0);

	midibuf = new uint8[midibuf_size];
	filehandle->Read(midibuf, midibuf_size);


	midiheader_t *midiheader = (midiheader_t*)midibuf;

	if (midiheader->header != *(int*)"MThd") {
		UE_LOG(LogTemp, Warning, TEXT("Wrong header: %x!\n"), midiheader->header);
		return false;
	}

	midiheader->headersize = _byteswap_ulong(midiheader->headersize);

	midiheader->fileformat = _byteswap_ushort(midiheader->fileformat);
	midiheader->numtracks = _byteswap_ushort(midiheader->numtracks);
	midiheader->deltatimeticksperquarternote = _byteswap_ushort(midiheader->deltatimeticksperquarternote);

	if (midiheader->headersize != 0x6) {
		UE_LOG(LogTemp, Warning, TEXT("Wrong header size: %d!\n"), midiheader->headersize);
		return false;
	}

	if (midiheader->fileformat != 0&& midiheader->fileformat!=1&& midiheader->fileformat!=2) {
		UE_LOG(LogTemp, Warning, TEXT("Wrong file format!\n"));
		return false;
	}
	
	int64 midioffset = midiheader_t_bytes;

	UE_LOG(LogTemp, Warning, TEXT("PLAYING TRACKS: %d __ %d!\n"), midiheader->numtracks);

	std::vector <uint8_t> midievents;

	uint16 tracknum = 0;//for (uint16 tracknum = 0; tracknum < midiheader->numtracks; tracknum++) 
	{
		if (midibuf_size < midioffset)
			return 1;//break;

		miditrack_t *track = (miditrack_t*)(midibuf + midioffset);

		UE_LOG(LogTemp, Warning, TEXT("HEADER: %x __ TRACKLEN: %x\n"), track->header, track->tracklength);

		if (track->header != *(int*)"MTrk")
			return 1;//continue;

		track->tracklength = _byteswap_ulong(track->tracklength);
		UE_LOG(LogTemp, Warning, TEXT("yo, track %d has a length of %d\n"), tracknum, track->tracklength);

		for (uint32 i = 0; i < track->tracklength; i++)
		{
			// [] = variable-length
			//<MTrk event> = <[]delta-time><curevent>
			//<curevent> = <MIDI event> | <sysex event> | <meta-event>
			
			uint8 *curpos = (uint8*)((uint8*)track + miditrack_t_bytes + i);


			uint8 baseoffset = 0;

			uint32 deltatime = from_seq(curpos, baseoffset);

			

			uint8 type_and_chan = *(curpos + baseoffset);
			uint8 type = type_and_chan & 0xF0;//high byte only
			uint8 chan = type_and_chan & 0x0F;//low byte only

			uint8 flag1 = *(curpos+ baseoffset+1);
			uint8 flag2 = *(curpos+baseoffset+2);

			UE_LOG(LogTemp, Warning, TEXT("LOL: %x __ %x __ %x\n"), type_and_chan, flag1, flag2);

			if (type == 0x90 && flag2 == 0)//pressed but 0 velocity = actually a release signal
				type = 0x80;

			if (type_and_chan == 0xFF)//metaevent
			{
				if (flag1 == 0x58)
				{

				
					uint8 tries = 0;
					uint32 metalength = from_seq((curpos + baseoffset + 2), tries);

					uint8* metadata = new uint8[metalength];



					midievents.push_back(type_and_chan);
					midievents.push_back(flag1);
					midievents.push_back(metalength);

					for (uint32 i = 0; i < metalength; i++)
					{
						midievents.push_back(metadata[i]);
					}


					delete[] metadata;
					UE_LOG(LogTemp, Warning, TEXT("TIME SIG :: %i __ %i __ %i"), metalength, i, i + baseoffset + metalength + tries);
					i = i + baseoffset + metalength + tries + 1;
					continue;

				}


				if (flag1 == 0x59)//key signature
				{


					uint8 tries = 0;
					uint32 metalength = from_seq((curpos + baseoffset + 1), tries);

					uint8* metadata = new uint8[metalength];



					midievents.push_back(type_and_chan);
					midievents.push_back(flag1);
					midievents.push_back(metalength);

					for (uint32 i = 0; i < metalength; i++)
					{
						midievents.push_back(metadata[i]);
					}


					delete[] metadata;
					UE_LOG(LogTemp, Warning, TEXT("KEYSIG  :: %i __ %i __ %i"), metalength, i,  baseoffset + metalength + tries);
					i = i + baseoffset + metalength + tries;
					break;

					continue;

				}

				if (flag1 == 0x51)//tempo
				{


					uint8 tries = 0;
					uint32 metalength = from_seq((curpos + baseoffset + 1), tries);

					uint8* metadata = new uint8[metalength];



					midievents.push_back(type_and_chan);
					midievents.push_back(flag1);
					midievents.push_back(metalength);

					for (uint32 i = 0; i < metalength; i++)
					{
						midievents.push_back(metadata[i]);
					}


					delete[] metadata;
					UE_LOG(LogTemp, Warning, TEXT("TEMPO  :: %i __ %i __ %i"), metalength, i, i + baseoffset + metalength + tries);
					i = i + baseoffset + metalength + tries + 1;

					break;
					continue;

				}

			}

			if (type == 0x90)//note on
			{
				PressKey(flag1, flag2);
			}
			
			if (type == 0x80)//note off
			{
				ReleaseKey(flag1, flag2);
			}


			midievents.push_back(type_and_chan);
			midievents.push_back(flag1);
			midievents.push_back(flag2);


			i = i + baseoffset + 2;

			Sleep(10);
		}

		midioffset += miditrack_t_bytes + track->tracklength;

	}

	return true;
}