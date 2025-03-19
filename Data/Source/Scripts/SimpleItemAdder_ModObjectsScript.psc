Scriptname SimpleItemAdder_ModObjectsScript Extends Quest

Int[] kVersionInternal
Int[] Property kVersion Hidden
	Int[] Function Get()
		return kVersionInternal
	EndFunction

	Function Set(Int[] a_newVer)
		If (a_newVer.Length != 3)

			Return
		EndIf

		kVersionInternal = a_newVer
	EndFunction
EndProperty

Event OnInit()

	kVersionInternal = new Int[3]
	kVersionInternal[0] = -1
	kVersionInternal[1] = 0
	kVersionInternal[2] = 0

	Int[] iDLLResponse = SEA_SimpleItemAdder.GetVersion()
	If (!iDLLResponse)
		Debug.Messagebox("Failed to get version from the DLL")
		Return
	EndIf

	kVersion  = iDLLResponse 
	Debug.Messagebox("Running SimpleItemAdder version: " + kVersion[0] + "." + kVersion[1] + "." + kVersion[2])
EndEvent