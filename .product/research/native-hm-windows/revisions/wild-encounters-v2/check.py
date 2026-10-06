"""Check encounter tables against the wild-encounters-v2 specs.

Usage: python3 check.py                      (checks every region in data/)
       python3 check.py data/kanto.json ...  (the region comes from the file name)
Reads data/*.json (tables, meta.json, species.json, tiles) and specs/prowlers.md, specs/reach-assignments.md.

Table data format, one file per region:
  { "MAP_ROUTE1_HNS": { "water_type": "pond",
                        "land": {"day": [12 species], "night": [12 species]},
                        "surf": {...5...}, "fish": {...10...}, "rock": {...5...} } }
Each species is a SPECIES_ name without the prefix, written as the slot's stage cap:
"PIDGEOT" means the Pidgey line up to Pidgeot, "METAPOD" caps Caterpie's line at Metapod.
A baby (PICHU, AZURILL, ...) is a rare baby slot of its own.
"""
import json, os, re, sys
from collections import defaultdict

HERE = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'data')  # the committed table data
REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../../../../..')) + '/.product/'
DATA = json.load(open(os.path.join(HERE, 'species.json')))
SP, BABIES = DATA['species'], set(DATA['babies'])
META = json.load(open(os.path.join(HERE, 'meta.json')))

SLOTS = {'land': 12, 'surf': 5, 'rock': 5, 'fish': 10}
WEIGHTS = {'land': [20, 20, 10, 10, 10, 10, 5, 5, 4, 4, 1, 1], 'surf': [60, 30, 5, 4, 1], 'rock': [60, 30, 5, 4, 1],
           'fish': [25, 16.7, 11, 9.3, 9, 7, 6.3, 5.7, 5, 5]}  # fishing: average of the Standard Rod's three qualities
RARE_FROM = {'land': 6, 'surf': 2, 'rock': 2, 'fish': 6}
_ROSTER = json.load(open(os.path.join(HERE, '..', 'roster_v2.json')))['roster']  # the v2 native HM roster
CARRIERS = {mv: {e['species'].removeprefix('SPECIES_') for e in _ROSTER if 'MOVE_' + mv in e['roles']} for mv in ('SURF', 'WHIRLPOOL')}

KANTO_CASTS = {
    'pond': {'MAGIKARP', 'POLIWAG', 'PSYDUCK', 'GOLDEEN', 'SLOWPOKE', 'MARILL', 'WOOPER'},
    'sea': {'MAGIKARP', 'TENTACOOL', 'KRABBY', 'SHELLDER', 'STARYU', 'HORSEA', 'CHINCHOU', 'CORSOLA', 'QWILFISH', 'REMORAID', 'SLOWPOKE', 'PSYDUCK', 'MANTINE'},
    'cold': {'MAGIKARP', 'SEEL', 'SHELLDER', 'HORSEA', 'KRABBY', 'TENTACOOL', 'SLOWPOKE', 'PSYDUCK', 'LAPRAS'},
    'cave': {'MAGIKARP', 'ZUBAT', 'GOLDEEN', 'PSYDUCK', 'SLOWPOKE', 'POLIWAG', 'MARILL', 'WOOPER'}}
