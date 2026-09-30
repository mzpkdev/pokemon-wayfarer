# Notable trainer voice bits (review draft)

Voice bits are short reusable lines for the 38 notable trainers in
`devtools/ui/src/modules/trainer-balance/catalog.json`.
[Notable haunts](../prds/notable-haunts.md) splice them into generic dialogue
at overworld meeting spots, so one haunt script works for any trainer. The
haunt line carries the facts of the place and its activity. The trainer's bits
carry personality.

Status: review draft. Nothing here is wired into scripts yet.

## Bits

Every trainer has all 15. A haunt picks the greeting by the trainer's
[friendship stage](../specs/notable-trainers.md#friendship): `MEET` or
`HEARD` for a stranger, `AGAIN` at Met, `HELLO` at Friend, and `CLOSE` at
Close.

| Bit | When |
| --- | --- |
| `HELLO` | greets a friend |
| `AGAIN` | greets a player they have met before but are not friends with yet |
| `CLOSE` | a warmer greeting for a close friend |
| `MEET` | introducing themselves the first time (includes their name) |
| `HEARD` | a first meeting, but the player's fame has reached them (see the haunts spec); always contains `{PLAYER}` |
| `NOT_YET` | first fight still ahead: "beat me properly first". Gym Leaders may mention their GYM; Elite Four and Champions don't |
| `NEWS` | lead-in before a gossip line |
| `ASK` | attention-getter before whatever the haunt proposes: no movement, favour, request content or implied destination or activity |
| `YES` | the player agreed |
| `NO` | the player declined |
| `NOT_READY` | the player doesn't meet a requirement yet |
| `PRAISE` | the player did well |
| `GIFT` | handing over an item; always contains `{ITEM}` |
| `BYE` | farewell |
| `QUIRK` | a signature aside showing their character |

## Trainer values

Each trainer also carries a buddy value that haunts read. It is authored in
the notable-trainer catalog, and the
[haunts spec](../specs/notable-haunts.md#trainer-values) owns its meaning.
Personality lives only in the voice bits: haunt lines never imply the trainer
needs help or is asking a favour, so every trainer can voice every quest.

- **Buddy** names one roster slot: the slot number (1-6, as in the catalog
  spec) and the species as authored. It is the trainer's anime companion, or
  their later-game companion, or else their iconic ace. `{BUDDY}` resolves at
  runtime to that slot's current species after the
  [downward rule](../specs/player-trainer-rating.md#evolution-stages), even
  before the slot has joined the team. So Brock's buddy is ONIX early in the
  journey and STEELIX later.

## Rules

- **Haunt-safe.** No places, map features, times or weather. No claims about
  the player's team. No type claims about the location. The trainer's own
  team or type is fine.
- **Named POKéMON** appear only when the species is on that trainer's catalog
  roster. The buddy is never named: lines use `{BUDDY}` and must read
  correctly at every stage ("{BUDDY} gets fussy if I burn the rice" works for
  ONIX and STEELIX). Other species stay generic or use `{ACE}`.
- **Nicknames** are allowed when canon has one. Jasmine's AMPHY is the only
  nickname used; it reads for every stage and is noted in her section.
- **Slots:** `{PLAYER}`, `{ITEM}`, `{ACE}` (the signature POKéMON's current
  species) and `{BUDDY}`. `{GOSSIP}` belongs to haunt dialogue and follows
  `NEWS`. Trainers write their own name out in `MEET`, so there is no name
  slot.
- **Length:** each line fits one GBA text box, 2 lines of about 36
  characters, so 70 characters at most, counting `{PLAYER}` as 7 characters,
  `{ITEM}` as 16 (the longest reward-pool item name), and `{ACE}` and `{BUDDY}` as 10. Line breaks are left to
  the text formatter. Game text style: POKéMON, GYM, names in caps.
- **Revisits.** A haunt can always be revisited while the trainer is placed
  there, so "come back later" lines are fine.
- **Tone:** dated or uncomfortable gags are left out. Villains stay
  villainous.
- **ASK is only an attention-getter before whatever the haunt proposes.** No
  movement (come, walk, go, follow, along, with me), no favour (help, hand,
  favor, assist), no request content, no implied destination or activity. It
  must read naturally before any haunt proposal, e.g. "...These tunnels run
  through to Vermilion. Walk it with me?", "...Something valuable went missing
  around here. Find it?", "...Your best one against my {ACE}?"
- **YES is pure approval of the player's answer.** No movement (go, move
  out, let's go, come, walk, follow), no timing (later, when you're done,
  after) and no assumed activity (meal, battle, search, walk, quiz). It must
  read naturally after any haunt proposal, including one where the trainer
  walks with the player. `NO`, `NOT_READY` and `PRAISE` likewise assume no
  specific activity, movement or place. Their mood and the trainer's own
  hooks (cooking, Water types, war stories) are fine.
- **Greetings by friendship.** `HEARD` knows the player only by fame,
  so it makes no claim about what they did and always holds `{PLAYER}`.
  `AGAIN` is polite and still getting acquainted; `CLOSE` is the warmest
  greeting. All three follow the haunt-safe rules above and assume no
  activity.
- **Tate & Liza** alternate halves of every line ("TATE: … LIZA: …").

## Source priority

1. **Anime**: voice, quirks, running gags, catchphrases.
2. **Later games**, where characters are more fleshed out: HGSS, FRLG, ORAS,
   Emerald, Let's Go, Masters EX, BW2 PWT and cameos. Original-generation
   game text (RBY, GSC, RSE) counts here too.
3. **Manga** (Pokémon Adventures) as a fallback.

Each trainer's **Hooks** note names the one or two canon hooks the lines keep
coming back to, plus where each comes from. `(verify)` marks anything I'm not
fully sure of: the hook is probably right, but the source or exact wording
needs checking before the lines ship.

---

## Kanto

### Brock

Buddy: slot 1, Steelix (ONIX early). Anime Onix.

| Bit | Line |
| --- | --- |
| HELLO | Hey, {PLAYER}! Good to see you. Eating well, I hope? |
| AGAIN | Oh, hi again! Still getting to know each other, huh? |
| CLOSE | {PLAYER}! Good to see you, friend. There is always a plate for you. |
| MEET | I'm BROCK. I train Rock types, and I cook for them too! |
| HEARD | So you're {PLAYER}? Word's out! Have you eaten yet? |
| NOT_YET | Test your rock-hard will at my GYM first. Then we'll talk. |
| NEWS | Word travels fast between breeders. Listen to this... |
| ASK | Hey, I've got an idea. |
| YES | Great! That's the spirit! |
| NO | No worries. Just don't skip any meals, okay? |
| NOT_READY | Not yet. Even Rock types need time to harden. |
| PRAISE | Nicely done! That's rock-hard willpower if I ever saw it. |
| GIFT | Take this {ITEM}. A good breeder always shares supplies. |
| BYE | Take care! And keep your POKéMON well fed! |
| QUIRK | {BUDDY} gets fussy if I burn the rice. So do my little siblings. |

Hooks: cooking for friends and POKéMON (anime); dream of becoming a POKéMON
Breeder (anime); a big family of younger siblings he looks after (anime);
"rock-hard willpower" (RBY/FRLG gym text) (verify); his Onix (anime).

### Misty

Buddy: slot 2, Golduck (PSYDUCK early). Anime Psyduck.

| Bit | Line |
| --- | --- |
| HELLO | Hey, {PLAYER}! Took you long enough! |
| AGAIN | Oh, it's you again. Fine, I guess I'm getting used to you. |
| CLOSE | Hey, {PLAYER}! Best friend alert! Don't tell anyone I said so. |
| MEET | I'm MISTY! My policy? An all-out offensive with Water types! |
| HEARD | You're {PLAYER}? I've heard things. They'd better be true! |
| NOT_YET | Beat me at my GYM first. My Water types won't go easy! |
| NEWS | Okay, listen up! You'll want to hear this. |
| ASK | Okay, listen up! |
| YES | Now that's what I like to hear! |
| NO | Hmph! Fine. Don't come crying to me later! |
| NOT_READY | Not yet! Come back when you've got some backbone! |
| PRAISE | Wow, not bad! You'd almost make a decent Water trainer! |
| GIFT | Take this {ITEM}! Don't say I never gave you anything! |
| BYE | See ya! ...{BUDDY}, get back in your ball! Not again! |
| QUIRK | Just keep Bug POKéMON away from me. Seriously. Ugh! |

Hooks: "My policy is an all-out offensive with Water-type POKéMON!" (RBY/FRLG
gym text); dream of becoming a Water POKéMON Master (anime); fear of Bug
POKéMON (anime); quick, feisty temper (anime); Psyduck popping out of its
ball uninvited (anime).

### Lt. Surge

Buddy: slot 1, Raichu (PIKACHU early). His anime Raichu, and the Pikachu gag.

| Bit | Line |
| --- | --- |
| HELLO | Hey, kid! {PLAYER}! Still standing? Good! |
| AGAIN | Hey, kid! You again! Starting to grow on me, ya know! |
| CLOSE | Hey there, {PLAYER}! My favorite soldier! At ease, friend! |
| MEET | Name's LT. SURGE, the Lightning American! Nice to meet ya! |
| HEARD | Hey, kid! You're {PLAYER}, right? I heard you're tough! |
| NOT_YET | Hey, kid! Win at my GYM first. Then we'll talk like soldiers! |
| NEWS | Listen up, kid! Here's the latest intel. |
| ASK | Listen up, soldier! |
| YES | Now that's a soldier! |
| NO | Hah! Chickened out, huh? Suit yourself, kid. |
| NOT_READY | Negative, kid! You won't live long in combat like that! |
| PRAISE | Ahaha! You're the real deal, kid! Outstanding! |
| GIFT | Supply drop, kid: one {ITEM}! Use it well! |
| BYE | Dismissed, kid! Stay sharp! |
| QUIRK | ELECTRIC POKéMON saved me in the war! {BUDDY} never lets me forget. |

Hooks: "Hey, kid!", "You won't live long in combat!" and "Electric POKéMON
saved me during the war!" (RBY/FRLG); "The Lightning American!" title (RBY/FRLG
gym sign); his Raichu beating Ash's unevolved Pikachu (anime).

### Erika

Buddy: slot 1, Vileplume (ODDISH, then GLOOM). Anime Gloom.

| Bit | Line |
| --- | --- |
| HELLO | Oh... {PLAYER}? Forgive me, I was nearly asleep. Hello! |
| AGAIN | Oh, hello again. I remember you. Please, relax a while. |
| CLOSE | Dear {PLAYER}. Your company is a joy. Please, stay a moment. |
| MEET | Pleased to meet you. I am ERIKA. I teach flower arranging. |
| HEARD | Oh... you are {PLAYER}? I have heard such lovely things. |
| NOT_YET | Please visit my GYM first. We shall have a proper match there. |
| NEWS | Oh, I heard the most curious thing... |
| ASK | Might I suggest something? |
| YES | How kind of you. Thank you ever so much. |
| NO | Oh, I see. That's quite all right. |
| NOT_READY | Perhaps not yet. Patience helps flowers bloom, too. |
| PRAISE | Oh my, how splendid! You have a gardener's patience. |
| GIFT | Please accept this {ITEM}. A small gift, but sincere. |
| BYE | Farewell. Oh... I think I'll close my eyes a moment... |
| QUIRK | Mm... zzz... Oh! Pardon me. I must have dozed off. |

Hooks: dozing off (RBY/FRLG, HGSS); "I teach the art of flower arranging"
(RBY/FRLG); polite, gentle speech (games, anime).

### Janine

Buddy: slot 1, Venomoth (VENONAT early). Iconic ace.

| Bit | Line |
| --- | --- |
| HELLO | Fufufufu... Surprised? It's just me! Hello, {PLAYER}! |
| AGAIN | Fufufu! You again! I'm starting to see through you! |
| CLOSE | {PLAYER}! Fufufu, there's no need to hide from you anymore! |
| MEET | I'm JANINE, daughter of KOGA, and a ninja in training! |
| HEARD | Fufufu! You're {PLAYER}! My spies told me all about you! |
| NOT_YET | Face me at my GYM first. A ninja doesn't allow shortcuts! |
| NEWS | Psst! A good ninja hears everything. Listen... |
| ASK | Hee hee! Psst, psst! |
| YES | Thank you! Father would call that honorable. |
| NO | Oh... all right. I'll handle it myself! |
| NOT_READY | Not yet! Even a ninja has to train first. |
| PRAISE | Amazing! Father would be impressed. I am, too! |
| GIFT | Take this {ITEM}. A ninja's gift, straight from the shadows! |
| BYE | Farewell! *Poof!* ...I'm still working on that part. |
| QUIRK | Sorry to disappoint you... I'm only joking! It's really me! |

Hooks: gym trainers disguised as her, then "Fufufufu... I'm sorry to
disappoint you... I'm only joking!" (GSC/HGSS) (verify);
daughter of Koga who took over his GYM (GSC/HGSS); ninja training (HGSS,
Masters EX).

### Sabrina

Buddy: slot 4, Gengar (GASTLY, then HAUNTER). Anime Haunter.

| Bit | Line |
| --- | --- |
| HELLO | {PLAYER}. I foresaw that you would come. |
| AGAIN | We meet again. I remember you. That is rare. |
| CLOSE | {PLAYER}. I see you clearly now. You are a true friend. |
| MEET | I am SABRINA. I have had psychic powers since childhood. |
| HEARD | You are {PLAYER}. Your name reached me before you did. |
| NOT_YET | My GYM awaits. I have already seen how our battle ends. |
| NEWS | My mind picked something up. Listen. |
| ASK | I foresee something. Pay attention. |
| YES | As I foresaw. |
| NO | I knew you would refuse. It changes nothing. |
| NOT_READY | No. Your future is not yet clear enough. |
| PRAISE | Even I did not foresee that. Well done. |
| GIFT | Take this {ITEM}. My visions say you will need it. |
| BYE | We will meet again. I have seen it. |
| QUIRK | {BUDDY} once made me laugh. It was... unexpected. |

Hooks: "I had a vision of your arrival! I have had psychic powers since I was
a child" (RBY/FRLG); Ash's Haunter making her laugh (anime), which the
catalog files as her Gengar slot ("anime Haunter").

### Blaine

Buddy: slot 1, Magmortar (MAGMAR early). His RBY and FRLG ace.

| Bit | Line |
| --- | --- |
| HELLO | Hah! {PLAYER}! Got your BURN HEAL handy? |
| AGAIN | Hah! Back again! You're getting to be a familiar face! |
| CLOSE | Hah! {PLAYER}, my friend! Always a hot time with you! |
| MEET | Hah! I am BLAINE! Quizmaster and red-hot Fire trainer! |
| HEARD | Hah! You're {PLAYER}! Heard you're a real hot shot! |
| NOT_YET | Hah! Pass my GYM quiz and beat me there first! |
| NEWS | Quiz time! No, wait... just some news. Listen! |
| ASK | Hey, kid, hear me out. |
| YES | Hah! Now you're talking! Hah! |
| NO | Hah! Cold feet? Well, suit yourself. |
| NOT_READY | Buzz! Not yet! Come back when you've got more fire. |
| PRAISE | Hah! You've got fire! Now that's hot stuff! |
| GIFT | Your prize for a correct answer: {ITEM}! Hah! |
| BYE | Hah! Better have BURN HEAL next time! |
| QUIRK | What burns hot but never melts? My fighting spirit! Hah! |

Hooks: gym quiz (RBY/FRLG) and riddles (anime); "Hah!" and "You better have
BURN HEAL!" (RBY/FRLG).

### Giovanni

Buddy: slot 4, Persian (MEOWTH early). Anime Persian.

| Bit | Line |
| --- | --- |
| HELLO | So. {PLAYER}. You keep turning up. |
| AGAIN | You again. I am beginning to remember your face. |
| CLOSE | {PLAYER}. You have earned my trust. Do not waste it. |
| MEET | I am GIOVANNI. Remember the name. Others have learned to. |
| HEARD | So. You are {PLAYER}. I have heard your name. Often. |
| NOT_YET | Earn your way through my GYM first. Then I may notice you. |
| NEWS | TEAM ROCKET hears everything. Listen well. |
| ASK | I have a proposition. |
| YES | A wise decision. |
| NO | Hmph. You'll regret wasting my time. |
| NOT_READY | You are not worth my time yet. Come back stronger. |
| PRAISE | Impressive. I rarely have cause to say that. |
| GIFT | Take this {ITEM}. Consider it a loan, not a kindness. |
| BYE | Go. We will meet again. |
| QUIRK | Ah, {BUDDY}. The only one who never disappoints me. |

Hooks: Persian at his side (anime); boss of TEAM ROCKET (RBY/FRLG, HGSS,
Let's Go). He stays cold and transactional throughout.

### Blue

Buddy: slot 1, Umbreon (EEVEE early). Anime Gary's Umbreon.

| Bit | Line |
| --- | --- |
| HELLO | Yo, {PLAYER}! Miss me? Course you did. |
| AGAIN | Oh, you again. Keep this up and I'll remember you! |
| CLOSE | Yo, {PLAYER}! My rival and my friend. Don't let it go to your head. |
| MEET | I'm BLUE! Remember it, 'cause I'm the best there is! |
| HEARD | You're {PLAYER}? Everyone's talking about you. Not me, though. |
| NOT_YET | Beat me in a real battle first. Till then, I'm the best! |
| NEWS | Heh, bet you haven't heard this one yet. |
| ASK | Hey, {PLAYER}. A second? |
| YES | Heh, knew you'd say yes! |
| NO | Pfft. Whatever. Your loss! |
| NOT_READY | What, that's it? You're not ready. Come back later! |
| PRAISE | Not bad! ...For someone who isn't me, anyway. |
| GIFT | Take this {ITEM}. Gramps says I should share. Ugh. |
| BYE | Smell ya later! |
| QUIRK | Gramps said I lacked trust and love for my POKéMON. Not anymore! |

Hooks: "Smell ya later!" (RBY/FRLG, Masters EX); "Gramps" (RBY/FRLG, HGSS);
Oak's scolding after the Champion loss that Blue forgot to treat his POKéMON
with trust and love (RBY/FRLG).

### Lorelei

Buddy: slot 1, Lapras. Her RBY and FRLG ace and Masters EX partner.

| Bit | Line |
| --- | --- |
| HELLO | Ah, {PLAYER}. Come to see me? How refreshing. |
| AGAIN | Ah, you again. I am beginning to know you. |
| CLOSE | Dear {PLAYER}. I am truly glad to see you. Stay a while. |
| MEET | I am LORELEI. No one can best me with icy POKéMON. |
| HEARD | You're {PLAYER}? I have heard so much about you. |
| NOT_YET | Defeat me in battle first. Until then, you're on thin ice. |
| NEWS | I heard something interesting. Listen closely. |
| ASK | Hmph. I have something to say. |
| YES | Good. I expected nothing less. |
| NO | Pity. I'll find someone else. |
| NOT_READY | Not yet. Come back once you're a little sharper. |
| PRAISE | Impressive. You kept your cool. I respect that. |
| GIFT | Take this {ITEM}. Consider it a cool reward. |
| BYE | Come along, {BUDDY}. Until next time. |
| QUIRK | Don't tell anyone, but I collect POKéMON dolls. They're adorable. |

Hooks: "No one can best me when it comes to icy POKéMON!" (RBY/FRLG);
Lapras as her signature partner (RBY/FRLG ace, Masters EX sync pair); her home
full of POKéMON dolls (FRLG Sevii postgame) (verify).

### Bruno

Buddy: slot 1, Machamp (MACHOP, then MACHOKE). Iconic ace.

| Bit | Line |
| --- | --- |
| HELLO | Hoo hah! {PLAYER}! Still training hard? |
| AGAIN | Hoo hah! You again! Your face is familiar now! |
| CLOSE | Hoo hah! {PLAYER}! My friend! Our bond is strong as steel! |
| MEET | I am BRUNO! I train with my POKéMON, body and soul! |
| HEARD | Hoo hah! You are {PLAYER}! I have heard of your strength! |
| NOT_YET | Hoo hah! Defeat me in battle first! Then we'll talk! |
| NEWS | Hmm. I heard something between training sets. Listen. |
| ASK | Hey. Listen closely. |
| YES | Hoo hah! A true fighter's answer! |
| NO | Hmph. A fighter knows his limits. I respect that. |
| NOT_READY | Not yet. Train harder, then face this again! |
| PRAISE | Hoo hah! Superior power! You've earned my respect! |
| GIFT | Take this {ITEM}. Use it to grow even stronger! |
| BYE | Train hard! Hoo hah! |
| QUIRK | {BUDDY} and I have trained together for years. Muscles never lie! |

Hooks: "Hoo hah!" and "We will grind you down with our superior power!"
(RBY/FRLG, HGSS); training alongside his Fighting POKéMON (RBY/FRLG, HGSS).

### Agatha

Buddy: slot 1, Gengar (GASTLY, then HAUNTER). Iconic ace.

| Bit | Line |
| --- | --- |
| HELLO | Hmph. {PLAYER}. Still kicking, are you? |
| AGAIN | Hmph. You again, child. You're persistent, I'll give you that. |
| CLOSE | Hmph. {PLAYER}. Fine, you're all right, child. Don't tell anyone. |
| MEET | I am AGATHA. Remember that name, child. OAK surely does. |
| HEARD | Hmph. So you're {PLAYER}. I've heard the rumors, child. |
| NOT_YET | Beat me in battle first, child. Then I might listen. |
| NEWS | Hee hee. Old ears still hear plenty. Listen, child. |
| ASK | Kukuku... a thought occurs to me. |
| YES | Hmph. Good. Don't disappoint me. |
| NO | Bah! Young people! |
| NOT_READY | Hah! Not yet, child. You'd only bore me. |
| PRAISE | Not bad. POKéMON are for fighting, and you know it. |
| GIFT | Take this {ITEM}. Don't make me regret it. |
| BYE | Off with you, child. {BUDDY} and I have things to do. |
| QUIRK | That old duff OAK was tough once. Now he fiddles with his POKéDEX! |

Hooks: old rivalry with Prof. Oak, "That old duff was once tough and
handsome", "POKéMON are for fighting!" and calling the player "child"
(RBY/FRLG); Gengar (RBY/FRLG ace).

### Lance

Buddy: slot 1, Dragonite (DRATINI, then DRAGONAIR). Iconic ace.

| Bit | Line |
| --- | --- |
| HELLO | {PLAYER}. Good to see you. You're looking strong. |
| AGAIN | Good to see you again. I'm glad our paths crossed. |
| CLOSE | {PLAYER}! My friend. It's always good to see you. |
| MEET | I'm LANCE, a trainer of dragons. It's a pleasure. |
| HEARD | So you're {PLAYER}. Your name has reached even me. |
| NOT_YET | Defeat me in battle first. Dragons respect only strength. |
| NEWS | I've been keeping an eye on things. Listen. |
| ASK | You. Child. Pay attention. |
| YES | A fine answer. |
| NO | I understand. The offer stands. |
| NOT_READY | Not yet. A dragon's power must be earned. |
| PRAISE | Remarkable. You have the heart of a dragon master. |
| GIFT | Take this {ITEM}. Use it well, as I know you will. |
| BYE | Until next time. {BUDDY}, let's go! |
| QUIRK | My cousin CLAIR pushes me hard. She'll never admit it, though. |

Hooks: dragon master (RBY/FRLG, GSC/HGSS); Dragonite (RBY/FRLG, GSC/HGSS
ace); cousin Clair (GSC/HGSS). Lance's catalog home region is Kanto, so he is
filed here.

---

## Johto

### Koga

Buddy: slot 1, Crobat (ZUBAT, then GOLBAT). His HGSS ace.

| Bit | Line |
| --- | --- |
| HELLO | Fwahahaha! {PLAYER}. You did not sense me, did you? |
| AGAIN | Fwahaha! You again. Your steps are familiar to me now. |
| CLOSE | Fwahaha! {PLAYER}. I trust few. You are one of them. |
| MEET | I am KOGA. I live in shadows, a ninja! |
| HEARD | Fwahaha! You are {PLAYER}. My shadows spoke of you. |
| NOT_YET | Best me in battle first. Until then, you walk in my shadow. |
| NEWS | A ninja gathers secrets. Here is one. |
| ASK | Hm? Just a moment. |
| YES | Fwahahaha! Good. A wise choice. |
| NO | So be it. A ninja works alone in any case. |
| NOT_READY | Too soon. Patience is the first of the ninja arts. |
| PRAISE | Fwahahaha! You have the makings of a ninja! |
| GIFT | Take this {ITEM}. Use it wisely, and quietly. |
| BYE | Until our paths cross again. Fwahahaha! |
| QUIRK | JANINE trains hard. She'll surpass me someday. Don't tell her. |

Hooks: "Fwahahaha!" (RBY/FRLG, GSC/HGSS); "I live in shadows, a ninja!"
(GSC/HGSS Elite Four text); father of Janine (GSC/HGSS). His catalog home
region is Johto, so he is filed here.

### Falkner

Buddy: slot 1, Pidgeot (PIDGEY, then PIDGEOTTO). Anime and HGSS Pidgeotto.

| Bit | Line |
| --- | --- |
| HELLO | {PLAYER}! Good timing. My birds are itching to fly. |
| AGAIN | Oh, hello again! We keep running into each other, don't we? |
| CLOSE | {PLAYER}! You're a true friend. My birds love you, too! |
| MEET | I'm FALKNER. I carry on my father's work with bird POKéMON. |
| HEARD | You're {PLAYER}? My father would want to hear of you! |
| NOT_YET | Beat me at my GYM first. My birds won't fall so easily! |
| NEWS | Word flies fast. Here's what I heard. |
| ASK | Excuse me, if I may! |
| YES | Thank you! Father always said to rely on good friends. |
| NO | I see. I'll manage on my own, then. |
| NOT_READY | Not yet. Even fledglings need time before they fly. |
| PRAISE | Magnificent! Even my father would admire that. |
| GIFT | Take this {ITEM}. You've earned it, fair and square. |
| BYE | Take care! Keep your spirits soaring! |
| QUIRK | Some say a jolt of electricity clips a bird's wings. Nonsense! |

Hooks: carrying on his father's bird POKéMON legacy (GSC/HGSS); "People say
you can clip Flying-type POKéMON's wings with a jolt of electricity... I won't
allow such insults" (GSC/HGSS).

### Bugsy

Buddy: slot 1, Scizor (SCYTHER early). Anime and GSC Scyther.

| Bit | Line |
| --- | --- |
| HELLO | Hi, {PLAYER}! Seen any interesting Bug POKéMON lately? |
| AGAIN | Hi again! I'm starting to learn your habits. For research! |
| CLOSE | {PLAYER}! My best research partner! I saved you a note! |
| MEET | I'm BUGSY! I never lose when it comes to Bug POKéMON! |
| HEARD | You're {PLAYER}? I've read about you! Can I study you? |
| NOT_YET | Beat me at my GYM first! It'll help my research, too! |
| NEWS | Oh! I've been taking notes, and I heard this... |
| ASK | Oh! Hey, hey, wait a second! |
| YES | Yes! Science thanks you! I do, too! |
| NO | Aww... Okay. I'll add it to my notes anyway. |
| NOT_READY | Not yet! Every bug has to molt before it grows. |
| PRAISE | Wow! That was amazing! Can I write it down? |
| GIFT | Take this {ITEM}! I found it during my research. |
| BYE | Bye! If you spot a rare bug, tell me first! |
| QUIRK | I measure {BUDDY} every day. For research! It hates that. |

Hooks: "I never lose when it comes to Bug POKéMON" and his research (GSC/HGSS);
Scyther as his signature (anime, GSC).

### Whitney

Buddy: slot 1, Miltank. Anime and GSC Miltank.

| Bit | Line |
| --- | --- |
| HELLO | Hiii, {PLAYER}! I'm so glad to see you! |
| AGAIN | Hiii again! I remember you! We should hang out more! |
| CLOSE | {PLAYER}! Yay! You're my bestest friend! Come on, hang out! |
| MEET | Hi! I'm WHITNEY! Everyone got into POKéMON, so I did too! |
| HEARD | Wait, you're {PLAYER}? Everyone's talking about you! |
| NOT_YET | Beat me at my GYM first! ...And no making me cry! |
| NEWS | Ooh, ooh! You've gotta hear this! |
| ASK | Ooh, ooh, I have an idea! |
| YES | Yay! You're the best! |
| NO | Waaah! Meanie! ...Just kidding. Mostly. |
| NOT_READY | Nuh-uh! Not yet! Come back when you're ready! |
| PRAISE | Wow! That was so cool! {BUDDY} thinks so too! |
| GIFT | This {ITEM} is for you! Yay, presents! |
| BYE | Bye-bye! Call me sometime, 'kay? |
| QUIRK | Waaaah! ...Sorry. I cry when I lose. I'm fine now! |

Hooks: crying after losing (GSC/HGSS); Miltank (GSC/HGSS, anime); "Everyone
was into POKéMON, so I got into it too!" (GSC/HGSS); phone calls (GSC/HGSS
PokéGear).

### Morty

Buddy: slot 1, Gengar (GASTLY, then HAUNTER). Anime Gengar.

| Bit | Line |
| --- | --- |
| HELLO | {PLAYER}. I had a feeling you'd come. |
| AGAIN | We meet again. I sense we will grow close. |
| CLOSE | {PLAYER}. I see a bright bond between us. I'm glad you came. |
| MEET | I'm MORTY. I train to see what others cannot. |
| HEARD | You are {PLAYER}. I saw your name in a vision. |
| NOT_YET | Face me at my GYM first. Then I'll see you clearly. |
| NEWS | EUSINE told me something. You should hear it. |
| ASK | Ah, a moment of your time. |
| YES | Thank you. I sensed you would agree. |
| NO | I understand. Perhaps another time. |
| NOT_READY | The vision isn't clear yet. Come back later. |
| PRAISE | Remarkable. The legend may yet choose you. |
| GIFT | Take this {ITEM}. It may help you see further. |
| BYE | Until we meet again. Keep your eyes open. |
| QUIRK | A rainbow-hued POKéMON appears to the truly strong. I'll see it. |

Hooks: training to meet the legendary rainbow POKéMON (Ho-Oh), which isn't
named because it isn't on his roster (GSC/HGSS); "I can see what you cannot"
mystic sight (GSC/HGSS) (verify); friendship with Eusine
(Crystal/HGSS); his Gengar (anime).

### Chuck

Buddy: slot 1, Poliwrath (POLIWAG, then POLIWHIRL). Anime and GSC Poliwrath.

| Bit | Line |
| --- | --- |
| HELLO | WAHAHAH! {PLAYER}! Good to see you, buddy! |
| AGAIN | WAHAHAH! You again! I'm getting used to that face! |
| CLOSE | WAHAHAH! {PLAYER}! My buddy! Pals train together, right? |
| MEET | I'm CHUCK! My POKéMON crush stones and shatter bones! |
| HEARD | WAHAHAH! You're {PLAYER}! I heard you've got guts! |
| NOT_YET | Beat me at my GYM first! Then we'll talk as equals! |
| NEWS | WAHAHAH! Listen up! I heard this during training! |
| ASK | Hey, kid! Eyes on me! |
| YES | WAHAHAH! That's the spirit! |
| NO | Hmm! Fine. I'll just train harder! |
| NOT_READY | Not yet! Toughen up and come back! |
| PRAISE | WAHAHAH! Great! That's real fighting spirit! |
| GIFT | Take this {ITEM}! Don't tell my wife I gave it away! |
| BYE | See you! Keep training! Hah! |
| QUIRK | Hyaaah! ...My wife says I overdo my training. She's right. |

Hooks: "WAHAHAH!" and "My POKéMON will crush stones and shatter bones!"
(GSC/HGSS); his wife, who comments on his training (GSC/HGSS); Poliwrath
(GSC/HGSS ace, anime).

### Jasmine

Buddy: slot 3, Ampharos (MAREEP, then FLAAFFY). Amphy, the lighthouse
Ampharos.

| Bit | Line |
| --- | --- |
| HELLO | Oh... {PLAYER}. Um... hello. It's nice to see you. |
| AGAIN | Oh... um... hello again. I remember you. |
| CLOSE | {PLAYER}... I'm so happy to see you. You're my friend. |
| MEET | Um... I'm JASMINE. I use... the Steel type. Nice to meet you. |
| HEARD | Um... you're {PLAYER}? I have... heard about you. |
| NOT_YET | Um... please beat me at my GYM first. I'll do my best. |
| NEWS | Um... I heard something. May I tell you? |
| ASK | Oh, might I have a word? |
| YES | Oh, thank you... Really, thank you. |
| NO | Oh... I understand. It's all right. |
| NOT_READY | Um... I don't think you're ready yet. I'm sorry. |
| PRAISE | That was wonderful... You're really strong. |
| GIFT | Um... please take this {ITEM}. I hope it helps. |
| BYE | Goodbye... Please take care. |
| QUIRK | AMPHY was sick once. I stayed by its side. It's strong now. |

Hooks: nursing the sick Ampharos, Amphy (GSC/HGSS, anime); shy, halting
speech (GSC/HGSS); Steel type and Steelix (GSC/HGSS, anime).
Nickname: her lines say AMPHY, which fits MAREEP, FLAAFFY and AMPHAROS alike.

### Pryce

Buddy: slot 1, Mamoswine (SWINUB, then PILOSWINE). Anime Piloswine.

| Bit | Line |
| --- | --- |
| HELLO | Ah, {PLAYER}. Good. An old man enjoys some company. |
| AGAIN | Ah, you again. An old man remembers a good face. |
| CLOSE | {PLAYER}! My dear friend. An old man treasures such company. |
| MEET | I am PRYCE. I've been with POKéMON since before you were born. |
| HEARD | Hm. So you're {PLAYER}. Even an old man hears the talk. |
| NOT_YET | Beat me at my GYM first, youngster. Experience is earned. |
| NEWS | At my age, one hears things. Listen. |
| ASK | Ho ho. Now, listen well. |
| YES | Hm. You have a good heart. |
| NO | Hm. I'll not force you. |
| NOT_READY | Not yet. Tests come to all of us, in their own time. |
| PRAISE | Hm! You remind me of myself, long ago. |
| GIFT | Take this {ITEM}. At my age, I've little use for it. |
| BYE | Go on, then. Don't keep your POKéMON waiting. |
| QUIRK | {BUDDY} and I have been together a long, long time. |

Hooks: an elder who has "seen and suffered much" and "been with POKéMON since
before you were born" (GSC/HGSS); his lifelong bond with Piloswine (anime)
(verify), which is his MAMOSWINE slot. His "winter trainer" title is left
out because it's a season word.

### Clair

Buddy: slot 2, Dragonite (DRATINI, then DRAGONAIR). Anime Dragonair.

| Bit | Line |
| --- | --- |
| HELLO | Oh, it's you, {PLAYER}. Hmph. Fine, I'll say hello. |
| AGAIN | You again? Hmph. I suppose I'm used to you now. |
| CLOSE | {PLAYER}. Hmph. Fine, you're a friend. Don't make it weird. |
| MEET | I am CLAIR. The world's best dragon master. |
| HEARD | You're {PLAYER}? Hmph. I've heard. Don't get cocky. |
| NOT_YET | Beat me at my GYM first. Even then, I may not accept it! |
| NEWS | Listen. I'll only say this once. |
| ASK | Hmph. Hear me out. |
| YES | Naturally. I expected nothing less. |
| NO | Hmph! Suit yourself. |
| NOT_READY | You? Not yet. Dragons don't bow to the unready. |
| PRAISE | ...Fine. I'll admit it. That was impressive. |
| GIFT | Take this {ITEM}. Don't read anything into it. |
| BYE | Go on. And don't tell LANCE I was nice to you. |
| QUIRK | I don't accept this! ...Hmph. Fine. Maybe I do. A little. |

Hooks: "I am CLAIR. The world's best dragon master." (GSC/HGSS); refusing to
accept defeat and withholding the badge (GSC/HGSS); cousin Lance (GSC/HGSS).

### Will

Buddy: slot 1, Xatu (NATU early). His GSC and HGSS ace.

| Bit | Line |
| --- | --- |
| HELLO | {PLAYER}. We meet again. As expected. |
| AGAIN | We meet again. I am beginning to know you. |
| CLOSE | {PLAYER}. I trust you more than most. Do not tell anyone. |
| MEET | I am WILL. I have trained all around the world. |
| HEARD | So you are {PLAYER}. Your name has reached me, mask and all. |
| NOT_YET | Defeat me in battle first. I will not lose so easily. |
| NEWS | My psychic POKéMON sensed something. Listen. |
| ASK | Hmm. I have something to say. |
| YES | Very good. |
| NO | Then so be it. I have no use for hesitation. |
| NOT_READY | You are not ready. My {BUDDY} sees it clearly. |
| PRAISE | ...Impressive. The mask hides a great deal, but not that. |
| GIFT | Take this {ITEM}. Consider it a courtesy. |
| BYE | Farewell. Do not keep me waiting next time. |
| QUIRK | The mask? It is part of me. Do not ask again. |

Hooks: "I have trained all around the world, making my psychic POKéMON
powerful" (GSC/HGSS); his mask (GSC/HGSS design); Xatu (GSC/HGSS ace).

### Karen

Buddy: slot 1, Umbreon (EEVEE early). Her GSC and HGSS ace.

| Bit | Line |
| --- | --- |
| HELLO | Oh, {PLAYER}. Good. I was getting bored. |
| AGAIN | Oh, you again. You're growing on me. A little. |
| CLOSE | Oh, {PLAYER}. Good. I really did hope you'd come. |
| MEET | I'm KAREN. I use Dark types. Remember that. |
| HEARD | You're {PLAYER}? I've heard you trust your POKéMON. |
| NOT_YET | Defeat me first. Then we'll see what kind of trainer you are. |
| NEWS | I heard something you might find interesting. |
| ASK | Ahem. A word. |
| YES | Good. I like decisive trainers. |
| NO | Fine. Your choice. I respect that. |
| NOT_READY | Not yet. Come back when you've really grown. |
| PRAISE | Well done. You trust your POKéMON. That's what matters. |
| GIFT | Take this {ITEM}. Don't waste it. |
| BYE | Go on. Keep battling with your favorites. |
| QUIRK | Strong POKéMON, weak POKéMON... Only people's selfish view. |

Hooks: "Strong POKéMON. Weak POKéMON. That is only the selfish perception of
people. Truly skilled trainers should try to win with their favorites."
(GSC/HGSS); Umbreon (GSC/HGSS ace).

---

## Hoenn

### Roxanne

Buddy: slot 1, Probopass (NOSEPASS early). Anime and RSE Nosepass.

| Bit | Line |
| --- | --- |
| HELLO | Hello, {PLAYER}! Ready for another lesson? |
| AGAIN | Hello again! I remember you. Your progress is fascinating! |
| CLOSE | {PLAYER}! My favorite student! I kept a page for you! |
| MEET | I'm ROXANNE. I study POKéMON battles, and I love to teach! |
| HEARD | You're {PLAYER}? Your name is in all my notes! |
| NOT_YET | Please take on my GYM first. Study hard, then battle me! |
| NEWS | Class, attention please! ...Oh, sorry. Habit. Listen... |
| ASK | Um... excuse me? Sorry! |
| YES | Excellent! Full marks for you! |
| NO | I see. We'll revisit it later. |
| NOT_READY | Not quite. You still need to study a little more. |
| PRAISE | Wonderful! That was textbook-perfect! |
| GIFT | Take this {ITEM}. A reward for such a good student! |
| BYE | Goodbye! Don't forget to review your notes! |
| QUIRK | {BUDDY} always faces north. It's very dependable! |

Hooks: teaching at the Trainer's School (anime, RSE/ORAS); learning that book
study needs real battle experience (anime, ORAS) (verify); Nosepass as her
signature (RSE/ORAS ace, anime; PROBOPASS on roster, and it faces north like
Nosepass per the POKéDEX) (verify).

### Brawly

Buddy: slot 1, Hariyama (MAKUHITA early). Anime and RSE Makuhita.

| Bit | Line |
| --- | --- |
| HELLO | Hey, {PLAYER}! Totally stoked to see you! |
| AGAIN | Hey, you again! Getting to know you is a total wave! |
| CLOSE | {PLAYER}! My bro! Always stoked to ride with you! |
| MEET | I'm BRAWLY! Fighting and surfing, that's my whole deal! |
| HEARD | Dude, you're {PLAYER}? Everyone's buzzing about you! |
| NOT_YET | Beat me at my GYM first! Show me what you're made of! |
| NEWS | Dude, you gotta hear this! |
| ASK | Hey! Quick question! |
| YES | Awesome! Totally righteous, dude! |
| NO | No big deal! There's always another wave. |
| NOT_READY | Not yet, dude. Keep paddling! |
| PRAISE | Whoa! You made a much bigger splash than I expected! |
| GIFT | Take this {ITEM}! Keep riding high! |
| BYE | Later! Stay stoked! |
| QUIRK | {BUDDY} and I train like we surf: wipe out, get back up! |

Hooks: surfer persona (RSE/ORAS, anime); "You made a much bigger splash than I
expected!" (RSE/ORAS); Makuhita/Hariyama (RSE/ORAS ace, anime). The wave
lines are surfing slang about himself, not claims about the location.

### Wattson

Buddy: slot 1, Manectric (ELECTRIKE early). His RSE ace.

| Bit | Line |
| --- | --- |
| HELLO | Wahahahah! {PLAYER}! You look fully charged! |
| AGAIN | Wahahahah! You again! Your face is getting familiar! |
| CLOSE | Wahahahah! {PLAYER}! My dear friend! You light me up! |
| MEET | Wahahahah! I'm WATTSON! Old, but still full of sparks! |
| HEARD | Wahahahah! You're {PLAYER}! I heard you're a live wire! |
| NOT_YET | Wahahahah! Beat me at my GYM first! |
| NEWS | Wahahahah! Here's a shocking bit of news! |
| ASK | Hey, hey, listen up! |
| YES | Wahahahah! Electrifying! |
| NO | Wahahahah! Fine, fine! Maybe next time! |
| NOT_READY | Not yet! You need a bit more charge in your battery! |
| PRAISE | Wahahahah! Simply shocking! Well done! |
| GIFT | Take this {ITEM}! Wahahahah! Don't spend it all at once! |
| BYE | Wahahahah! Off you go! Stay charged! |
| QUIRK | I tinker with gadgets and traps for fun! Wahahahah! |

Hooks: "Wahahahah!" laugh (RSE/ORAS); a tinkerer who put his spare time into
building GYM traps and gadgets (RSE) (verify).

### Flannery

Buddy: slot 1, Torkoal. Anime and RSE Torkoal.

| Bit | Line |
| --- | --- |
| HELLO | Oh, {PLAYER}! Hi! I mean... Ahem. Greetings! |
| AGAIN | Oh, hi again! I'm starting to get to know you! |
| CLOSE | {PLAYER}! You're my friend! I can be myself with you! |
| MEET | I'm FLANNERY! My grandfather taught me all about fire! |
| HEARD | Wait, you're {PLAYER}? Gramps told me all about you! |
| NOT_YET | Beat me at my GYM first! I'll show you my hottest moves! |
| NEWS | Ooh, hot news! Listen! |
| ASK | Oh, um... can I say something? |
| YES | Yes! That's the fiery spirit I like! |
| NO | Oh... Right. No, I'm fine! Totally fine! |
| NOT_READY | Not yet! Come back with more fire in your heart! |
| PRAISE | Wow! That was hot! You really burned bright! |
| GIFT | Take this {ITEM}! Keep that fire burning! |
| BYE | See you! Stay fired up! |
| QUIRK | I was trying to act all cool... That's not really me, is it? |

Hooks: taking over from her grandfather (RSE/ORAS); admitting she "tried too
hard to be someone I'm not" after losing (RSE/ORAS); Torkoal (RSE/ORAS ace,
anime).

