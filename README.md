# MontageForge

Turns an animation sequence into a playable montage: creates the asset, puts it in a slot, places
gameplay-event notifies at authored times, stamps the layering curves that let it play over
locomotion, and sets its blends.

```
animation  +  recipe  →  montage  →  you assign it to whatever plays it
```

**Status: 0.2 — prototype quality, and proven.** Built and verified in a live editor: a recipe produces a
montage with notifies at authored positions, layering curves stamped, and blend settings applied; the
result plays in game, drives an ability, and layers over locomotion. Capture reads timings back
correctly.

Prototype quality means exactly that. It produces a working montage from a generated clip in seconds,
which is the point — but the *authoring* is still crude: timings are guessed then nudged, body masks
are copied from an animation that already worked, and none of it substitutes for an animator deciding
what a clip should actually do.

---

## What this is not

It knows nothing about your game. It does not decide what a tag means, which curves constitute a body
mask, or which slot your anim blueprint samples — and it does **not** assign the finished montage to
an ability, an item or a dialogue line. That step needs to know what those are, and coupling a
reusable tool to one framework to save a single drag is a bad trade.

Assign it yourself, by hand or with a prompt through the generic property tools. It is one action per
clip; building the montage was the expensive part.

The one thing here that is about *playing* rather than building is `EMontagePlayStyle` — whether the
character can move while a montage runs. It lives in the runtime module because blocking movement
input during an animation is an engine-level idea with no game knowledge in it, so anything that plays
a montage can use it. See [PLAYBACK_PLAN.md](../MotionForge/PLAYBACK_PLAN.md).

---

## Why the notify exists

GameplayAbilities ships `UAnimNotify_GameplayCue`, which fires cosmetic **cues**. Nothing in the
engine or in most frameworks fires a gameplay **event** from an animation — so an ability waiting on
one has no stock way of being told the moment arrived, and every project ends up writing this class.

`UMontageForgeGameplayEventNotify` is that class. It carries a `FGameplayTag`, and at its point in
the montage it sends that tag to the animating actor. That single thing is what lets an ability be
driven by animation rather than by a timer.

It is the only reason this plugin depends on **GameplayAbilities**. Everything else is plain UE.

---

## The recipe is the source of truth

The working habit this is built around:

> build once → nudge the timing by hand → ship it

So a built montage is **expected** to drift from its recipe, and that is fine. Three consequences:

- **Nothing rebuilds automatically.** No build-on-save, no watching, no keeping "in sync". A rebuild
  is a deliberate act, because it discards the hand-tuning that is the only work here which is not
  reproducible.
- **Rebuilding an existing montage is refused** unless you explicitly overwrite.
- **Capture is optional.** For the clips worth making reproducible, it reads the montage's current
  notify times back into the recipe so a later rebuild starts from what you decided.

Timings are stored as a **fraction of the clip**, not seconds. The animation underneath will be
regenerated and come back a different length; "sixty percent through" survives that, "at 2.4
seconds" quietly stops meaning what it meant.

### Rebuilding keeps references

A rebuild edits the existing montage in place rather than deleting and recreating it, so everything
pointing at it — abilities, items, dialogue lines — keeps working. The difference between a rebuild
and a rewire.

---

## Use

Create a **Montage Recipe** asset:

| Field | |
|---|---|
| `Sequence` | The animation to play. Usually something MotionForge imported. |
| `SlotName` | `DefaultSlot` for full-body. A layered slot for anything that should play while walking — see below. |
| `BlendInTime` / `BlendOutTime` / `BlendOption` | How it eases in and out. |
| `CurvePreset` / `ExtraCurves` | The body mask, as layering curves. |
| `Notifies` | A tag and a position (0–1) per event. |
| `OutputPath` | Where the montage lands. Empty builds it beside the recipe. |
| `BuiltMontage` | Filled in by the builder, so a recipe can be traced to its result. |

Name it `MR_Something`; the montage comes out as `AM_Something`.

Then build it — through the [MontageForgeToolset](../MontageForgeToolset/README.md) plugin from an
agent, or from Blueprint/Python via `UMontageForgeSubsystem`.

### Tags

Whatever the thing playing the montage waits on. In this project that is Narrative Pro's
`GameplayEvent.Consumable.Apply`, declared natively in `NP_Consumables`. Match it exactly — a typo
fails silently, because an event nobody listens for looks identical to one that was never sent.

---

## Layered playback, and why a montage can play and animate nothing

A montage on a full-body slot takes the whole skeleton, so the walk cycle stops and the character
slides. The alternative is a **layered slot** — arms and torso from the montage, legs from locomotion.