CFG = {
    'Kanto': dict(
        natives={1, 2}, share_gen=2, reward_sections=('Generation I',), cover_gens=(1,),
        bands={'Kanto west': (10, 30), 'Kanto east': (5, 20), 'Border': (20, 40)},
        anchors=[('Route 1', 'PIDGEY'), ('Route 1', 'RATTATA'), ('Viridian Forest', 'PIKACHU'), ('Mt. Moon', 'CLEFAIRY'),
                 ('Rock Tunnel', 'ONIX'), ("Diglett's Cave", 'DIGLETT'), ('Power Plant', 'MAGNEMITE'), ('Power Plant', 'VOLTORB'),
                 ('Pokémon Tower', 'GASTLY'), ('Pokémon Tower', 'CUBONE'), ('Pokémon Mansion', 'GRIMER'), ('Pokémon Mansion', 'KOFFING'),
                 ('Seafoam Islands', 'SEEL'), ('Seafoam Islands', 'JYNX'), ('Cerulean Cave', 'DITTO')],
        banned={'LARVITAR', 'MAREEP', 'HOPPIP', 'STANTLER', 'MANTINE', 'AIPOM'}, limits={'WOOPER': 4},
        fossils={'OMANYTE', 'KABUTO'}, surf_first_cap=8, casts=KANTO_CASTS,
        crossings={'MAP_VERMILION_CITY_HNS': 'SURF', 'MAP_VERMILION_CITY_PORT_OUTSIDE_HNS': 'SURF', 'MAP_CINNABAR_ISLAND': 'SURF'}),
    'Johto': dict(
        natives={1, 2}, share_gen=2, reward_sections=('Generation II',), cover_gens=(2,),
        bands={'Johto west': (40, 60), 'Johto east': (30, 50), 'Border': (20, 40)},
        anchors=[('Route 29', 'SENTRET'), ('Route 32', 'MAREEP'), ('Route 32', 'WOOPER'), ('Union Cave', 'WOOPER'), ('Union Cave', 'LAPRAS'),
                 ('Slowpoke Well', 'SLOWPOKE'), ('Ruins of Alph', 'NATU'), ('Ruins of Alph', 'UNOWN'),
                 ('Ice Path', 'SNEASEL'), ("Dragon's Den", 'DRATINI'), ('*', 'AIPOM'), ('*', 'HERACROSS')],
        banned={'KANGASKHAN', 'CHANSEY'}, limits={}, fossils=set(), surf_first_cap=8, casts=KANTO_CASTS,
        crossings={'MAP_OLIVINE_CITY_HNS': 'SURF', 'MAP_OLIVINE_CITY_PORT_OUTSIDE_HNS': 'SURF', 'MAP_CIANWOOD_CITY_HNS': 'SURF',
                   'MAP_BLACKTHORN_CITY_HNS': 'SURF', 'MAP_DRAGONS_DEN_CAVERN_HNS': 'WHIRLPOOL'}),
    'Hoenn': dict(
        natives={3, 5}, share_gen=5, reward_sections=('Generation III', 'Generation V'), cover_gens=(3, 5),
        bands={'West': (5, 25), 'Centre': (25, 45), 'East': (35, 55), 'Far east': (45, 65)},
        anchors=[('Route 101', 'ZIGZAGOON'), ('Route 101', 'WURMPLE'), ('Route 102', 'POOCHYENA'), ('Route 102', 'RALTS'),
                 ('Petalburg Woods', 'SHROOMISH'), ('Petalburg Woods', 'SLAKOTH'), ('Route 104', 'TAILLOW'), ('Route 104', 'WINGULL'),
                 ('Granite Cave', 'MAKUHITA'), ('Granite Cave', 'ARON'), ('Granite Cave', 'SABLEYE'), ('Granite Cave', 'MAWILE'),
                 ('Route 110', 'ELECTRIKE'), ('Route 110', 'PLUSLE'), ('Route 110', 'MINUN'), ('Route 111', 'TRAPINCH'),
                 ('Route 111', 'CACNEA'), ('Route 111', 'BALTOY'), ('Route 113', 'SPINDA'), ('Fiery Path', 'NUMEL'),
                 ('Fiery Path', 'TORKOAL'), ('Jagged Pass', 'SPOINK'), ('Route 117', 'VOLBEAT'), ('Route 117', 'ILLUMISE'),
                 ('Route 117', 'ROSELIA'), ('Meteor Falls', 'LUNATONE'), ('Meteor Falls', 'SOLROCK'), ('Meteor Falls', 'BAGON'),
                 ('Route 119', 'KECLEON'), ('Route 119', 'FEEBAS'), ('Route 120', 'ABSOL'), ('Route 120', 'KECLEON'),
                 ('Mt. Pyre', 'SHUPPET'), ('Mt. Pyre', 'DUSKULL'), ('Mt. Pyre', 'CHIMECHO'), ('Shoal Cave', 'SPHEAL'),
                 ('Shoal Cave', 'SNORUNT'), ('Sky Pillar', 'BALTOY'), ('Sky Pillar', 'SHUPPET'), ('Sky Pillar', 'SABLEYE'),
                 ('Sky Pillar', 'SWABLU'), ('Victory Road', 'MAKUHITA'), ('Victory Road', 'ARON'), ('Victory Road', 'MEDITITE')],
        banned=set(), limits={}, fossils={'LILEEP', 'ANORITH', 'TIRTOUGA', 'ARCHEN'}, surf_first_cap=12,
        crossings={'MAP_ROUTE118': 'SURF', 'MAP_LILYCOVE_CITY': 'SURF', 'MAP_MOSSDEEP_CITY': 'SURF', 'MAP_PACIFIDLOG_TOWN': 'SURF'},
        casts={'pond': {'LOTAD', 'SURSKIT', 'BARBOACH', 'CORPHISH', 'CARVANHA', 'MARILL', 'AZURILL', 'BASCULIN', 'TYMPOLE', 'DUCKLETT'},
               'sea': {'WINGULL', 'WAILMER', 'CARVANHA', 'LUVDISC', 'CORPHISH', 'FRILLISH', 'ALOMOMOLA', 'DUCKLETT', 'TYNAMO'},
               'underwater': {'CLAMPERL', 'RELICANTH', 'FRILLISH', 'ALOMOMOLA'},
               'cold': {'SPHEAL', 'WAILMER', 'CARVANHA', 'FRILLISH'},
               'cave': {'BARBOACH', 'CORPHISH', 'MARILL', 'TYMPOLE', 'BASCULIN', 'WINGULL', 'WAILMER', 'CARVANHA', 'LUVDISC', 'FRILLISH', 'ALOMOMOLA', 'TYNAMO'}}),
    'Alola': dict(
        natives={7}, share_gen=None, reward_sections=('Generation VII',), cover_gens=(7,),
        bands={'Melemele': (0, 20), 'Akala': (0, 20), "Ula'ula": (0, 20), 'Poni': (0, 20), 'Alola sea': (0, 20)},
        blend_gens={1, 2, 3, 5}, blend_homes=('kanto.json', 'johto.json', 'hoenn.json'),
        regional='_ALOLA', form_bases={'RATTATA_ALOLA', 'PIKACHU', 'SANDSHREW_ALOLA', 'VULPIX_ALOLA', 'DIGLETT_ALOLA', 'MEOWTH_ALOLA',
                      'GEODUDE_ALOLA', 'GRIMER_ALOLA', 'EXEGGCUTE', 'CUBONE'},
        anchors=[], banned=set(), limits={}, fossils=set(), surf_first_cap=4, lead_cap=4, crossings={},
        casts={'sea': {'WISHIWASHI', 'MAREANIE', 'PYUKUMUKU', 'BRUXISH', 'DHELMISE', 'POPPLIO', 'MAGIKARP', 'TENTACOOL', 'WINGULL',
                       'CHINCHOU', 'STARYU', 'SHELLDER', 'CORSOLA', 'LUVDISC', 'WAILMER', 'CARVANHA', 'CLAMPERL', 'ALOMOMOLA'},
               'cave': {'DEWPIDER', 'WISHIWASHI', 'MAGIKARP', 'BARBOACH', 'PSYDUCK', 'BASCULIN', 'ZUBAT'}})}
