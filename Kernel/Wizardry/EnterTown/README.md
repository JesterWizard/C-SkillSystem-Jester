Enter Town ASM (or any chapter by technicality) V.1.1

Simple, adds a script to have the player to enter towns (and by that I mean chapter) and explore and return seamlessly to the world map

There are multiple parts to this and the way I had it originally was confusing so I going to adress that and make this easier to work with. Also this is made with FEBuilder in mind.
There are 2 folders here, one for the event and one for the ASM's that need to be installed seperatly.
1. Install the event normally, I rememberd the .dmp's this time.
2. Install all 3 ASM's and take note of the adresses of where they are.
3. You now need to make 2 new menus: one where the overworld node commands are (Mannage Items, Enter Shop, etc.) and one where the map menu is (Suspend, Retreat, ect.)
4. For the overworld menu use the adress for "EnterTownCheck.asm" for the usability, and "EnterTower.asm" for the effect.
Optional: You can change the 4F in the "EnterTower.asm" on line 18 to what ever Chapter/Map ID you want to load. Also you can change the 05 in "EnterTownCheck.asm" on line 12 to what ever world map node you want it to appear on.
5. For the map menu use the adress from "ExitSerafew.asm" for usability and the effect from the Retreat command in the effect.
If all goes right it should work, have fun.