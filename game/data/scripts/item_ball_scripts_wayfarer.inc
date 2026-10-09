RocketHideout_B1F_EventScript_ItemEscapeRope::
	finditem ITEM_ESCAPE_ROPE
	end

RocketHideout_B1F_EventScript_ItemHyperPotion::
	finditem ITEM_HYPER_POTION
	end

RocketHideout_B2F_EventScript_ItemXSpeed::
	finditem ITEM_X_SPEED
	end

RocketHideout_B2F_EventScript_ItemMoonStone::
	finditem ITEM_MOON_STONE
	end

RocketHideout_B2F_EventScript_ItemTM12::
	finditem ITEM_TM12
	end

RocketHideout_B2F_EventScript_ItemSuperPotion::
	finditem ITEM_SUPER_POTION
	end

RocketHideout_B3F_EventScript_ItemRareCandy::
	finditem ITEM_RARE_CANDY
	end

RocketHideout_B3F_EventScript_ItemTM21::
	finditem ITEM_TM21
	end

RocketHideout_B3F_EventScript_ItemBlackGlasses::
	finditem ITEM_BLACK_GLASSES
	end

RocketHideout_B4F_EventScript_ItemTM49::
	finditem ITEM_TM49
	end

RocketHideout_B4F_EventScript_ItemMaxEther::
	finditem ITEM_MAX_ETHER
	end

RocketHideout_B4F_EventScript_ItemCalcium::
	finditem ITEM_CALCIUM
	end

PowerPlant_EventScript_ItemMaxPotion::
	finditem ITEM_MAX_POTION
	end

PowerPlant_EventScript_ItemTM17::
	finditem ITEM_TM_PROTECT
	end

PowerPlant_EventScript_ItemTM25::
	finditem ITEM_TM_THUNDER
	end

PowerPlant_EventScript_ItemThunderStone::
	finditem ITEM_THUNDER_STONE
	end

PowerPlant_EventScript_ItemElixir::
	finditem ITEM_ELIXIR
	end

@ The shared item ball script of the daily world slots (src/wayfarer_daily_slots.c).
@ Every item ball spot runs it. The native puts the ball's item in VAR_0x8000: the
@ prize while it is untaken, otherwise today's find. Std_FindItem leaves
@ VAR_0x8007 TRUE when the item went into the bag, and only then does the spot
@ count as picked up today. An untaken prize also sets its permanent flag (the
@ template keeps it); any other spot's template has none.
DailyItems_EventScript_Ball::
	callnative DailyItems_ResolveBall
	finditem VAR_0x8000, 1
	goto_if_eq VAR_0x8007, TRUE, DailyItems_EventScript_BallTaken
	end

DailyItems_EventScript_BallTaken::
	callnative DailyItems_MarkBallCleared
	end
