# Concept Lab 3 — Same Problem, Twice: adding it to the student repository

Unzip this over the top of your `dungeon-delve-core` working copy, then commit and
push. Students get it with one `git pull`.

| | |
|:--|:--|
| `labs/week03_same_problem_twice/` | the lab |
| `CMakeLists.txt` | the root build file — replaces yours |

## What the students get

| File | Role |
|:--|:--|
| `spellbook.h` | the contract — DO NOT MODIFY |
| `lab_support.cpp` | `find_spell()`, shared by both designs |
| `same_problem_twice.cpp` | the driver, compiled against each design |
| `tests.cpp` | 90 supplied checks, run against each design |
| `impl_inherit/spells.cpp` | scaffold one — the inheritance design, 6 TODOs |
| `impl_compose/spells.cpp` | scaffold two — the composition design, 6 TODOs |
| `README.md` | what to do |
| `ANALYSIS.md` | the graded write-up they fill in and submit |

## Four targets, not one

One header, two implementations, so the driver and the tests are each built twice:

```
./build/course-debug/same_problem_twice_inherit
./build/course-debug/same_problem_twice_compose
./build/course-debug/same_problem_twice_inherit_tests
./build/course-debug/same_problem_twice_compose_tests
```

There is no `same_problem_twice` target. If any grading script of yours names one,
it needs updating.

## Why the root CMakeLists.txt is in here

Labs self-wire: each folder carries its own `CMakeLists.txt` and the root adds
whichever are present, so publishing a lab is a `git pull` with no edit to the root
file. It also pins the runtime output directory — without that, the lab binaries land
in `build/<preset>/labs/<name>/` and every path above breaks.

If you have already pushed this root file with the full lab bundle, it is unchanged
and overwriting it is a no-op.

## Verified before sending

Both scaffolds compile warning-free under `-Wall -Wextra -Wpedantic` (g++ 13), and
both test binaries fail 11 of 11 checks against the untouched scaffold — so the tests
can tell a finished lab from an unstarted one. Against the reference solution all 90
checks pass and the two drivers produce identical transcripts.

## One thing to tell students

The two drivers' output must `diff` clean, and that only holds if they register the
spells in the same order in both folders. The README says so twice, but a student who
skips it will see a non-empty diff and think their code is broken rather than their
ordering.

## The reference solutions are not in here

They are in `IS_267_ConceptLab_Solutions_INSTRUCTOR_ONLY.zip`. Keep that off the
student remote and out of Canvas Files.