CFG['Sevii'] = dict(
    natives={8}, share_gen=None, reward_sections=('Generation VIII',), cover_gens=(8,), cover_max_nat=898,
    bands={'Near islands': (0, 20), 'Outer islands': (0, 10)},
    blend_gens={1, 2, 3, 5, 7}, blend_homes=('kanto.json', 'johto.json', 'hoenn.json', 'alola.json'),
    regional='_GALAR', form_bases={'MEOWTH_GALAR', 'PONYTA_GALAR', 'SLOWPOKE_GALAR', 'FARFETCHD_GALAR', 'KOFFING', 'MR_MIME_GALAR',
                                   'CORSOLA_GALAR', 'ZIGZAGOON_GALAR', 'DARUMAKA_GALAR', 'YAMASK_GALAR', 'STUNFISK_GALAR'},
    anchors=[], banned=set(), limits={}, fossils={'DRACOZOLT', 'ARCTOZOLT', 'DRACOVISH', 'ARCTOVISH'}, surf_first_cap=12, crossings={},
    casts={'sea': {'CHEWTLE', 'ARROKUDA', 'PINCURCHIN', 'CLOBBOPUS', 'CRAMORANT', 'CORSOLA_GALAR', 'SLOWPOKE_GALAR', 'SOBBLE',
                   'MAGIKARP', 'TENTACOOL', 'HORSEA', 'KRABBY', 'SHELLDER', 'STARYU', 'WINGULL', 'WAILMER', 'REMORAID', 'QWILFISH',
                   'CHINCHOU', 'MAREANIE', 'PYUKUMUKU', 'WISHIWASHI', 'LUVDISC', 'FRILLISH', 'ALOMOMOLA'},
           'pond': {'CHEWTLE', 'ARROKUDA', 'STUNFISK_GALAR', 'SOBBLE', 'SLOWPOKE_GALAR', 'MAGIKARP', 'PSYDUCK', 'POLIWAG', 'GOLDEEN',
                    'BASCULIN', 'MARILL', 'WOOPER', 'TYMPOLE', 'DEWPIDER', 'LOTAD', 'BARBOACH', 'SLOWPOKE_GALAR'},
           'cold': {'EISCUE', 'ARROKUDA', 'CLOBBOPUS', 'MAGIKARP', 'SEEL', 'SHELLDER', 'SPHEAL', 'LAPRAS', 'HORSEA', 'TENTACOOL'}})
KALOS_WATER = {'BINACLE', 'CLAUNCHER', 'SKRELP', 'INKAY', 'FROAKIE', 'BERGMITE'}
CFG['Safari'] = dict(
    natives={6}, share_gen=None, reward_sections=('Generation VI',), cover_gens=(6,), cover_max_nat=721,
    bands={}, anchors=[], banned=set(), limits={}, fossils={'TYRUNT', 'AMAURA'}, surf_first_cap=12, crossings={},
    any_temperament=True,  # Safari mode: no battles to lose
    casts={'sea': KALOS_WATER, 'pond': KALOS_WATER, 'cold': KALOS_WATER, 'cave': KALOS_WATER})