### Norman

Buddy: slot 1, Slaking (SLAKOTH, then VIGOROTH). Anime and RSE Slaking.

| Bit | Line |
| --- | --- |
| HELLO | Ah, {PLAYER}. Good to see you. Keeping strong? |
| AGAIN | Ah, hello again. I'm glad to see you back. |
| CLOSE | {PLAYER}. You're like family. I am always glad to see you. |
| MEET | I'm NORMAN. A GYM LEADER, but a father first. |
| HEARD | So you're {PLAYER}. I've heard a lot about you. |
| NOT_YET | Earn the right at my GYM first. I don't hold back. |
| NEWS | I heard something. You should know about it. |
| ASK | Ahem. Might I have a word? |
| YES | Thank you. I knew you had it in you. |
| NO | I understand. Every trainer makes their own choices. |
| NOT_READY | Not yet. Strength comes from steady effort. |
| PRAISE | Well done! You've grown. I can see it. |
| GIFT | Take this {ITEM}. Use it wisely. |
| BYE | Take care. And keep training hard. |
| QUIRK | Don't judge {BUDDY} by its moods. Its power is the real thing. |

Hooks: devoted father (the player's father in RSE/ORAS; May and Max's father
in the anime); Slaking (RSE/ORAS ace, anime), whose line fits the lazy
SLAKOTH, the restless VIGOROTH and SLAKING alike. The lines never say he is
the player's father.

