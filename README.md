<!-- markdownlint-disable MD033 MD041 -->
<p align="center">
  <img src="./Documentation/Images/Background.jpg" alt="FE8U C-SkillSystem" width="720" />
</p>

<h1 align="center">FE8U C-SkillSystem</h1>

<p align="center">
  <b>A modern, C-based buildfile for Fire Emblem: The Sacred Stones (FE8U)</b><br/>
  Hundreds of skills and dozens of new systems, written in C and toggled at runtime.
</p>

<p align="center">
  <a href="./Documentation/Setup.md">Setup Guide</a> ·
  <a href="./Documentation/CONTRIBUTING.md">Contributing</a> ·
  <a href="https://feuniverse.us/t/fe8-modern-c-skillsystem-release/24614">FEUniverse Thread</a> ·
  <a href="./Data/DesignerConfig/designer-config.c">Runtime Config</a> ·
  <a href="#-guides--reference">Guides</a>
</p>

---

## ✨ What is this?

This buildfile turns FE8U into a playground for new mechanics. It started from Mokha's
[master C-SkillSystem kernel](https://github.com/MokhaLeee/fe8u-cskillsys-kernel/) and has been heavily extended with
the features below.

- **900+ skills** that units get from their character, class, items, weapons and level-ups.
- **New gameplay systems:** pair-ups, gambits, gaiden magic, MP, shields, real-time battle, free movement and more.
- **New menus and screens:** base chapters, skill trees, augury, infuse, skill synthesis, a scrollback chatlog and more.
- **Towns and minigames:** explorable districts, fishing, River Crossing, Tower Stack, the Hex card game and Snek.
- **A campaign you can play to the end:** a world map driven by the story, placeholder battles for the chapters not
  written yet (Chapters 10–22), the ending and the postgame.
- **Runtime toggles:** about 180 designer-config switches in
  [`designer-config.c`](./Data/DesignerConfig/designer-config.c) turn features on or off without touching their code.
- **Readable code:** game logic is written in C and built with devkitARM, `lyn` and Event Assembler.

## 🚀 Getting started

1. Follow the [Setup Guide](./Documentation/Setup.md) to install devkitPro and the tools.
2. Put a clean FE8U ROM named `fe8.gba` at the repository root.
3. Run `make` to build `fe8-kernel-dev.gba`.
4. Turn features on or off in [`designer-config.c`](./Data/DesignerConfig/designer-config.c)
   (see [System Config](./Documentation/Features/SystemConfig.md) for what each switch does).

## 🎮 Features

Every feature links to its documentation. Click a preview to open it.

### ⚔️ Battle & Combat

<table>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/AnimaTriangle.md"><img src="./Documentation/Gifs/Anima_Triangle.gif" alt="Anima Triangle preview" width="360" /></a><br/><b><a href="./Documentation/Features/AnimaTriangle.md">Anima Triangle</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/BanimFeatures.md"><img src="./Documentation/Gifs/Banim_Switcher.gif" alt="Battle Animations preview" width="360" /></a><br/><b><a href="./Documentation/Features/BanimFeatures.md">Battle Animations</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/Biorhythm.md"><img src="./Documentation/Gifs/Biorhythm.gif" alt="Biorhythm preview" width="360" /></a><br/><b><a href="./Documentation/Features/Biorhythm.md">Biorhythm</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/CombatArt.md"><img src="./Documentation/Images/Home_CombatArt1.png" alt="Combat Art preview" width="360" /></a><br/><b><a href="./Documentation/Features/CombatArt.md">Combat Art</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/CritCutin.md"><img src="./Documentation/Gifs/Crit_Cutin.gif" alt="Critical Cut-in preview" width="360" /></a><br/><b><a href="./Documentation/Features/CritCutin.md">Critical Cut-in</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/DynamicWeaponSlots.md"><img src="./Documentation/Gifs/Dynamic_Weapon_Slots.gif" alt="Dynamic Weapon Slots preview" width="360" /></a><br/><b><a href="./Documentation/Features/DynamicWeaponSlots.md">Dynamic Weapon Slots</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/ExpandedHP.md"><img src="./Documentation/Gifs/Expanded_HP.gif" alt="Expanded HP preview" width="360" /></a><br/><b><a href="./Documentation/Features/ExpandedHP.md">Expanded HP</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/GaidenMagic.md"><img src="./Documentation/Gifs/Gaiden_Magic.gif" alt="Gaiden Magic preview" width="360" /></a><br/><b><a href="./Documentation/Features/GaidenMagic.md">Gaiden Magic</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/KillRewards.md"><img src="./Documentation/Gifs/Kill_Rewards.gif" alt="Kill Rewards preview" width="360" /></a><br/><b><a href="./Documentation/Features/KillRewards.md">Kill Rewards</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/LaguzBars.md"><img src="./Documentation/Gifs/Laguz_Bars.gif" alt="Laguz Bars preview" width="360" /></a><br/><b><a href="./Documentation/Features/LaguzBars.md">Laguz Bars</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/MagicSeal.md"><img src="./Documentation/Gifs/Magic_Seal.gif" alt="Magic Seal preview" width="360" /></a><br/><b><a href="./Documentation/Features/MagicSeal.md">Magic Seal</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/MokhaAOE.md"><img src="./Documentation/Gifs/Mokha_Gambits.gif" alt="Mokha Gambits preview" width="360" /></a><br/><b><a href="./Documentation/Features/MokhaAOE.md">Mokha Gambits</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/MpSystem.md"><img src="./Documentation/Gifs/MP_System.gif" alt="MP System preview" width="360" /></a><br/><b><a href="./Documentation/Features/MpSystem.md">MP System</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/PairUp.md"><img src="./Documentation/Gifs/Pair_Up.gif" alt="Pair-Up Commands preview" width="360" /></a><br/><b><a href="./Documentation/Features/PairUp.md">Pair-Up Commands</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/RealTimeBattle.md"><img src="./Documentation/Gifs/Real_Time_Battle.gif" alt="Real-Time Battle preview" width="360" /></a><br/><b><a href="./Documentation/Features/RealTimeBattle.md">Real-Time Battle</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/ShieldItem.md"><img src="./Documentation/Gifs/Shield_Item.gif" alt="Shield Item preview" width="360" /></a><br/><b><a href="./Documentation/Features/ShieldItem.md">Shield Item</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/Tonics.md"><img src="./Documentation/Gifs/Item_Tonics.gif" alt="Tonics preview" width="360" /></a><br/><b><a href="./Documentation/Features/Tonics.md">Tonics</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/WeaponRankSkills.md"><img src="./Documentation/Gifs/Weapon_Rank_Skills.gif" alt="Weapon Rank Skills preview" width="360" /></a><br/><b><a href="./Documentation/Features/WeaponRankSkills.md">Weapon Rank Skills</a></b></td>
  </tr>
</table>

### 🪄 Staves

<table>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/CustomStaves/ArbalestStaff.md"><img src="./Documentation/Gifs/Staff_Arbalest.gif" alt="Arbalest Staff preview" width="360" /></a><br/><b><a href="./Documentation/Features/CustomStaves/ArbalestStaff.md">Arbalest Staff</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/CustomStaves/AumStaff.md"><img src="./Documentation/Gifs/Staff_Aum.gif" alt="Aum Staff preview" width="360" /></a><br/><b><a href="./Documentation/Features/CustomStaves/AumStaff.md">Aum Staff</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/CustomStaves/PhoenixStaff.md"><img src="./Documentation/Gifs/Staff_Phoenix.gif" alt="Phoenix Staff preview" width="360" /></a><br/><b><a href="./Documentation/Features/CustomStaves/PhoenixStaff.md">Phoenix Staff</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/CustomStaves/SkillStaff.md"><img src="./Documentation/Gifs/Staff_Skill.gif" alt="Skill Staff preview" width="360" /></a><br/><b><a href="./Documentation/Features/CustomStaves/SkillStaff.md">Skill Staff</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/ModularStaffEXP.md"><img src="./Documentation/Gifs/Modular_Staff_EXP.gif" alt="Modular Staff EXP preview" width="360" /></a><br/><b><a href="./Documentation/Features/ModularStaffEXP.md">Modular Staff EXP</a></b></td>
    <td width="50%"></td>
  </tr>
</table>

### 🗺️ Maps, Fog & AI

<table>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/AIRescueRetreat.md"><img src="./Documentation/Gifs/AI_Rescue_Retreat.gif" alt="AI Rescue Retreat preview" width="360" /></a><br/><b><a href="./Documentation/Features/AIRescueRetreat.md">AI Rescue Retreat</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/ChapterGoals/Escape.md"><img src="./Documentation/Gifs/Goal_Escape.gif" alt="Chapter Goal: Escape preview" width="360" /></a><br/><b><a href="./Documentation/Features/ChapterGoals/Escape.md">Chapter Goal: Escape</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/ChapterGoals/Timer.md"><img src="./Documentation/Gifs/Goal_Timer.gif" alt="Chapter Goal: Timer preview" width="360" /></a><br/><b><a href="./Documentation/Features/ChapterGoals/Timer.md">Chapter Goal: Timer</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/EnemiesPromoteWhenKillingUnit.md"><img src="./Documentation/Gifs/Enemies_Promote_When_Killing_Unit.gif" alt="Enemies Promote When Killing Unit preview" width="360" /></a><br/><b><a href="./Documentation/Features/EnemiesPromoteWhenKillingUnit.md">Enemies Promote When Killing Unit</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/EnemyFogVision.md"><img src="./Documentation/Gifs/Enemy_Fog_Vision.gif" alt="Enemy Fog Vision preview" width="360" /></a><br/><b><a href="./Documentation/Features/EnemyFogVision.md">Enemy Fog Vision</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/FE7FancyTeleport.md"><img src="./Documentation/Gifs/FE7FancyTeleport.gif" alt="FE7 Fancy Teleport preview" width="360" /></a><br/><b><a href="./Documentation/Features/FE7FancyTeleport.md">FE7 Fancy Teleport</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/FogStages.md"><img src="./Documentation/Gifs/Fog_Stages.gif" alt="Fog Stages preview" width="360" /></a><br/><b><a href="./Documentation/Features/FogStages.md">Fog Stages</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/FogVision.md"><img src="./Documentation/Gifs/Fog_Vision.gif" alt="Fog Vision preview" width="360" /></a><br/><b><a href="./Documentation/Features/FogVision.md">Fog Vision</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/FreeMovement.md"><img src="./Documentation/Gifs/Free_Movement.gif" alt="Free Movement preview" width="360" /></a><br/><b><a href="./Documentation/Features/FreeMovement.md">Free Movement</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/MenuSkillAI.md"><img src="./Documentation/Gifs/Menu_Skills_AI.gif" alt="Menu Skill AI preview" width="360" /></a><br/><b><a href="./Documentation/Features/MenuSkillAI.md">Menu Skill AI</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/StartMapEffects.md"><img src="./Documentation/Gifs/Start_Map_Effects.gif" alt="Start Map Effects preview" width="360" /></a><br/><b><a href="./Documentation/Features/StartMapEffects.md">Start Map Effects</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/TranslucentUnitSprite.md"><img src="./Documentation/Gifs/Translucent_Unit_Sprite.gif" alt="Translucent Unit Sprite preview" width="360" /></a><br/><b><a href="./Documentation/Features/TranslucentUnitSprite.md">Translucent Unit Sprite</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/TrapRework.md"><img src="./Documentation/Gifs/Trap_Rework.gif" alt="Trap Rework preview" width="360" /></a><br/><b><a href="./Documentation/Features/TrapRework.md">Trap Rework</a></b></td>
    <td width="50%"></td>
  </tr>
</table>

### 📈 Growth & Progression

<table>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/AllocateStatPoints.md"><img src="./Documentation/Gifs/Allocate_Stat_Points.gif" alt="Allocate Stat Points preview" width="360" /></a><br/><b><a href="./Documentation/Features/AllocateStatPoints.md">Allocate Stat Points</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/ArenaRoster.md"><img src="./Documentation/Gifs/Arena_Awards.gif" alt="Arena Roster preview" width="360" /></a><br/><b><a href="./Documentation/Features/ArenaRoster.md">Arena Roster</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/BonusEXPMenu.md"><img src="./Documentation/Gifs/Bonus_EXP_Menu.gif" alt="Bonus EXP Menu preview" width="360" /></a><br/><b><a href="./Documentation/Features/BonusEXPMenu.md">Bonus EXP Menu</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/ClassGrowths.md"><img src="./Documentation/Gifs/Class_Growths.gif" alt="Class Growths preview" width="360" /></a><br/><b><a href="./Documentation/Features/ClassGrowths.md">Class Growths</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/InfuseMenu.md"><img src="./Documentation/Gifs/Infuse_Menu.gif" alt="Infuse Menu preview" width="360" /></a><br/><b><a href="./Documentation/Features/InfuseMenu.md">Infuse Menu</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/Prestige.md"><img src="./Documentation/Gifs/Prestige.gif" alt="Prestige preview" width="360" /></a><br/><b><a href="./Documentation/Features/Prestige.md">Prestige</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/SkillSynthesis.md"><img src="./Documentation/Gifs/Skill_Synthesis.gif" alt="Skill Synthesis Menu preview" width="360" /></a><br/><b><a href="./Documentation/Features/SkillSynthesis.md">Skill Synthesis Menu</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/SkillTree.md"><img src="./Documentation/Gifs/Skill_Tree.gif" alt="Skill Tree preview" width="360" /></a><br/><b><a href="./Documentation/Features/SkillTree.md">Skill Tree</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/TelliusSkillCapacity.md"><img src="./Documentation/Gifs/Tellius_Skill_Capacity.gif" alt="Tellius Skill Capacity preview" width="360" /></a><br/><b><a href="./Documentation/Features/TelliusSkillCapacity.md">Tellius Skill Capacity</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/TimingStatLevelUp.md"><img src="./Documentation/Gifs/Timing_Stat_System.gif" alt="Timing Stat Level-Up preview" width="360" /></a><br/><b><a href="./Documentation/Features/TimingStatLevelUp.md">Timing Stat Level-Up</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/TraineeInChapterPromotion.md"><img src="./Documentation/Gifs/Trainee_In_Chapter_Promotion.gif" alt="Trainee In-Chapter Promotion preview" width="360" /></a><br/><b><a href="./Documentation/Features/TraineeInChapterPromotion.md">Trainee In-Chapter Promotion</a></b></td>
    <td width="50%"></td>
  </tr>
</table>

### 🧭 Menus, Screens & UI

<table>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/AuguryMenu.md"><img src="./Documentation/Gifs/Augury_Menu.gif" alt="Augury Menu preview" width="360" /></a><br/><b><a href="./Documentation/Features/AuguryMenu.md">Augury Menu</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/BaseChapters.md"><img src="./Documentation/Gifs/Base_Chapters.gif" alt="Base Chapters preview" width="360" /></a><br/><b><a href="./Documentation/Features/BaseChapters.md">Base Chapters</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/BaseConversationsMenu.md"><img src="./Documentation/Gifs/Base_Conversations.gif" alt="Base Conversations Menu preview" width="360" /></a><br/><b><a href="./Documentation/Features/BaseConversationsMenu.md">Base Conversations Menu</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/CharacterBiographies.md"><img src="./Documentation/Gifs/Character_Biography_Menu.gif" alt="Character Biographies preview" width="360" /></a><br/><b><a href="./Documentation/Features/CharacterBiographies.md">Character Biographies</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/ConversationViewer.txt"><img src="./Documentation/Gifs/Conversation_Viewer.gif" alt="Conversation Viewer preview" width="360" /></a><br/><b><a href="./Documentation/Features/ConversationViewer.txt">Conversation Viewer</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/KonamiCode.md"><img src="./Documentation/Gifs/Konami_Code.gif" alt="Konami Code preview" width="360" /></a><br/><b><a href="./Documentation/Features/KonamiCode.md">Konami Code</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/LanguageSelect.md"><img src="./Documentation/Gifs/Language_Select.gif" alt="Language Select preview" width="360" /></a><br/><b><a href="./Documentation/Features/LanguageSelect.md">Language Select</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/LimitedShopStock.md"><img src="./Documentation/Gifs/Limited_Shop_Stock.gif" alt="Limited Shop Stock preview" width="360" /></a><br/><b><a href="./Documentation/Features/LimitedShopStock.md">Limited Shop Stock</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/MenuOptions.md"><img src="./Documentation/Gifs/Menu_Options.gif" alt="Menu Options preview" width="360" /></a><br/><b><a href="./Documentation/Features/MenuOptions.md">Menu Options</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/PaginatedHelpBoxes.md"><img src="./Documentation/Gifs/Paginated_Help_Boxes.gif" alt="Paginated Help Boxes preview" width="360" /></a><br/><b><a href="./Documentation/Features/PaginatedHelpBoxes.md">Paginated Help Boxes</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/PopupItemIconShine.md"><img src="./Documentation/Gifs/Popup_Item_Icon_Shine.gif" alt="Popup Item Icon Shine preview" width="360" /></a><br/><b><a href="./Documentation/Features/PopupItemIconShine.md">Popup Item Icon Shine</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/QualityOfLife.md"><img src="./Documentation/Gifs/Quality_Of_Life.gif" alt="Quality Of Life preview" width="360" /></a><br/><b><a href="./Documentation/Features/QualityOfLife.md">Quality Of Life</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/StatScreenChapterLocks.md"><img src="./Documentation/Gifs/Stat_Screen_Chapter_Locks.gif" alt="Stat Screen Chapter Locks preview" width="360" /></a><br/><b><a href="./Documentation/Features/StatScreenChapterLocks.md">Stat Screen Chapter Locks</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/StatScreenPromotions.md"><img src="./Documentation/Gifs/Promotion_Stat_Page.gif" alt="Stat Screen Promotions preview" width="360" /></a><br/><b><a href="./Documentation/Features/StatScreenPromotions.md">Stat Screen Promotions</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/UnlimitedPrepMenuOptions.md"><img src="./Documentation/Gifs/Unlimited_Prep_Menu_Options.gif" alt="Unlimited Prep Menu Options preview" width="360" /></a><br/><b><a href="./Documentation/Features/UnlimitedPrepMenuOptions.md">Unlimited Prep Menu Options</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/VariableCharacterDescriptions.md"><img src="./Documentation/Gifs/Variable_Character_Descriptions.gif" alt="Variable Character Descriptions preview" width="360" /></a><br/><b><a href="./Documentation/Features/VariableCharacterDescriptions.md">Variable Character Descriptions</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/VeslyUIStyles.md"><img src="./Documentation/Gifs/VeslyUI_Styles.gif" alt="VeslyUI Styles preview" width="360" /></a><br/><b><a href="./Documentation/Features/VeslyUIStyles.md">VeslyUI Styles</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/ExpBarStyles.md"><img src="./Documentation/Images/ExpBarStyles.png" alt="EXP Bar Styles preview" width="360" /></a><br/><b><a href="./Documentation/Features/ExpBarStyles.md">EXP Bar Styles</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/MapColors.md"><img src="./Documentation/Images/MapColors.png" alt="Map Colors preview" width="360" /></a><br/><b><a href="./Documentation/Features/MapColors.md">Map Colors</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/NewGamePlus.md"><img src="./Documentation/Images/NewGamePlus.png" alt="New Game+ preview" width="360" /></a><br/><b><a href="./Documentation/Features/NewGamePlus.md">New Game+</a></b><br/><sub>Extras > New Game+: a new game where each character keeps their equipped skills and skill list</sub></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/Gallery.md"><img src="./Documentation/Images/Gallery.png" alt="Gallery preview" width="360" /></a><br/><b><a href="./Documentation/Features/Gallery.md">Gallery</a></b><br/><sub>Extras > Gallery: the CGs seen in game, 3x3 to a page, and full size</sub></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/Bestiary.md"><img src="./Documentation/Images/Bestiary.png" alt="Bestiary preview" width="360" /></a><br/><b><a href="./Documentation/Features/Bestiary.md">Bestiary</a></b><br/><sub>Extras > Bestiary: every character and class met, with their bases, growths and descriptions</sub></td>
  </tr>
</table>

### 💬 Story, Text & World Map

<table>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/Chatlog.md"><img src="./Documentation/Gifs/Chatlog.gif" alt="Chatlog preview" width="360" /></a><br/><b><a href="./Documentation/Features/Chatlog.md">Chatlog</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/CustomBattleQuotes.md"><img src="./Documentation/Gifs/Battle_Quotes.gif" alt="Custom Battle Quotes preview" width="360" /></a><br/><b><a href="./Documentation/Features/CustomBattleQuotes.md">Custom Battle Quotes</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/CustomDefeatQuotes.md"><img src="./Documentation/Gifs/Defeat_Quotes.gif" alt="Custom Defeat Quotes preview" width="360" /></a><br/><b><a href="./Documentation/Features/CustomDefeatQuotes.md">Custom Defeat Quotes</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Standalone/gameover_quotes/README.md"><img src="./Documentation/Gifs/Game_Over_Quotes.gif" alt="Game Over Quotes preview" width="360" /></a><br/><b><a href="./Standalone/gameover_quotes/README.md">Game Over Quotes</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/LevelUpQuotes.md"><img src="./Documentation/Gifs/Level_Up_Quotes.gif" alt="Level Up Quotes preview" width="360" /></a><br/><b><a href="./Documentation/Features/LevelUpQuotes.md">Level Up Quotes</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/TextEngineRework.md"><img src="./Documentation/Gifs/Text_Engine_Rework.gif" alt="Text Engine Rework preview" width="360" /></a><br/><b><a href="./Documentation/Features/TextEngineRework.md">Text Engine Rework</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/UnitSelectionQuotes.md"><img src="./Documentation/Gifs/Unit_Selection_Quotes.gif" alt="Unit Selection Quotes preview" width="360" /></a><br/><b><a href="./Documentation/Features/UnitSelectionQuotes.md">Unit Selection Quotes</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/WorldMapEditor.md"><img src="./Documentation/Gifs/World_Map_Story_Chapters.gif" alt="World Map Editor preview" width="360" /></a><br/><b><a href="./Documentation/Features/WorldMapEditor.md">World Map Editor</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/WorldMapSkillShop.md"><img src="./Documentation/Gifs/WorldMap_SkillShop.gif" alt="World Map Skill Shop preview" width="360" /></a><br/><b><a href="./Documentation/Features/WorldMapSkillShop.md">World Map Skill Shop</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/WorldMapStoryChapters.md"><img src="./Documentation/Gifs/World_Map_Story_Chapters.gif" alt="World Map Story Chapters preview" width="360" /></a><br/><b><a href="./Documentation/Features/WorldMapStoryChapters.md">World Map Story Chapters</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/WorldMapThoughts.md"><img src="./Documentation/Gifs/World_Map_Thoughts.gif" alt="World Map Thoughts preview" width="360" /></a><br/><b><a href="./Documentation/Features/WorldMapThoughts.md">World Map Thoughts</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/StoryStubs.md"><img src="./Documentation/Gifs/Story_Stubs.gif" alt="Story Stubs preview" width="360" /></a><br/><b><a href="./Documentation/Features/StoryStubs.md">Story Stubs</a></b><br/><sub>Placeholder battles to the end, the postgame, and <code>new_chapter.py</code></sub></td>
  </tr>
</table>

### 🎨 Portraits & Graphics

<table>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/Portrait32Color.md"><img src="./Documentation/Gifs/Portrait_32_Color.gif" alt="32-Color Portraits preview" width="360" /></a><br/><b><a href="./Documentation/Features/Portrait32Color.md">32-Color Portraits</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/ExceedPortraitBox.md"><b><a href="./Documentation/Features/ExceedPortraitBox.md">Exceed Portrait Box</a></b></a><br/><sub>No preview yet</sub></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/Halfbodies.md"><img src="./Documentation/Gifs/Halfbodies.gif" alt="Halfbodies preview" width="360" /></a><br/><b><a href="./Documentation/Features/Halfbodies.md">Halfbodies</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/MaxColorBackgrounds.md"><img src="./Documentation/Gifs/Max_Color_Backgrounds.gif" alt="Max Color Backgrounds preview" width="360" /></a><br/><b><a href="./Documentation/Features/MaxColorBackgrounds.md">Max Color Backgrounds</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/PortraitExpressions.md"><img src="./Documentation/Gifs/Portrait_Expressions.gif" alt="Portrait Expressions preview" width="360" /></a><br/><b><a href="./Documentation/Features/PortraitExpressions.md">Portrait Expressions</a></b></td>
    <td></td>
  </tr>
</table>

### 🏘️ Towns & Minigames

<table>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/Districts.md"><img src="./Documentation/Gifs/Districts.gif" alt="Districts preview" width="360" /></a><br/><b><a href="./Documentation/Features/Districts.md">Districts</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/Minigames/Fishing.md"><img src="./Documentation/Gifs/District_Fishing.gif" alt="District Fishing preview" width="360" /></a><br/><b><a href="./Documentation/Features/Minigames/Fishing.md">District Fishing</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/Minigames/Hex.md"><img src="./Documentation/Gifs/Hex_Card_Minigame.gif" alt="Hex Card Minigame preview" width="360" /></a><br/><b><a href="./Documentation/Features/Minigames/Hex.md">Hex Card Minigame</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/Minigames/Crossing.md"><img src="./Documentation/Gifs/River_Crossing.gif" alt="River Crossing preview" width="360" /></a><br/><b><a href="./Documentation/Features/Minigames/Crossing.md">River Crossing</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/Minigames/Snek.md"><img src="./Documentation/Gifs/Minigame_Snek.gif" alt="Snek preview" width="360" /></a><br/><b><a href="./Documentation/Features/Minigames/Snek.md">Snek</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/Minigames/Tower.md"><img src="./Documentation/Gifs/Tower_Stack.gif" alt="Tower Stack preview" width="360" /></a><br/><b><a href="./Documentation/Features/Minigames/Tower.md">Tower Stack</a></b></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/GuildBoard.md"><img src="./Documentation/Gifs/Guild_Board.gif" alt="Guild Board preview" width="360" /></a><br/><b><a href="./Documentation/Features/GuildBoard.md">Guild Board</a></b></td>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/Districts.md#entering"><img src="./Documentation/Gifs/District_Flags.gif" alt="District Flags preview" width="360" /></a><br/><b><a href="./Documentation/Features/Districts.md#entering">District Flags</a></b><br/><sub>Green flags on the world map and minimap mark open districts</sub></td>
  </tr>
  <tr>
    <td align="center" valign="top" width="50%"><a href="./Documentation/Features/BigUnits.md"><img src="./Documentation/Gifs/Big_Units.gif" alt="2x2 Units preview" width="360" /></a><br/><b><a href="./Documentation/Features/BigUnits.md">2x2 Units</a></b><br/><sub>Bosses and monsters on four tiles, drawn at double size, with an HP bar per corner</sub></td>
    <td></td>
  </tr>
</table>


## 📚 Guides & reference

| Guide | What it covers |
|---|---|
| [Overview](./Documentation/Features/Abstract.md) | A high-level map of the kernel's systems |
| [How To Make A Chapter](./Documentation/Features/HowToMakeAChapter.md) | Building a chapter in C, step by step |
| [Story Stubs](./Documentation/Features/StoryStubs.md) | The placeholder chapters, and turning one into a real chapter with `Tools/chapter/new_chapter.py` |
| [World Map Story Chapters](./Documentation/Features/WorldMapStoryChapters.md) | How the story picks its next chapter and node, and the postgame |
| [System Config](./Documentation/Features/SystemConfig.md) | Every runtime toggle in the designer config |
| [Wizardry](./Documentation/Features/Wizardry.md) | How the kernel's code modules are installed |
| [Skill System](./Documentation/Features/SkillSys.md) | How units, classes and items get skills |
| [Skills Glossary](./Documentation/Features/SkillInfo.md) | Every skill with its icon and description |
| [Battle System](./Documentation/Features/BattleSystem.md) | The battle call stack and where it is hooked |
| [Item System](./Documentation/Features/ItemSys.md) | Item usability and effects (IER) |
| [Debuffs](./Documentation/Features/Debuff.md) | The expanded unit status system |
| [Misc Skill Effects](./Documentation/Features/MiscSkillEffect.md) | Notes on skill effects outside battle |
| [Popup System](./Documentation/Features/PopupR.md) | How skill and item popups are queued |
| [Save System](./Documentation/Features/Save.md) | The save data layout |
| [Save Data](./Documentation/FeatureTypes/SaveData.md) | Adding a save chunk, and the layout guard that refuses saves from another layout |
| [Cloud Build & Smoke Tests](./Tools/cloud/README.md) | Building in a cloud session, and the emulator smoke tests (`smoketest.py`, run side by side with `--jobs`) |
| [Skill Icon Palettes](./Documentation/Features/SkillIconPalettes.md) | Palette banks for skill icons |
| [Fonts](./Documentation/Features/Fonts.md) | Choosing a font with CONFIG_FONT_* |
| [Voice Acted Intros](./Documentation/Features/VoiceActedIntros.md) | Installing WAV voice lines |
| [Credits](./Documentation/Features/Credits.md) | Who made what |

## 🤝 Community & contributing

- Read [How to contribute](./Documentation/CONTRIBUTING.md) before opening a pull request.
- Join the discussion on the [FEUniverse thread](https://feuniverse.us/t/fe8-modern-c-skillsystem-release/24614).
- See [Credits](./Documentation/Features/Credits.md) for everyone whose work is in this buildfile.
<!-- markdownlint-enable MD033 MD041 -->
