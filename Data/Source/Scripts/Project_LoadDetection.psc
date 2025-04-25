Scriptname Project_LoadDetection Extends ReferenceAlias
{Replace the name with your own naming convection. Used to detect when the game loads.}

Event OnPlayerLoadGame()

	(Self.GetOwningQuest() As Project_DLLWorking).Check()
EndEvent