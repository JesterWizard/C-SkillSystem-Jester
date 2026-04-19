# Misc skill effects

---
## post-action position fix

```
SID_PosReturn
SID_Lunge
```

Unit can change position on post-action stage. For allies, players can choose whether or not to use the skill effect if possible. For enemies, they will automatically enable the skill effect and change the position if possible.

---
## thunderstorm
```
SID_Thunderstorm
```

It may cause an AOE attack at the *2 * 2 circle* area of the target. This effect can only be activated if he successfully hitted the enemy.

---
## weather control skills
```
SID_Airlock
SID_Drought
SID_Drizzle
SID_SnowWarning
SID_SandStream
```

Skill above can sets the weather to a specific type at the beginning of the chapter. They have a priority order, with flags to track which weathers have been triggered. Weathers with lower priorities cannot be triggered once a higher priority weather has been. The order is as follows:

1. Airlock
2. Drought
3. Drizzle
4. SnowWarning
5. SandStream