SINJOH_GEN4 = {'SNOVER', 'RIOLU', 'BRONZOR', 'DRIFLOON', 'CHINGLING', 'CROAGUNK', 'BUIZEL', 'SHELLOS', 'FINNEON'}
CFG['Sinjoh'] = dict(
    natives={8}, share_gen=None, reward_sections=('Generation IV',), cover_gens=(), regional='_HISUI',
    extra_natives={'BASCULIN_WHITE_STRIPED'}, residents=True,
    blend_fierce_ok=True,  # Sinjoh's large blend includes Stantler and Scyther, the bases of Wyrdeer and Kleavor
    resident_rewards={'VOLTORB_HISUI', 'ZORUA_HISUI', 'GROWLITHE_HISUI', 'SNEASEL_HISUI', 'QWILFISH_HISUI', 'BASCULIN_WHITE_STRIPED',
                      'STANTLER', 'SCYTHER'},  # other rewards: rare slots only, at most three species per map
    lead_cap=5,  # Hisuian natives are residents at home: common slots allowed
    reward_only={'VOLTORB_HISUI', 'ZORUA_HISUI', 'GROWLITHE_HISUI', 'SNEASEL_HISUI', 'QWILFISH_HISUI', 'BASCULIN_WHITE_STRIPED'},
    bands={'Sinjoh': (30, 55)}, blend_gens={1, 2, 3, 4, 5}, blend_homes=('kanto.json', 'johto.json', 'hoenn.json', 'alola.json'),
    gen4_ok=SINJOH_GEN4,
    form_bases={'GROWLITHE_HISUI', 'VOLTORB_HISUI', 'SNEASEL_HISUI', 'QWILFISH_HISUI', 'ZORUA_HISUI', 'BASCULIN_WHITE_STRIPED',
                'STANTLER', 'SCYTHER', 'URSARING'},
    # evolve-only Hisuian forms keep their lines out; plain forms of the Hisuian natives stay out too
    banned={'CYNDAQUIL', 'OSHAWOTT', 'ROWLET', 'PETILIL', 'RUFFLET', 'GOOMY', 'BERGMITE',
            'GROWLITHE', 'VOLTORB', 'SNEASEL', 'QWILFISH', 'ZORUA', 'BASCULIN', 'BASCULIN_RED_STRIPED', 'BASCULIN_BLUE_STRIPED'},
    anchors=[], limits={}, fossils=set(), surf_first_cap=8, crossings={},
    casts={'pond': {'BASCULIN', 'BUIZEL', 'REMORAID', 'MAGIKARP', 'BARBOACH', 'PSYDUCK',
                    'SHELLOS', 'FINNEON'}})  # Route 49's lake: Shellos and Finneon, like Hisuian Qwilfish, are recorded exceptions
# Forms that share a national dex entry with a listed species
FORM_BASE = {'ORICORIO_POM_POM': 'ORICORIO', 'ORICORIO_PAU': 'ORICORIO', 'ORICORIO_SENSU': 'ORICORIO', 'ORICORIO_BAILE': 'ORICORIO',
             'SINISTEA_PHONY': 'SINISTEA', 'LYCANROC_MIDNIGHT': 'LYCANROC', 'LYCANROC_DUSK': 'LYCANROC', 'LYCANROC_MIDDAY': 'LYCANROC', 'ROCKRUFF_OWN_TEMPO': 'ROCKRUFF'}
LEGENDARY = {'ARTICUNO', 'ZAPDOS', 'MOLTRES', 'MEWTWO', 'MEW', 'RAIKOU', 'ENTEI', 'SUICUNE', 'LUGIA', 'HO_OH', 'CELEBI',
             'REGIROCK', 'REGICE', 'REGISTEEL', 'LATIAS', 'LATIOS', 'KYOGRE', 'GROUDON', 'RAYQUAZA', 'JIRACHI', 'DEOXYS',
             'VICTINI', 'COBALION', 'TERRAKION', 'VIRIZION', 'TORNADUS', 'THUNDURUS', 'RESHIRAM', 'ZEKROM', 'LANDORUS',
             'KYUREM', 'KELDEO', 'MELOETTA', 'GENESECT', 'TYPE_NULL', 'SILVALLY', 'TAPU_KOKO', 'TAPU_LELE', 'TAPU_BULU', 'TAPU_FINI',
             'COSMOG', 'COSMOEM', 'SOLGALEO', 'LUNALA', 'NIHILEGO', 'BUZZWOLE', 'PHEROMOSA', 'XURKITREE', 'CELESTEELA', 'KARTANA',
             'GUZZLORD', 'NECROZMA', 'MAGEARNA', 'MARSHADOW', 'POIPOLE', 'NAGANADEL', 'STAKATAKA', 'BLACEPHALON', 'ZERAORA',
             'MELTAN', 'MELMETAL', 'ZACIAN', 'ZAMAZENTA', 'ETERNATUS', 'KUBFU', 'URSHIFU', 'ZARUDE', 'REGIELEKI', 'REGIDRAGO',
             'GLASTRIER', 'SPECTRIER', 'CALYREX', 'XERNEAS', 'YVELTAL', 'ZYGARDE', 'DIANCIE', 'HOOPA', 'VOLCANION'}

# Each map's encounter terrain, from scan_tiles.py (source layouts of main, script-swapped layouts included)
TILES = {}
for _f in ('tiles.json', 'safari_tiles.json', 'sinjoh_tiles.json'):
    TILES.update(json.load(open(os.path.join(HERE, _f))))