### Winona

Buddy: slot 1, Altaria (SWABLU early). Anime and RSE Altaria.

| Bit | Line |
| --- | --- |
| HELLO | Hello, {PLAYER}. It's always a pleasure. |
| AGAIN | Ah, hello again. I remember you well. |
| CLOSE | {PLAYER}. I'm truly glad of your company, dear friend. |
| MEET | I am WINONA. I have become one with bird POKéMON. |
| HEARD | You are {PLAYER}? Word of you has travelled far. |
| NOT_YET | Please face me at my GYM first. Our grace awaits you. |
| NEWS | A little bird told me something. Listen. |
| ASK | Hmm. Listen carefully. |
| YES | Thank you. That was graceful of you. |
| NO | I see. Perhaps another time, then. |
| NOT_READY | Not yet. Even the strongest wings must grow first. |
| PRAISE | Splendid! You soared beautifully. |
| GIFT | Please take this {ITEM}. May it lift your spirits. |
| BYE | Farewell. May you soar ever higher. |
| QUIRK | {BUDDY} hums when it's happy. Listen closely sometime. |

Hooks: "I have become one with bird POKéMON and have soared the skies"
(RSE/ORAS); elegant grace in battle (RSE/ORAS); Altaria (RSE/ORAS ace; the
humming is from the POKéDEX).

