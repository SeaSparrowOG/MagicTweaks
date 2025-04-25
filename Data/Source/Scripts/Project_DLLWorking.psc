Scriptname Project_DLLWorking Extends Quest
{Replace the name with your own. Checks that both SKSE and the DLL are working.}

Message Property SKSEMissingMeassage Auto
Message Property DLLMissingMessage Auto
Message Property GoodToGoMessage Auto

Function Check()

	Int iSKSEVersion = SKSE.GetVersion()
	If (!iSKSEVersion) 

		SKSEMissingMeassage.Show()
		Return
	EndIf

	Int[] iDLLVersion = SEA_TemplateProject.GetVersion()
	If (!iDLLVersion) 

		DLLMissingMessage.Show()
		Return
	EndIf

	SEA_TemplateProject.Verify(GoodToGoMessage)
EndFunction