# Terrain that deliberately has no table
NO_LAND = {  # Emerald's MOUNTAIN_TOP rocks and logs, which never had encounters; Dragon's Den's shrine floor; the Abandoned Ship's indoor floors
    'MAP_LILYCOVE_CITY', 'MAP_MOSSDEEP_CITY', 'MAP_SOOTOPOLIS_CITY', 'MAP_PACIFIDLOG_TOWN', 'MAP_EVER_GRANDE_CITY', 'MAP_ROUTE105',
    'MAP_ROUTE106', 'MAP_ROUTE107', 'MAP_ROUTE109', 'MAP_ROUTE122', 'MAP_ROUTE124', 'MAP_ROUTE125', 'MAP_ROUTE126', 'MAP_ROUTE127',
    'MAP_ROUTE128', 'MAP_ROUTE129', 'MAP_ROUTE131', 'MAP_ROUTE132', 'MAP_ROUTE133', 'MAP_ROUTE134', 'MAP_SEVEN_ISLAND_TANOBY_RUINS',
    'MAP_DRAGONS_DEN_CAVERN_HNS', 'MAP_ABANDONED_SHIP_ROOMS_B1F', 'MAP_ABANDONED_SHIP_HIDDEN_FLOOR_CORRIDORS', 'MAP_SOUTHERN_ISLAND_EXTERIOR_HNS'}
PATH_ROCKS = {  # rocks that only clear a path or solve a puzzle
    'MAP_ROUTE115', 'MAP_RUSTURF_TUNNEL', 'MAP_MIRAGE_TOWER_3F', 'MAP_MIRAGE_TOWER_4F', 'MAP_SEAFLOOR_CAVERN_ROOM1',
    'MAP_SEAFLOOR_CAVERN_ROOM2', 'MAP_SEAFLOOR_CAVERN_ROOM5', 'MAP_FOUR_ISLAND'}
# Species that rest at night outdoors, by region: Hoenn's day birds, and the Safari Zones' day-only Kalos species
DAY_ONLY = {'Hoenn': {'TAILLOW', 'PIDOVE', 'SWABLU', 'WINGULL', 'DUCKLETT', 'VULLABY', 'RUFFLET'},
            'Safari': {'FLETCHLING', 'HELIOPTILE', 'HAWLUCHA', 'SCATTERBUG'}}
INDOORS = ('UNDERGROUND', 'INDOOR', 'NONE', 'UNDERWATER')  # caves and buildings change more lightly
MIN_TILES = 10  # smaller grass patches and puddles get no table

errors, warnings = [], []
def err(m): errors.append(m)
def warn(m): warnings.append(m)

def gen(name):
    n = SP.get(name, {}).get('nat') or nat_of(name) or 0  # forms such as FLOETTE_RED take their species' number
    for g, hi in enumerate([151, 251, 386, 493, 649, 721, 809, 905, 1025], 1):
        if n and n <= hi: return g
    return 0

PRED = {}
for s, v in SP.items():
    for e in v['evolves_to']:
        PRED.setdefault(e['target'], (s, e['method'], e['param']))

def chain(cap):
    """Species reachable from a cap by the downward rule (never into a baby), cap first."""
    out = [cap]
    if cap in BABIES: return out
    x = cap
    while x in PRED:
        p = PRED[x][0]
        if p in BABIES: break
        out.append(p); x = p
    return out

def full_line(cap):
    """The cap and every earlier stage, babies included."""
    out = [cap]; x = cap
    while x in PRED: x = PRED[x][0]; out.append(x)
    return out

def base(cap): return chain(cap)[-1]

def load_rewards():
    text = open(REPO + 'specs/prowlers.md').read()
    out = {}
    for sec in re.findall(r'^### (Generation [IVX]+)\b[^\n]*rewards$', text, re.M):
        a = re.search(r'^### ' + sec + r'\b[^\n]*rewards$', text, re.M).start()  # not the first section whose name starts with it (V vs VI)
        b = text.find('\n### ', a + 5); b = len(text) if b < 0 else b
        for line in text[a:b].split('\n'):
            c = [x.strip() for x in line.strip('|').split('|')]
            if len(c) == 5 and c[0] not in ('Species', '---'):
                m_ = re.match(r'(.*) \(([^)]*)\)$', c[0])
                if m_: c[0] = m_.group(1) + '_' + {'Alolan': 'ALOLA', 'Galarian': 'GALAR', 'Hisuian': 'HISUI'}.get(m_.group(2), m_.group(2).replace('-', '_'))
                name = re.sub(r' line$', '', c[0]).upper().replace("'", '').replace('. ', '_').replace(' ', '_').replace('-', '_')
                out.setdefault(name, (sec, c[2]))
    return out
REWARDS = load_rewards()
EVOLUTION_REWARDS = {k for k, v in REWARDS.items() if v[1] == '—'}

def non_level_cap(cap):
    if cap in EVOLUTION_REWARDS: return True  # e.g. Togetic, whose pre-evolution is a baby
    p = PRED.get(cap)
    if not p or cap in BABIES or p[0] in BABIES: return False
    return not (p[1].startswith('LEVEL') and p[2].isdecimal() and int(p[2]) > 0)

def reward_of(cap):
    for s in chain(cap):
        s = FORM_BASE.get(s, s)
        if s in REWARDS and REWARDS[s][1] != '—': return s
    return None

def native(cap, gens):
    family = set(full_line(cap))
    if cap in BABIES: family |= {e['target'] for e in SP[cap]['evolves_to']}
    return any(gen(s) in gens for s in family)

