# Concept Lab 3 — Same Problem, Twice

**Name:**  ___________________    **Date:** ____________

---

## Step 1 — Get both versions passing

Fill this in before you measure anything. A measurement taken on a version
that does not pass is a measurement of an unfinished program.

| | Output |
|:--|:--|
| `./build/course-debug/same_problem_twice_inherit_tests` final line | `ALL TESTS PASSED (90 checks)` |
| `./build/course-debug/same_problem_twice_compose_tests` final line | `ALL TESTS PASSED (90 checks)` |
| `diff a.txt b.txt` of the two drivers | empty |

If the `diff` was not empty, say what differed and what you changed:

*My answer:* Not applicable; the driver output diff was empty.

---

## Step 2 — The measurement table

Run the commands in `README.md`. Paste the numbers they printed. Do not
estimate anything in this table; if you could not measure something, write
"could not measure" and say why in Step 3.

### Line count

| | Version A — inheritance | Version B — composition |
|:--|:--:|:--:|
| Files in the folder | 1 | 1 |
| Raw lines (`wc -l`, total) | could not measure | could not measure |
| Code lines only (neither blank nor `//`) | 150 | 113 |

### Files touched to add Hex

| | Version A — inheritance | Version B — composition |
|:--|:--:|:--:|
| Existing files I **edited** | 1 (`spells.cpp`) | 1 (`spells.cpp`) |
| New files I **added** | 0 | 0 |
| Separate places inside those files I had to change | 2 | 2 |

> That last row is the one people skip. Adding a variant usually means adding
> the thing *and* adding a line to a table that lists the things. Two edits in
> one file is not the same cost as one.

Paste the `diff -rq` output you counted from:

```
N/A
```

### Depth of the structure

| | Version A — inheritance | Version B — composition |
|:--|:--:|:--:|
| Number reported | 2 | 1 |
| What I counted to get it | The base `Spell` class and one derived spell class | One function-pointer indirection from the `Spell` entry to its effect |

### Where the shared mana rule ended up

The "not enough mana, so nothing happens" rule applies to every spell. Say,
for each version, the **one** place it is written:

| | Where the mana rule lives |
|:--|:--|
| Version A — inheritance | Once, in the base class's `Spell::cast()` method |
| Version B — composition | Once, in `cast_spell()` |

If you had to write it more than once in either version, say how many times
and why. Writing it three times and not noticing is the failure mode this row
exists to catch.

---

## Step 3 — What the numbers do not show

Two or three sentences. Name one thing that was harder or easier to write
that none of the numbers above captured. "Version B was shorter" is already
in the table; this section is for what the table missed.

*My answer:* Composition uses fewer code lines (113 versus 150), while adding Hex required editing one existing file and two places in each design. The inheritance version makes each spell a distinct type and puts the shared casting rule on the base class, while composition keeps the variable effect in a function pointer and the shared rule in the dispatcher. I did not capture the raw line totals or exact `diff -rq` output, and the driver transcript didnt proc Hex because only 2 mana remain when it is reached.

---

## Step 4 — The recommendation. This is the graded artifact.

**Which one would you ship as the ability system for Build 4?**

*I would ship:*  [ ] Version A, inheritance   [x] Version B, composition

**Why — in terms of the numbers above.** Name at least one row of the
measurement table and say what it told you. An answer that does not refer to
anything you measured earns nothing, however reasonable it sounds.

*My answer:* I would ship composition: its measured code-only count is 113 lines versus 150 for inheritance, with the same one file/two place cost to add Hex. Its single function-pointer indirection is also shallower than the inheritance chain's two classes.

---

**Name something the design you rejected does better.** There is always
something. A submission that claims the rejected design does nothing better
loses these marks — not because the claim is rude, but because it means you
compared two things and only looked at one of them.

It must be something concrete that the rejected design makes easier, cheaper,
or harder to get wrong. Not "it is more familiar" and not "some people prefer
it."

*My answer:* Inheritance makes the spell interface explicit: a concrete spell cannot be instantiated unless it implements the base class's pure virtual effect, name, and mana-cost functions. That makes missing spell behavior harder to overlook than a composition table whose function pointer could be left unset.

---

**One change that would flip your recommendation.** Finish this sentence with
a specific change to the requirements, not a change of mood:

> I would ship the other design instead if the requirements changed so that
> ...

*My answer:* I would ship inheritance instead if each ability needed a larger set of required, spell specific operations that the compiler should enforce through a common abstract interface.

---

## What is being graded

| | |
|:--|:--|
| Both versions pass the supplied tests, unchanged | required |
| The two drivers produce identical output | required |
| Every number in Step 2 came from a command, not an estimate | required |
| The `diff -rq` output is pasted in | required |
| The recommendation cites a row of the measurement table | required |
| Something the rejected design does better is named, concretely | required |
| Steps 3 and 4 together | **the graded artifact** — the table alone is not |

Submit this file, completed, as directed on the assignment page.