### Tate & Liza

Buddy: slot 1, Solrock. Tate's half of the anime pair; the duo has one buddy.

| Bit | Line |
| --- | --- |
| HELLO | TATE: Hey, {PLAYER}! LIZA: We knew you'd come! |
| AGAIN | TATE: Oh, it's you again! LIZA: We remember you! |
| CLOSE | TATE: {PLAYER}! LIZA: Our best friend! We're so happy! |
| MEET | TATE: I'm TATE! LIZA: And I'm LIZA! We're twins! |
| HEARD | TATE: You're {PLAYER}! LIZA: We've heard all about you! |
| NOT_YET | TATE: Beat us at our GYM... LIZA: ...first, okay? |
| NEWS | TATE: Guess what? LIZA: We heard something! |
| ASK | TATE: Hey, listen! LIZA: Ooh, wait! |
| YES | TATE: Yay! LIZA: We knew you would! |
| NO | TATE: Aww... LIZA: ...no fair! |
| NOT_READY | TATE: Not yet! LIZA: You're not strong enough! |
| PRAISE | TATE: Wow, that was... LIZA: ...really amazing! |
| GIFT | TATE: This {ITEM}... LIZA: ...is from both of us! |
| BYE | TATE: See you... LIZA: ...later! |
| QUIRK | TATE: We can read... LIZA: ...each other's minds! Hehehe! |