def counts_as(cap, g):
    """Whether a slot counts towards generation g's share: a baby by its own generation, else its line's base."""
    if cap in BABIES: return gen(cap) == g
    if g == 3 and any(gen(s) == 3 for s in full_line(cap)): return True  # e.g. Marill's line through Azurill
    return gen(base(cap)) == g

def regional(cap, suffix):
    return any(s.endswith(suffix) for s in full_line(cap))

def blend(cap, cfg):
    """Alola and Sevii: a slot from outside the native generation and its regional forms."""
    if cap in BABIES: return gen(cap) not in cfg['natives']
    if base(cap) in cfg.get('extra_natives', ()) or gen(cap) in cfg['natives']: return False
    return not regional(cap, cfg['regional']) and gen(FORM_BASE.get(base(cap), base(cap))) not in cfg['natives']

def nat_of(s):
    s = FORM_BASE.get(s, s)
    while s and not SP.get(s, {}).get('nat') and '_' in s: s = s.rsplit('_', 1)[0]  # VIVILLON_MEADOW -> VIVILLON
    return SP.get(s, {}).get('nat')

def nat_line_base(s):
    """The earliest stage reached by walking back through predecessors, forms included."""
    while s in PRED and nat_of(PRED[s][0]): s = PRED[s][0]
    return s

def in_cast(cap, cast):
    return any(s.split('_')[0] in cast for s in full_line(cap))

