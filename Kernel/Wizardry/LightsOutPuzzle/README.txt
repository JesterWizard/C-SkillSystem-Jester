Lights Out puzzle settings:
  - Four sizes: 3x3, 3x4, 4x4 and 4x5.
  - Either two or three icons.
  
InitPuzzleTiles determines the initial setup based on board size and icon count (four possible board sizes times two possible iconcounts means eight initial puzzle settings. Every setting consists of 20 tiles, of which the first X will be taken to make up the initial board, where X is 9 for a 3x3 board, 12 for a 3x4 board, etc). 

Which icons will appear is determined by NextRN_N. Only icons corresponding to weapons that haven't been obtained by previous puzzles will appear. So make sure that at least two or three weapons (depending on the iconcount) haven't been obtained yet when puzzle is triggered.

Idea is to award the weapon corresponding to the icon that fills the board to the player. Idea is to have six instances of these puzzles, where the final puzzle awards all three remaining weapons if all the previous puzzles have been completed, otherwise awards only the weapon whose icon filled the entire board. This entire paragraph refers to things that'll be handled with events and are unrelated to the implementation in this directory.


Puzzle LOCA uses objectType 0x1A. 0x1A is now reserved for puzzles.


TileAnimAffineSrc format: scalar x, scalar y, rotation angle, padding. Is used to animate the icons when they're being flipped.


Credits:
  Tiger Electronics made the original Lights Out game.

  BigWeaponIcons and their 16x16 counterparts were made by GabrielKnight.
  The cursor, and boards are from Yu-Gi-Oh Reshef of Destruction. Board ripped by Urathros. Cursor ripped by me. 
  I also modified the board.
  The song and sound effects (these two can be found in the music directory) are also from Yu-Gi-Oh Reshef of Destruction.
  
  I, Huichelaar, made the rest. Tell me if I forgot anyone!