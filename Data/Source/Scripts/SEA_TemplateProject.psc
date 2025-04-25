Scriptname SEA_TemplateProject

;Called by the DLLWorking script if everything is in order. Passes in a confirmation message to be displayed.
;If the message is null, or debug messages are disabled in the ini file, then no message is shown.
Function Verify(Message a_kConfirmation) Global Native

;Returns the version of the DLL. Can be used to check that the DLL works correctly, or for addons.
Int[] Function GetVersion() Global Native