def check(region, tables):
    cfg = CFG[region]
    maps = {m: v for m, v in META.items() if v['region'] == region}
    present_nat = set(); weight_by_band = defaultdict(lambda: [0.0, 0.0]); lead = defaultdict(set)
    rewards_seen = set(); anchor_hits = set(); limited = defaultdict(set); present_sp = set()
    HOMES = set()
    for h in cfg.get('blend_homes', ()):
        for t_ in json.load(open(os.path.join(HERE, h))).values():
            for mt_ in t_.values():
                if isinstance(mt_, dict):
                    for sl in mt_.values():
                        for c in sl: HOMES.update(full_line(c))
    for m, meta in maps.items():
        contest = ':' in m  # a Bug-Catching Contest day: bugs from every included generation, Wilds rules
        t = tables.get(m)
        if t is None: err(f'{region} {m} ({meta["place"]}): no tables'); continue
        wt = t.get('water_type')
        if set(meta['methods']) & {'surf', 'fish'} and wt not in cfg['casts']:
            err(f'{m}: needs a water_type ({", ".join(cfg["casts"])})')
        for meth in meta['methods']:
            if meth not in t: err(f'{m}: missing method {meth}'); continue
            for time in ('day', 'night'):
                slots = t[meth].get(time)
                if not slots or len(slots) != SLOTS[meth]:
                    err(f'{m} {meth} {time}: needs {SLOTS[meth]} slots'); continue
                for i, cap in enumerate(slots):
                    if cap not in SP: err(f'{m} {meth} {time}: unknown species {cap}'); continue
                    w = WEIGHTS[meth][i]
                    weight_by_band[meta['band']][0] += w
                    if 'blend_gens' in cfg:
                        if blend(cap, cfg):
                            weight_by_band[meta['band']][1] += w
                            if gen(base(cap)) not in cfg['blend_gens'] and gen(cap) not in cfg['blend_gens']: err(f'{m}: blend {cap} is from an excluded generation')
                            if i < 2: err(f'{m} {meth} {time}: blend {cap} leads the table')
                            if gen(base(cap)) == 4:  # Gen IV has no region yet: only the species that fit, by name
                                if base(cap).split('_')[0] not in cfg.get('gen4_ok', ()): err(f'{m}: Gen IV {cap} is not one of the species allowed in {region}')
                            elif base(cap) not in HOMES and cap not in HOMES: err(f'{m}: blend {cap} is not catchable in its home region')
                            r_ = reward_of(cap)
                            if r_ and REWARDS[r_][1] in (('Dangerous',) if cfg.get('blend_fierce_ok') else ('Fierce', 'Dangerous')):
                                err(f'{m}: blend {cap} is a {REWARDS[r_][1].lower()} reward')
                    elif not contest:
                        if not native(cap, cfg['natives']) and nat_of(cap) and not (650 <= nat_of(cap) <= 721):
                            err(f'{m} {meth} {time}: {cap} is not a native of {region}')
                        if counts_as(cap, cfg['share_gen']): weight_by_band[meta['band']][1] += w
                    if not contest:
                        for x in chain(cap): present_nat.add(nat_of(x)); present_sp.add(x)
                    if cap in BABIES:  # catching a baby covers the stage it evolves into
                        for e in SP[cap]['evolves_to']: present_nat.add(SP.get(e['target'], {}).get('nat'))
                    if i < 2 and meth in ('land', 'rock'): lead[(meth, base(cap))].add(m)
                    if i == 0 and meth == 'surf': lead[('surf-first', base(cap))].add(m)
                    if cap in BABIES and i < RARE_FROM[meth]: err(f'{m} {meth} {time}: baby {cap} must be in a rare slot')
                    if time == 'night' and meta['map_type'] not in INDOORS and DAY_ONLY.get(region, set()) & {x.split('_')[0] for x in full_line(cap)}:
                        err(f'{m} {meth} {time}: {cap} rests at night outdoors')
                    if cap in LEGENDARY: err(f'{m}: legendary {cap} belongs to a later spec')
                    if non_level_cap(cap):
                        deep = meta['reach'] == 'Outlands' or (meta['reach'] == 'Dungeon' and (meta['floors'] == 1 or meta['floor'] >= 1))
                        if not deep: err(f'{m} ({meta["reach"]}) {meth} {time}: {cap} needs item/trade/friendship; only Outlands or deeper dungeon floors')
                    r = reward_of(cap)
                    if r:
                        rewards_seen.add(r); temp = REWARDS[r][1]
                        free = cfg.get('any_temperament') and not contest
                        if temp == 'Fierce' and meta['reach'] == 'Road' and not free: err(f'{m} (Road): fierce reward {r}')
                        if temp == 'Dangerous' and meta['reach'] not in ('Outlands', 'Dungeon') and not free: err(f'{m} ({meta["reach"]}): dangerous reward {r}')
                        if temp == 'Dangerous' and meta['reach'] == 'Dungeon' and meta['floors'] > 1 and meta['floor'] == 0 and not free:
                            err(f'{m} {meth} {time}: dangerous reward {r} on the first floor step (only deeper floors, or a single-floor or flat dungeon)')
                    if base(cap) in cfg['banned']: err(f"{m}: {cap} is another region's signature and stays out of {region}")
                    if base(cap) in cfg['limits']: limited[base(cap)].add(m)
                    if meth in ('surf', 'fish') and wt in cfg['casts'] and not in_cast(cap, cfg['casts'][wt]) and not reward_of(cap) and cap not in BABIES:
                        warn(f'{m} {meth} {time}: {cap} is outside the {wt} cast')
                    for place, a in cfg['anchors']:
                        if a in full_line(cap) and (place == '*' or place == meta['place']): anchor_hits.add((place, a))
            if meth == 'fish' and len(t[meth].get('day', [])) == 10:
                for time in ('day', 'night'):
                    f = t[meth][time]
                    if len({base(c) for c in f[2:]}) < 4: err(f'{m} fish {time}: entries 3-10 need at least four families')
                    if m in cfg['crossings']:
                        mv = cfg['crossings'][m]
                        if not any(set(chain(c)) & CARRIERS[mv] and c != 'MAGIKARP' for c in f[:3]):  # a Magikarp slot never knows the move; Gyarados can (hm_audit.py checks levels)
                            err(f'{m} fish {time}: crossing needs a {mv.lower()} carrier in entries 1-3')
            if 'day' in t[meth] and 'night' in t[meth] and len(t[meth]['day']) == SLOTS[meth] and len(t[meth]['night']) == SLOTS[meth]:
                day_lines = {base(c) for c in t[meth]['day']}
                total = sum(WEIGHTS[meth])
                night_only = sum(WEIGHTS[meth][i] for i, c in enumerate(t[meth]['night']) if base(c) not in day_lines)
                light = meta['map_type'] in ('UNDERGROUND', 'INDOOR', 'NONE', 'UNDERWATER') or meth in ('fish', 'rock')
                if not light and meth == 'land' and night_only < 0.3 * total:
                    err(f'{m} land: only {night_only/total:.0%} night-only weight (needs 30%)')
                if light or meth == 'surf':
                    dw = defaultdict(float); nw = defaultdict(float)
                    for i, c in enumerate(t[meth]['day']): dw[base(c)] += WEIGHTS[meth][i]
                    for i, c in enumerate(t[meth]['night']): nw[base(c)] += WEIGHTS[meth][i]
                    if not any(nw[k] > 0 and (dw[k] == 0 or nw[k] >= 2 * dw[k]) for k in nw):
                        err(f'{m} {meth}: night needs a night-only or much more common species')
        if m.split(':')[0] in TILES:
            tl = TILES[m.split(':')[0]]
            for meth_, n_ in (('land', tl['land']), ('surf', tl['water']), ('fish', tl['water']), ('rock', tl['trees'] + tl['rocks'])):
                if meth_ in t and n_ == 0: err(f'{m}: {meth_} table but the map has no such terrain')
                exempt = (n_ < MIN_TILES and meth_ != 'rock') or (meth_ == 'land' and m in NO_LAND) or (meth_ == 'rock' and m in PATH_ROCKS) \
                    or (meth_ == 'fish' and meta['map_type'] == 'UNDERWATER')
                if meth_ not in t and n_ > 0 and not exempt: err(f'{m}: the map has {meth_} terrain ({n_}) but no table')
        if cfg.get('any_temperament') and not contest:  # the reserve: rewards are residents, never leading, at most 10% of a table
            for meth_, tb_ in t.items():
                if not isinstance(tb_, dict): continue
                for time_, row in tb_.items():
                    wsum = defaultdict(float)
                    for i_, cap_ in enumerate(row):
                        r_ = reward_of(cap_)
                        if r_:
                            wsum[r_] += WEIGHTS[meth_][i_]
                            if i_ < 2: err(f'{m} {meth_} {time_}: reward {cap_} in a top-two slot')
                    for r_, w_ in wsum.items():
                        if w_ > 10.5: err(f'{m} {meth_} {time_}: reward {r_} at {w_:.0f}% (limit 10%)')
        if cfg.get('resident_rewards') is not None:
            others = set()
            for meth_, tb_ in t.items():
                if not isinstance(tb_, dict): continue
                for time_, row in tb_.items():
                    wsum = defaultdict(float)
                    for i_, cap_ in enumerate(row):
                        r_ = reward_of(cap_)
                        if r_ and r_ not in cfg['resident_rewards']:
                            others.add(r_); wsum[r_] += WEIGHTS[meth_][i_]
                            if i_ < RARE_FROM[meth_]: err(f'{m} {meth_} {time_}: reward {cap_} outside the rare slots')
                    for r_, w_ in wsum.items():
                        if w_ > 5.5: err(f'{m} {meth_} {time_}: reward {r_} at {w_:.0f}% (limit 5%)')
            if len(others) > 3: err(f'{m}: {len(others)} reward species besides the residents (limit 3): {sorted(others)}')
        n_rewards = len({reward_of(c) for meth in t if isinstance(t[meth], dict) for time in t[meth] for c in t[meth][time] if c in SP and reward_of(c)})
        water_only_road = meta['reach'] == 'Road' and set(meta['methods']) <= {'surf', 'fish'}
        if n_rewards == 0 and not water_only_road: warn(f'{m} ({meta["place"]}): no reward slot')
    # coverage, by national dex number
    need = {}
    for s, v in SP.items():
        if not v['nat'] or v['nat'] > cfg.get('cover_max_nat', 9999) or gen(s) not in cfg['cover_gens'] or s in LEGENDARY or s.split('_')[0] in LEGENDARY or s in ('HO_OH',): continue
        if re.search(r'_(ALOLA|GALAR|HISUI|PALDEA)', s): continue  # regional forms live in their own regions
        if s in PRED and gen(PRED[s][0]) and gen(PRED[s][0]) < gen(s) and PRED[s][0] not in BABIES: continue  # a newer stage of an older line
        if s in PRED and FORM_BASE.get(PRED[s][0]) and gen(FORM_BASE[PRED[s][0]]) == gen(s): continue  # evolves from a form of a listed species
        low = s
        if low not in PRED:  # a species whose evolutions are listed only on its forms, such as Florges_Red from Floette_Red
            forms = sorted(x for x in PRED if x.startswith(s + '_'))
            if forms: low = nat_line_base(forms[0])
        while low in PRED and gen(PRED[low][0]) == gen(s) and nat_of(PRED[low][0]): low = PRED[low][0]
        if low in cfg['fossils'] or s in cfg['fossils']: continue
        need[nat_of(low)] = low
    if region == 'Johto':  # Gen II species from Gen I lines need the line's base in Johto
        for s, v in SP.items():
            if v['nat'] and gen(s) == 2 and s in PRED and gen(PRED[s][0]) == 1 and s not in BABIES:
                need[SP[base(s)]['nat']] = base(s)
    missing = sorted(name for n, name in need.items() if n not in present_nat)
    missing += sorted(cfg.get('form_bases', set()) - present_sp)
    if missing: err(f'{region} coverage: not catchable: {", ".join(missing)}')
    for r, (sec, temp) in REWARDS.items():
        if cfg.get('reward_only') is not None and r not in cfg['reward_only']: continue
        if sec in cfg['reward_sections'] and temp != '—' and r not in rewards_seen and SP.get(r, {}).get('nat') not in present_nat:
            err(f'{region}: reward {r} never appears')
    for a in cfg['anchors']:
        if a not in anchor_hits: err(f'{region}: anchor {a[1]} missing from {a[0]}')
    for f, ms in limited.items():
        if len(ms) > cfg['limits'][f]: err(f'{region}: crossover {f} on {len(ms)} maps (limit {cfg["limits"][f]})')
    over = {f'{g}:{f}': len(ms) for (g, f), ms in lead.items() if len(ms) > (cfg['surf_first_cap'] if g == 'surf-first' else cfg.get('lead_cap', 8))}
    if over: err(f'{region}: families leading too many maps (land and trees/rocks: top two, limit 8; surfing: first slot, limit {cfg["surf_first_cap"]}): {over}')
    for b, (tot, g) in sorted(weight_by_band.items()):
        share = 100 * g / tot if tot else 0
        if b not in cfg['bands']: continue  # no share target, such as the Safari Zones
        lo, hi = cfg['bands'][b]
        what = 'blend' if 'blend_gens' in cfg else f'Gen {cfg["share_gen"]}'
        (warn if lo <= share <= hi else err)(f'{region} {b}: {what} share {share:.0f}% (target {lo}-{hi}%)')

if __name__ == '__main__':
    for path in sys.argv[1:] or [os.path.join(HERE, r.lower() + '.json') for r in CFG]:
        region = next(r for r in CFG if r.lower() in os.path.basename(path).lower())
        check(region, json.load(open(path)))
    for w_ in warnings: print('NOTE ', w_)
    for e in errors: print('ERROR', e)
    print(f'{len(errors)} errors, {len(warnings)} notes')
    sys.exit(1 if errors else 0)
