Scriptname SEA_MagicTweaks Hidden

;Returns the version of the DLL.
Int[] Function GetVersion() Global Native

;Unbinds all spells currently bound on the player. Returns true if successful.
Bool Function UnBindAllSpells() Global Native

;Unbinds selected spell from the player. Returns how many of said spell instances are
;still bound on the player
Int Function UnBindSpell(Spell a_kSpell) Global Native