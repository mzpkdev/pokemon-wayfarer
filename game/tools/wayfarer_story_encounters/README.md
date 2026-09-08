# Ordinary trainer caller audit

`ordinary.json` is the explicit caller allowlist. Run `python3 generate.py --check`
from this directory, or `make wayfarer-story-encounters-audit` from `game/`.
Wayfarer ROM and mechanics builds run the audit before linking. Run the generator
without `--check` to render a reviewed manifest change.

The initial inventory contains 851 callers on 155 current maps: 688 base singles,
99 single rematches, 54 base doubles and 10 double rematches. The 787 single
callers opt into field loss return. Doubles retain their existing defeat routing.

Each base caller starts directly with a trainerbattle macro. There is no authored
pre-battle movement, object transformation, party substitution or chapter writer
to undo. On loss, the callback selects the common retreat script before either
the macro's victory continuation or the following script can resume. Phone and
Match Call registration, item offers and other post-victory branches therefore
remain unexecuted on defeat. Rematches are explicit branches reachable from these
ordinary base scripts and share the base trainer's frozen dialogue key.

The manifest records the exact macro and a fingerprint of its command block.
Adding a new script does not enroll it. Moving the macro or changing its block
fails the audit until that caller is reviewed again. A script label plus one
points to the trainerbattle argument block; no assembler labels are inserted into
maps by this tool.

Policy and dialogue are separate. Optional faction members retain ordinary
bypass and use an explicit hostile dialogue assignment. Base callers permit their
normal completed-trainer text; rematch callers still require a usable party.
Dialogue keys are frozen integers derived from the base trainer constant name;
they use no gameplay RNG or saved counter.

Gym-map callers are excluded from this manifest, including ordinary-looking Gym
members. Staged rivals, objective guards, tutorials, League battles and chains
belong to the regional reviewed registries or retain their existing routing.
The regional registries also own the optional S.S. Aqua sailor and other supported
scenes that require a guard before their authored staging. The runtime registry
validation rejects duplicate callers across all three inventories.
