# Concept Lab 2 — Prediction Table

**Name:**  ___________________    **Date:** ____________

---

## Step 1 — Predict. Do this BEFORE you compile.

Read `prediction_table.cpp`. For each of the six calls, write what you think
will happen. You will be wrong on at least one of these. That is the point of
the lab; being wrong here costs nothing and teaches more than being right.

For each row: how many **copies** and how many **moves** happen between the
call and its return, and what the function sees.

| # | The call | Copies | Moves | What the function receives |
|:--|:--|:--:|:--:|:--|
| 1 | `by_value(original)` | 1 | 0 | A copy named `hero-copy` |
| 2 | `by_const_ref(original)` | 0 | 0 | A const reference to `hero` |
| 3 | `by_ref(original)` | 0 | 0 | A mutable reference to `hero`; it changes the name to `hero!` |
| 4 | `by_pointer(&original)` | 0 | 0 | A pointer to `hero` |
| 5 | `by_value(make_probe("temp"))` | 0 | 0 | The temporary `temp`, with no extra Probe copy or move |
| 6 | `Holder h{original}` | 1 | 1 | The parameter is a copy named `hero-copy`, then it is moved into `held` |

**Predicted totals:** copies 2   moves 1

> Once you compile, you cannot un-see the answer. Fill the table in first.

---

## Step 2 — Run it.

```bash
cmake --preset course-debug
cmake --build --preset course-debug
./build/course-debug/prediction_table
```

Record the actual numbers:

| # | Actual copies | Actual moves | Matched my prediction? |
|:--|:--:|:--:|:--:|
| 1 | 1 | 0 | Yes |
| 2 | 0 | 0 | Yes |
| 3 | 0 | 0 | Yes |
| 4 | 0 | 0 | Yes |
| 5 | 0 | 0 | Yes |
| 6 | 1 | 1 | Yes |

**Actual totals:** copies 2   moves 1

---

## Step 3 — Reconcile. This is the graded artifact.

**For every row you got wrong, write two sentences:** what you expected, and
what actually happened. Not "I was wrong about 5" — say what you believed about
how arguments are passed that turned out not to be true.

Write a reconciliation for every incorrect row, not only the ones that
surprised you. A row you got wrong and shrugged at is the one that will cost
you in Build 2.

No incorrect rows all six predictions matched the output, so no reconciliation is needed.

---

## Step 4 — The closing question

**Which one of the six calls would you change, and to what?**

Your answer must name a **cost**, not a preference. "I'd use `const&` because it's cleaner" earns nothing. "I'd change call 1 to `const&` because by value costs a full string copy on every call and the function never modifies it" earns the marks.

*My answer:* I would change call 1 to `by_const_ref(original)`. Passing the
`Probe` by value makes a copy of the object, including its `std::string`, on every call, while `by_value` only reads the name; a const reference avoids that copy.

---

## What is being graded

| | |
|:--|:--|
| All six predictions recorded **before** the first compile | required |
| Reconciliation written for **every** incorrect row | required |
| The closing recommendation names a cost, not a preference | required |
| The reconciliation write-up | **the graded artifact** — the table alone is not |

Submit this file, completed, as directed on the assignment page.
