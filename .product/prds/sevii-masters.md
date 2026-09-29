# Sevii Masters

Implemented: No
Specification: [Sevii Masters specification](../specs/sevii-masters.md)
Design status: v0 approved: the Sevii Masters is an off-the-record club for
champions, one of the three [Leagues](leagues.md), that calls the player once
they are a Master, with wins at both Indigo and Hoenn. It plays as a team
game: the player and a partner they asked by phone face four pairs from an
eight-strong lineup in tag battles, then the partner in a singles final.
There the aloof rule is off, and notable trainers who are Masters have
guaranteed seats. A notable trainer gives the player their phone number the
first time the player beats them. The Masters Gallery tallies every winner.
Balance is informational for now. Terms follow the
[glossary](player-trainer-rating.md#glossary).

## Intent

Indigo and Hoenn crown regional champions; the Sevii Masters is where those
champions find out who is actually best. It should feel like a secret
reward for having conquered both regions: a hidden club with its own rules,
where the best trainers in the world turn up and the player fights beside a
partner of their choosing, then against them. It shares the
[Leagues](leagues.md) machinery (invitations, league scores, halls, and
titles) and changes only what makes it the Masters.

## Design

**The club.** The Masters isn't an official league. It lives in a hidden
basement under Seven Island's battle house: an off-the-record club where
champions find out who is actually best. An old caretaker, the retired
trainer who runs the house, keeps the door. It gives no regional title,
Hall of Fame, or Ribbon.

**Masters.** A **Master** is anyone, the player or a notable trainer, who
has been champion at both Indigo and Hoenn at some point, whether or not
they reign there now. A Masters title never counts toward it. The game
remembers, for each notable trainer, whether they have ever reigned at
Indigo and at Hoenn ([Leagues](leagues.md#design)); once both are true,
they are a Master for good. The player's first wins at both Indigo and
Hoenn, in either order, make them a Master.

**When the Masters knows you.** The Masters calls only once the player is a
Master; until then it never calls. It then takes its turn with the other
leagues ([Leagues](leagues.md#design)).

**The lineup.** The Masters is elite by definition, the company aloof
trainers seek, so the aloof rule doesn't apply there: there is no base
lineup, and everyone may come. Its lineup has eight opponents, and the
partner is never one of them. Notable trainers who are Masters get
guaranteed seats; if more than eight are Masters, the eight with the highest
league scores sit. The remaining seats go to the highest league scores, as
anywhere. Fatigue still lowers everyone's league score, a Master's
included, and everyone is at home there. Being a Master gives no seat at
Indigo or Hoenn, and a Master who is the player's partner gives up their
seat. The eight are paired in battle order: the two weakest are the first
pair, the next two the second, and the two strongest the last pair.

**Tag battles.** The Masters plays as a team game. The player brings a
**partner**, and in each of the four themed halls the two of them fight a
pair from the lineup in a **tag battle**: two against two, each trainer with
three Pokémon. Pair 1 fights in the first hall, and so on, weakest first,
and each hall's condition applies as usual, covering both Pokémon on a side;
hazards greet all four leads. If the pair of them win all four, the
caretaker heals both sides fully, and the Champion's Room settles who is
best: a singles final between the player and their partner, each with their
full team.

**Bringing three.** Tag battles let each trainer bring only three Pokémon.
At each hall's door the player picks three from their current party, and
their Pokémon keep their damage from one hall to the next. Notable trainers,
opponent or partner, bring their best three, aces first. The final is a
full battle: after the heal, the player uses their whole party and the
partner their whole team.

**Phone numbers.** The first time the player beats a notable trainer, in any
battle with them, that trainer gives the player their phone number and joins
the phone's contacts for good. Tate and Liza, a duo, give none. Here a
contact is for one thing: asking them to be the player's partner.
[Notable haunts](notable-haunts.md) also treat contacts as friends.

**The partner.** The partner is whoever the player last asked. Any contact
can be asked once the player is a Master, and the ask always succeeds. The
player asks by phone, or in person when they meet the contact at a
[haunt](notable-haunts.md#design). If the player has never asked anyone,
Lorelei steps in: she spoke of the player to the caretaker, and she comes
along. The partner is never in the lineup, so asking a strong trainer also
keeps them off the other side. The partner is fixed when the player answers
the invitation; asking someone else later changes the next Masters event,
not this one.

**The title.** A player who loses the final hands the title to their
partner, who beat them; a loss in a hall, or leaving before the final,
crowns the lineup's strongest, as at any league. A Masters title never makes
anyone a Master.

**Winning.** Winning the Masters brings a Gallery win, prize money, and the
title, and no new status, first time or again. The **Masters Gallery** on
the basement wall tallies each Masters winner's wins, the player's or a
notable trainer's, including trainers who won the events the player
declined.

## Balance

Informational for now. The balance explorer shows, at the Masters, who has
reigned where, who is a Master, and which seats are guaranteed. It doesn't
show the tag format yet: its Masters lineups still have five seats and no
partner (Later). With the aloof rule off, the Champions come to the Masters
at any world progress: Lance (TR 200, level 100) attends from the Masters'
first call. The tag format isn't tuned either: in the halls the partner's
three stand beside the player's, and the pairs rise to the strongest two of
the eight, while the final pits the player against a partner they chose,
strong or not.

## Presentation

The Masters' call comes from the caretaker, not from an official league,
and her first call says that Lorelei, retired to Four Island, spoke of the
player. Lorelei is also the partner who steps in until the player asks
someone; otherwise she stays an ordinary notable trainer with no guaranteed
seat. The player asks a partner with a phone call to that trainer, from the
phone's contacts. Accepting a Masters invitation names the four pairs and the
partner, and in the lobby the caretaker introduces each pair before its hall.
In the tag battles the partner stands beside the player; in the final they
are the last opponent. The Gallery shows every winner's count.

## Sample playthrough

This continues the [Leagues sample](leagues.md#sample-playthrough): the
player holds all eight Kanto badges and all eight Hoenn badges, TR 120,
throughout, and fights each accepted event on the day of its call.

1. Day 7: the player beats Koga, Karen, Blue, Giovanni, and Jasmine at
   Indigo. Each of the five gives the player their phone number, as the Gym
   Leaders they beat for their badges already have (all but Tate & Liza,
   who give none).
2. Day 14: with Indigo and Hoenn both won, the player is a Master.
3. Day 21: the phone rings, and it is the caretaker of Seven Island's battle
   house: Lorelei spoke of the player, and there is a room downstairs. The
   player has never asked anyone, so Lorelei would be their partner. The
   aloof rule is off at the Masters, so the Champions come; the eight are
   Koga, Karen, Blue, Giovanni, Jasmine, Steven, Wallace, and Lance. The
   player declines. Lance takes the Masters title, and the Masters Gallery
   records his win. A Masters title doesn't make him a Master.
4. Day 42: the player declines again, and the Masters crowns Lance again.
   By day 49 Giovanni has reigned at both Indigo and Hoenn, so he is a
   Master for good.
5. Day 63: at the Masters, Giovanni's seat is guaranteed, though his league
   score would have put him in the top eight anyway. Lance wins the declined
   event and adds a Gallery win.
6. After day 77, before the next call, the player phones Blue, whose number
   they have held since day 7, and asks him to be their partner; he agrees.
7. Day 84: the Masters calls. Giovanni fought at Hoenn a week earlier, so he
   is tired: league score 65, too low for a seat on its own. His guaranteed
   seat still holds. Blue is the partner, so he is out of the lineup, and
   Erika takes the last seat. The pairs, weakest first, are Erika and Koga,
   Karen and Giovanni, Jasmine and Steven, and Wallace and Lance. The player
   accepts. At each hall's door they pick three, and their Pokémon carry
   their damage from hall to hall. Beside Blue they beat all four pairs, the
   last one, Wallace and Lance, under Karen's Hall's Wonder Room. The
   caretaker heals everyone, and in the Champion's Room the player beats
   Blue, full team against full team: a Gallery win for the player, with
   prize money and the Masters title, and no Hall of Fame, Ribbon, or
   regional title. Had Blue won the final, he would have taken the title.

## Boundaries

In: the Masters' identity and caretaker, the Master rule, when the Masters
knows the player, its lineup (eight seats, guaranteed Master seats, no aloof
rule, the partner left out, and pairs), the tag battles, picking three at
each door, the heal and the singles final, phone numbers, the partner asked
by phone, the Masters' title rules, and the Masters Gallery. The league
machinery it shares, including its halls, belongs to [Leagues](leagues.md).

Out of scope for v0: tag or double battles at Indigo or Hoenn, a partner who
can refuse, any use of phone numbers here beyond asking a partner, and any
status or reward for winning the Masters beyond its Gallery win, prize
money, and title.

## Open risks

- Partnering needs a back pic for every eligible notable trainer, about 37
  new sprites; the art cost is unresolved.

## Specifications

- [Sevii Masters specification](../specs/sevii-masters.md): the Master rule,
  phone contacts, the partner, the Masters' lineup changes and pairs, the
  title and Gallery, the tag matches, party choice and heal, the final,
  saved state, load validation, presentation, and acceptance.
- [Leagues specification](../specs/leagues.md): the shared league machinery
  and the halls.

## Later

- More of the Masters' story: the caretaker's later calls and lines, and
  what the Gallery shows beyond each winner's win count.
- Explorer support for the tag format: the partner, eight-seat Masters
  lineups, the pairs, and each notable's three for a tag battle.
- A friendship score with each contact, raised by partnering and other
  shared play.
- Picky partners: contacts who turn the player down, by friendship or by
  trait, such as an aloof trainer refusing a weak player.
- Gifts and trades with contacts.

## References

- [Leagues](leagues.md)
- [Notable trainers](notable-trainers.md)
- [Notable haunts](notable-haunts.md)
- [Player Trainer Rating](player-trainer-rating.md)