Hooks: twins who read each other's minds, "Hehehe... Were you surprised?"
(RSE/ORAS); Solrock and Lunatone (RSE/ORAS, anime).

### Juan

Buddy: slot 1, Kingdra (HORSEA, then SEADRA). His Emerald ace.

| Bit | Line |
| --- | --- |
| HELLO | Ah, {PLAYER}. How delightful to see you again. |
| AGAIN | Ah, hello again! Our paths cross gracefully. |
| CLOSE | Ah, dear {PLAYER}! My friend! What a delight! |
| MEET | I am JUAN. It was I who taught WALLACE all he knows. |
| HEARD | Ah, you are {PLAYER}! Your fame precedes you. |
| NOT_YET | Show me your artistry at my GYM first. Then we'll speak. |
| NEWS | Allow me to share a little something. |
| ASK | Ahem. May I have your attention? |
| YES | Magnificent! Such grace in your answer. |
| NO | Ah, how unfortunate. No matter. |
| NOT_READY | Not yet. True artistry requires more polish. |
| PRAISE | Bravo! Truly, that was a work of art! |
| GIFT | Accept this {ITEM}, a token of my appreciation. |
| BYE | Farewell! Go forth with elegance! |
| QUIRK | Ahahaha! A battle should dazzle, like a fine performance! |

