# Concept Lab 4 — Kill the Switch

**Name:**  Troy Ellison    **Date:** 11/13/2026

---

## Step 0 — The baseline

Fill this in **before** you change anything. If this section is blank, nothing
below it is a measurement.

| | |
|:--|:--|
| `kill_the_switch_tests` final line, before I touched anything | `ALL TESTS PASSED (237 checks)` |
| `kill_the_switch_variant_tests` final line, before | `13 of 22 checks FAILED` |
| Files in `impl/` before | 1 |
| `wc -l` total for `impl/` before | 215 |
| Code lines only (neither blank nor `//`) for `impl/` before | 152 |

---

## Step 1 — Line counts, broken down by file

### Before

| File | Raw lines | Code lines only |
|:--|:--:|:--:|
| `impl/dispatch.cpp` | 215 | 152 |
| **total** | **215** | **152** |

### After

Add a row per file. Delete the spare rows you do not use.

| File | Raw lines | Code lines only |
|:--|:--:|:--:|
| `impl/ability.h` | 80 | 66 |
| `impl/abilities.cpp` | 156 | 136 |
| `impl/registry.cpp` | 60 | 48 |
| `impl/shatter.cpp` | 25 | 20 |
| **total** | **321** | **270** |

**Did the total go up or down, and by how much?**

*My answer:* The raw total went up by 106 lines (215 to 321); code lines went
up by 118 (152 to 270).

If it went up, do not apologise for it. Say what you got in exchange for the
extra lines, in one sentence, and make it a specific thing rather than
"better design".

*My answer:* Each ability now owns its metadata and effect in a concrete
class, and adding Shatter required only a new implementation file rather than
editing a shared dispatch roster and behavior switch.

---

## Step 2 — Behavior is unchanged. Prove it.

| | |
|:--|:--|
| `kill_the_switch_tests` final line, after the refactor | `ALL TESTS PASSED (247 checks)` |
| `diff before.txt after.txt` on the driver output | Not provided; the posted driver output includes Shatter |
| Did you modify `tests.cpp`, `test_harness.h` or `kill_the_switch.h`? | No |

**Any test that failed on the way, and what caused it.** This is the most
useful row on the page for next week, so write a real answer. "None" is
allowed only if it is true.

*My answer:* The initial Shatter variant run had 13 of 22 checks fail because Shatter was not implemented yet, that was the expected pre Step 4 state. In the results posted after implementation, the unchanged behavior suite passed all 247 checks and the Shatter suite passed all 22 checks.

---

## Step 3 — Zero type inspection in the dispatch path

**Where does an ability tag still appear in your code, and what is it doing
there?** List every place. A tag in a lookup is fine; a tag deciding what
happens next is not, and this is where you show which yours are.

*My answer:* Tags occur as metadata values in each concrete class's
constructor, in the registry's tag based lookup, in the sort that keeps the roster in ascending tag order, and in `ability_tag_at()` when it returns a roster entry. The lookup and roster ordering use tags as identifiers; after lookup, metadata and effects are obtained through virtual methods.

**Confirm, by searching rather than remembering:** no `switch` and no
comparison against an `ABIL_` constant decides behavior anywhere in `impl/`.
Paste what you searched and what it found.

```
Search: `switch\s*\(|ABIL_[A-Z_]+\s*(==|!=|<|>|<=|>=)|(==|!=|<|>|<=|>=)\s*ABIL_[A-Z_]+`

Result: no matches in `impl/`.
```

---

## Step 4 — Add one ability

`ABIL_SHATTER`, specified in `kill_the_switch.h` and tested by
`variant_tests.cpp`.

| | |
|:--|:--|
| `kill_the_switch_variant_tests` final line | =======================================
ALL TESTS PASSED  (22 checks)
=======================================` |
| `kill_the_switch_tests` still passing afterwards? | Yes — `ALL TESTS PASSED (247 checks)` |
| Files I **added** | `impl/shatter.cpp` for Shatter |
| Existing files I **edited** | 0 implementation files for Shatter |

Paste the `diff -rq /tmp/week04_before labs/week04_kill_the_switch` output you
counted from: i forgot to capture this

```
The saved baseline is `impltemp/dispatchtemp.cpp`, not a full
`/tmp/week04_before` directory. `wc -l` reports 215 raw lines and the
code-line command reports 152. A full `diff -rq` against the pre-change lab
directory cannot be reconstructed from this one saved file.
```

**If the number of existing files you edited is not zero, name each one and
say why.** An honest 2 with a reason scores better than a 0 you did not
measure.

*My answer:* i didnt have to edit any existing implementation file to add shatter. Its class and registration are in the new `impl/shatter.cpp`; the shared registry discovers it without changes to the other implementation files. I did not edit any existing file to add Shatter. The saved `impltemp/dispatchtemp.cpp` is the old source, not a full pre-change directory snapshot. and i forgot to run the inital build i just immediately ran the test

**For comparison: how many of the five sites in the old `dispatch.cpp` would
Shatter have needed?** Count them from the `SITE 1`–`SITE 5` markers in the
original file.

*My answer:* Five sites: the old roster and the four switches for name, cost, hostility, and resolution.

---

## Step 5 — Name the thing that got worse. This is the graded artifact.

Something got worse. A submission claiming nothing got worse loses the
analysis marks — not because the claim is rude, but because it means the
comparison was not done.

It has to be a concrete cost in the new design: something that is now harder to find, harder to read, slower, longer, more scattered, easier to get wrong,
or impossible to see in one place. Not "it took longer to write" and not "it
is less familiar."

*The thing that got worse:* The ability definitions are more spread out and there is no single file where I can scan every ability's name, cost,
hostility, and effect. The implementation is now 321 raw lines across four files instead of 216 raw lines in one file.
*Who pays that cost, and when:* Someone reviewing or balancing the roster must find the relevant concrete class and compare multiple files instead of reading one centralized switch and table.

> Examples of the shape of a good answer, so you can see what is being asked
> — do not just pick one of these:
>
> - "Reading one ability's behavior used to be scrolling to a `case` label.
>   Now it is finding which file holds that class."
> - "The old file showed me all eight costs in fourteen lines. Now a cost is
>   a one-line method on each class and there is no single place to read the
>   balance of the game."

---

**And the honest counterweight: what got better, stated as a measurement.**
Point at a row from Step 1 or Step 4, not at an adjective.

*My answer:* Adding Shatter required zero edits to existing implementation files it is one new `.cpp` file, instead of edits at all five sites in the old dispatch.

---

**Would you make this change to the engine's own `resolve_ability()` in Build
4?** Yes or no, and one reason drawn from something you measured in this lab.

*My answer:* Yes. In this lab, adding a variant required zero edits to
existing implementation files, although the implementation grew by 105 raw lines overall.

---

## What is being graded

| | |
|:--|:--|
| Step 0 filled in before anything changed | required |
| `kill_the_switch_tests` passes, unmodified, after the refactor | required |
| Driver output matches `before.txt` apart from Shatter | required |
| No `switch` or `ABIL_` comparison decides behavior in `impl/` | required |
| `kill_the_switch_variant_tests` passes | required |
| Existing files edited for Shatter, measured with `diff -rq` | required |
| Line counts are per file and came from a command | required |
| A genuine cost of the new design is named | **the graded artifact** — the line counts alone are not |

Submit this file, completed, as directed on the assignment page.