The trap: in a GASP-style anim blueprint the layered slots' blend weights are driven by **curves read
off the playing animation**. A montage with no curves evaluates every layered slot at weight zero. It
plays, its notifies fire, and it animates *nothing at all* — no error, no warning, and every
mechanical check passes. That failure cost a session to diagnose, which is why it is the first thing
written down here.

So a layered montage needs two things: the slot name, and the curves.

### Curve presets

Which curves mean "upper body" is a property of the rig, not of this plugin. Narrative Pro's biped
uses `LayerArmLeft`, `LayerArmRight`, `LayerHandLeft`, `LayerHandRight`, `LayerArmLeftLocalSpace`,
`LayerArmRightLocalSpace`, `LayerSpine`, `LayerHead`, `LayerPelvis`, `LayerLegs` and additive
variants. Stock Game Animation Sample names them differently again.

So the mask lives in a **`Montage Curve Preset`** asset you author, and recipes point at it. Adding a
new mask is a new asset, not a code change. `ExtraCurves` on the recipe override the preset per name,
for one-off adjustments without copying the whole set.

**Copy the values from a montage that already works**, rather than reasoning about them. Spelling is
unforgiving and failure is silent — a misnamed curve is indistinguishable from one never authored.

**Include the zeros.** Leaving a curve out is not the same as setting it to zero: an absent curve
leaves whatever the anim blueprint was already doing in place, which is rarely what a mask means.

### The mask is per clip, not per project

It expresses what the animation is *for*. A talking gesture wants arms and hands but **not** the
spine, so the torso keeps following locomotion and the character does not freeze from the waist up. A
clip where the character bends to reach something wants the spine too, or the lean never happens.

`LocalSpace` chooses whether an arm layer blends in local or mesh space: local means the arms travel
with the torso, which is right for gestures; mesh space holds them steady in the world while the body
moves under them, which is right for aiming.

## Scripting

Everything is on `UMontageForgeSubsystem`, an editor subsystem, so it is reachable from C++,
Blueprint, Python and MCP.

| Call | |
|---|---|
| `BuildFromRecipe` | Build one. Takes an overwrite flag. |
| `BuildMany` | Build several; each is independent, one failure does not stop the rest. |
| `CaptureTimingsFromMontage` | Read hand-tuned times back into a recipe. |
| `FindRecipes` | Every recipe in the project. |

`FMontageBuildResult` reports `NotifiesPlaced` and `CurvesStamped`. A layered montage reporting zero
curves is the silent-failure case above, and worth treating as an error even though the build
succeeded.

Filter the Output Log on **`LogMontageForge`**.

### What capture does and does not do

Notifies are matched to recipe entries **by tag**. Ones added by hand in the montage editor are
picked up. Entries whose tag no longer appears in the montage are **left alone, not deleted** — a
notify removed on purpose and one that was never placed look identical from here, and silently
discarding authored intent is the worse mistake.

---

## Notes

**Rebuilt montages keep their original frame rate.** `UpdateCommonTargetFrameRate` is private to
`UAnimMontage` and reachable only from the factory, so an in-place rebuild cannot refresh it.
Harmless while replacement clips come from the same pipeline at the same rate; if that stops being
true, delete and rebuild instead, at the cost of breaking references.

**Blends default to `Linear`, not the factory's `HermiteCubic`.** Of 60 Narrative Pro montages
sampled, 48 use Linear — so a generated clip on the factory default blends unlike every hand-authored
montage around it, which reads as wrong without anyone being able to say why. Blend times default to
0.25, also the most common value. 14 of those 60 use a blend-in of **0**, a hard snap, which is worth
considering for anything sharp.

**Single-section montages only.** No sections, no blending between clips. Add them when something
actually needs them.

**Curves and notifies are stamped, not merged.** A rebuild replaces both wholesale from the recipe.
Anything added by hand to a montage is lost on rebuild unless it was captured first — which is the
deliberate bargain described above.

---

## Layout

```
MontageForge.uplugin
Source/MontageForge/                     runtime - what a packaged game needs
  MontageForgeGameplayEventNotify.*      sends a gameplay tag to the animating actor
  MontagePlayStyle.*                     Free vs BlockMovement, and the input-blocking helper
Source/MontageForgeEditor/               editor - everything that builds
  MontageRecipe.*                        the authored intent, and the source of truth
  MontageCurvePreset.*                   a body mask, as the curves that produce it
  MontageForgeSubsystem.*                build, rebuild, capture
```