Hooks: Wallace's teacher, "It was I who taught WALLACE everything" (Emerald);
flamboyant, artistic style (Emerald); Luvdisc on his team (Emerald, not named
in these lines); "Ahahaha" laugh (verify).

### Sidney

Buddy: slot 1, Absol. His RSE ace.

| Bit | Line |
| --- | --- |
| HELLO | Hey, {PLAYER}! I like that look you're giving me! |
| AGAIN | Hey, it's you again! I like where this is going! |
| CLOSE | Hey, {PLAYER}! My partner in trouble! Let's have a blast! |
| MEET | I'm SIDNEY of the ELITE FOUR. Let's enjoy ourselves, huh? |
| HEARD | Hey, you're {PLAYER}? I've heard you're a blast! |
| NOT_YET | Beat me in a real battle first. Then we'll talk big. |
| NEWS | Heh. Got something you'll wanna hear. |
| ASK | Hey, hey, ooh, listen! |
| YES | That's good! Looking real good! |
| NO | Heh, fine by me. No hard feelings. |
| NOT_READY | Nah, not yet. Come back with more guts. |
| PRAISE | How do you like that? Eh, it was fun, so it's all good! |
| GIFT | Catch! One {ITEM} for you. Don't mention it. |
| BYE | Later! Let's have another blast sometime! |
| QUIRK | {BUDDY} shows up when trouble's coming. Me? I just like trouble. |

Hooks: "I like that look you're giving me", "That's good! Looking real good!"
and "Eh, it was fun, so it doesn't matter" (RSE/ORAS); Absol (RSE/ORAS ace;
the omen of disaster is from the POKéDEX).

### Phoebe

Buddy: slot 1, Dusknoir (DUSKULL, then DUSCLOPS). Anime and RSE Dusclops.

| Bit | Line |
| --- | --- |
| HELLO | Ahahaha! {PLAYER}! Hi! The ghosts said you'd drop by! |
| AGAIN | Ahahaha! You again! The ghosts remember you, too! |
| CLOSE | Ahahaha! {PLAYER}! My friend! Even the ghosts adore you! |
| MEET | Ahahaha! I'm PHOEBE! I can commune with Ghost POKéMON! |
| HEARD | Ahahaha! You're {PLAYER}! The ghosts whispered your name! |
| NOT_YET | Beat me in battle first! My ghosts are dying to meet you! |
| NEWS | Ooh, {BUDDY} whispered this to me! |
| ASK | Ooh, wait, wait! Listen! |
| YES | Ahahaha! Yay! Thank you! |
| NO | Aww, you're no fun! Oh well! |
| NOT_READY | Not yet! Come back when you're a bit stronger! |
| PRAISE | Ahahaha! Amazing! Even my ghosts got goosebumps! |
| GIFT | Take this {ITEM}! It's not haunted. Probably! Ahahaha! |
| BYE | Bye-bye! Ahahaha! Watch out for ghosts! |
| QUIRK | My grandma taught me to talk with ghosts. She's the best! |

Hooks: "Ahahaha!" and gaining "the ability to commune with Ghost-type
POKéMON" during training (RSE/ORAS); a grandmother tied to her training
(RSE/ORAS) (verify); Dusclops (RSE/ORAS ace, anime).

### Glacia

Buddy: slot 1, Walrein (SPHEAL, then SEALEO). Her RSE ace.

| Bit | Line |
| --- | --- |
| HELLO | {PLAYER}. So you've come. I hope you won't bore me. |
| AGAIN | You again. I may remember you after all. |
| CLOSE | {PLAYER}. You do not bore me. I count that as friendship. |
| MEET | I am GLACIA. I traveled from afar to hone my icy skills. |
| HEARD | So you are {PLAYER}. I have heard of you. Barely. |
| NOT_YET | Defeat me in battle first. So far, I've met only weaklings. |
| NEWS | I heard something. Perhaps it will interest you. |
| ASK | Mmm. Pay attention. |
| YES | Good. Don't make me regret it. |
| NO | As expected. How disappointing. |
| NOT_READY | No. I've no time for trainers who aren't ready. |
| PRAISE | How hot your spirit burns... I did not expect that. |
| GIFT | Take this {ITEM}. I have no use for it. |
| BYE | Go. Come back when you're worth the chill. |
| QUIRK | Only weak trainers, everywhere. Am I asking for too much? |

Hooks: "I've traveled from afar... to hone my icy skills. But all I have seen
are challenges by weak Trainers" (RSE/ORAS); "How hot your spirits burn!"
(RSE/ORAS); Walrein (RSE/ORAS ace).

### Drake

Buddy: slot 1, Salamence (BAGON, then SHELGON). His RSE ace.

| Bit | Line |
| --- | --- |
| HELLO | {PLAYER}. Good. You've still got fire in your eyes. |
| AGAIN | You again. I am beginning to take note of you. |
| CLOSE | {PLAYER}. A friend of dragons is a friend of mine. |
| MEET | I am DRAKE. I raise dragons. Do you know what that takes? |
| HEARD | You are {PLAYER}. Sailors speak of you. Rumors, at least. |
| NOT_YET | Defeat me in battle first. Then we'll talk as equals. |
| NEWS | Listen well. I'll not repeat myself. |
| ASK | Hmph. Listen. |
| YES | Good. That took virtue. |
| NO | Hmph. Knowing your limits is its own kind of wisdom. |
| NOT_READY | Not yet. Raising dragons takes virtue. So does this. |
| PRAISE | Superb! You have earned my respect. |
| GIFT | Take this {ITEM}. A worthy trainer should have it. |
| BYE | Go. And keep your heart strong. |
| QUIRK | Dragons heed only those with true virtue. Remember that. |

Hooks: "For us to battle with POKéMON as partners, do you know what it takes?
... You must have virtue" (RSE/ORAS) (verify); weathered
old-sailor look (RSE/ORAS design, not used in lines) (verify); Salamence
(RSE/ORAS ace).

### Wallace

Buddy: slot 1, Milotic (FEEBAS early). Anime and RSE Milotic.

| Bit | Line |
| --- | --- |
| HELLO | Ah, {PLAYER}. Elegant as ever, I see. |
| AGAIN | Ah, hello again! I do remember you fondly. |
| CLOSE | Dear {PLAYER}! A friend as elegant as you is rare. |
| MEET | Allow me to present myself. I am WALLACE. Elegance is my art. |
| HEARD | Ah, you are {PLAYER}! Your reputation is quite elegant. |
| NOT_YET | Battle me first, with all the grace you can muster. |
| NEWS | A little something I heard. Listen closely. |
| ASK | Ah, a moment, if you please. |
| YES | Splendid! Your answer shines brilliantly. |
| NO | A pity. But I won't insist. |
| NOT_READY | Not yet. Polish your style a little more. |
| PRAISE | Magnificent! Your style is truly elegant. |
| GIFT | Please accept this {ITEM}. It suits you beautifully. |
| BYE | Farewell! May your style shine ever brighter. |
| QUIRK | {BUDDY} taught me that true beauty takes patience. Lovely, no? |

Hooks: elegance and POKéMON Contests (Emerald/ORAS, anime Wallace Cup);
Milotic (RSE/Emerald/ORAS ace, anime), which the patience line fits from
FEEBAS up; student of Juan (Emerald, not used in lines).

### Steven

Buddy: slot 1, Metagross (BELDUM, then METANG). His RSE ace and Masters EX
partner.

| Bit | Line |
| --- | --- |
| HELLO | {PLAYER}! Good to see you. Found any rare stones? |
| AGAIN | Oh, hello again! Good to see you back. |
| CLOSE | {PLAYER}! My friend! I saved you a special stone to see. |
| MEET | I'm STEVEN. I'm crazy about rare stones. Nice to meet you. |
| HEARD | You're {PLAYER}? I've heard of you! Like a rare gem! |
| NOT_YET | Defeat me in battle first. I'll be waiting. |
| NEWS | I heard something interesting. Want to hear it? |
| ASK | Oh, pardon me. Something just occurred to me. |
| YES | Splendid. I'm glad you're in. |
| NO | That's fine. I'll manage. |
| NOT_READY | Not yet. Give it more time, then come find me. |
| PRAISE | Impressive. You and your POKéMON really shine. |
| GIFT | Take this {ITEM}. Think of it as a token of friendship. |
| BYE | I'll see you around. Take care. |
| QUIRK | Sorry, I was admiring a stone. They're all so fascinating. |

Hooks: rare-stone collector (RSE/ORAS, Masters EX); Metagross (RSE/ORAS ace,
Masters EX sync pair).

---

## Open questions

1. **Koga's and Lance's grouping** follows catalog `homeRegion` (Koga is
   Johto, Lance is Kanto). Flip either if haunts group by first appearance
   instead.
2. **Lt. Surge's "war" line** is canon and kept light. Drop it if even that
   feels off-tone.
3. **Length measurement.** Line lengths are counted in characters with slots
   at their worst-case lengths. The GBA box is measured in pixels, not
   characters. Check the lines with long `{ITEM}`, `{ACE}` and `{BUDDY}`
   values, the AMPHY nickname, and the Tate & Liza speaker labels, in-engine.
4. **Unverified wording.** Items marked `(verify)` are canon I'm confident of
   in spirit but not in exact source or wording: Brock "rock-hard willpower",
   Janine's disguise line, Lorelei's doll collection, Morty "see what you
   cannot", Pryce and Piloswine, Roxanne's book-versus-battle lesson and
   Probopass facing north, Wattson's traps and gadgets, Juan's laugh, Phoebe's
   grandmother, and Drake's "virtue" line and sailor look.
