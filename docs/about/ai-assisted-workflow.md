# How I Use AI to Build a Project

I did not start with a method. I started with a folder of notes and a model that kept answering a slightly different project than the one I was building.

The first architecture it produced for PepNat was tidy, plausible, and wrong in a way I could not see until the storage phase: it had invented a single "pipeline" object that owned the database, which made the store impossible to test without the generator, and the generator impossible to test without ESM-2 weights. I had asked for an architecture before I had written down what the database had to guarantee. What fixed it was not a better prompt. It was writing the requirement down first, and keeping the notes where the model could read them.

That is the whole of this page: five steps, each one paid for by a failure I can point at.

```text
1. Build a local knowledge base before the first prompt.
2. Write the requirement as tests, not as a wish.
3. Let the model draw the architecture, then redraw it yourself.
4. Build one phase at a time, interface before implementation.
5. Treat every test as the contract the next phase relies on.
```

## 1. Step 1: the knowledge base comes before the first prompt

A model answers from what it can see. If all it can see is one sentence — "help me build a peptide library generator" — it answers from the average of every project it has read, and none of those is mine.

For PepNat the knowledge base is an Obsidian vault, not a folder inside the repository:

```text
D:\工程文件\笔记仓库\Obsidian Vault\生物信息\生物信息学工具\多肽对接\多肽生成器\
├── 1-1-整体设计方案.md
├── 1-2 整体模块划分.md
├── 1-3 PepNat 构建路线.md
├── 1-4-1 Conda环境构建.md
├── 2-0 Phase 0 详细施工 + 全架构生成脚本.md
├── 2-1 Phase 1 — infra 层完整施工.md
├── 2-2 Phase 2 — M7 存储层完整施工.md
├── 2-3 Phase 3 — M1 来源层 + utils 补充.md
├── …
├── 2-12 Phase 12 测试.md
├── 2-13 Todo List.md
├── 2-14 顶层接口.md
├── 3-1 论文推进路线（P13–P20）.md
└── 3-2 文档架构.md
```

Twenty-eight notes, written before the first line of `src/`. They are not a diary; they are the specification, one phase per file, in the order the phases were built. When I ask the model to implement M7, I paste `2-2` and `1-2`, and it answers about my store, my tables and my idempotency rule rather than about a generic SQLite wrapper.

Three rules made the folder usable, and I learned all three by breaking them:

- **A note states its source.** "PDB column 22 is the chain ID" is a fact I can check against the format specification. "The parser reads the chain" is a memory, and memories drift silently.
- **A note is short enough to paste.** The Phase 2 note is four pages, not forty. Anything longer gets truncated, and the truncated part is usually the constraint that mattered.
- **A decision records what was rejected.** `1-3 PepNat 构建路线.md` says not only that the store is SQLite, but that a single JSON file was rejected because a 3.2-million-row enumeration cannot be rewritten atomically.

The folder also taught me that naming is part of the contract. Two files ended up as `2-8`: `2-8 FIX BUGS.md` and `2-8 Phase 8 — M4 贝叶斯优化 完整施工.md`. I can still tell them apart by their suffixes, but when I pasted "the 2-8 note" into a prompt, the model read the bug list and answered about the Bayesian phase. I now number every note into a slot that nothing else uses, because a filename is an address.

**The failure this prevents**: an architecture invented from a one-line request, and code written to fit it. That is what happened before `1-1` existed.

## 2. Step 2: write the requirement as tests, not as a wish

"Write me a peptide generator" is a request. What I actually needed for PepNat was narrower and much harder:

```text
A peptide store that:
- ignores a repeated (sequence, length, line) instead of duplicating it
- survives kill -9 mid-run: after reopen, PRAGMA integrity_check = ok
  and no duplicate rows, and the run resumes from the last committed batch
- keeps one SQLite file per generation line, so B, C, D and A can be
  compared, rebuilt and merged without touching each other
- writes <store>.manifest.json with pepnat version, schema version,
  threshold version, model version, run id and row counts
- derives every random choice from a global seed plus per-line sub-seeds,
  so the same command produces the same library
- installs with pip install -e . and downloads no checkpoint at install time
```

Every clause became either a test or a build check. The idempotency clause became `UNIQUE(sequence, length, line)` plus a test that inserts the same row twice and counts one. The kill -9 clause became a test that opens a store, kills the process, reopens it and continues. The manifest clause became a test that reads the file and compares it to the database.

The difference between the request and the requirement is that the requirement can fail. "A peptide generator" cannot fail; it can only disappoint. A clause about `integrity_check` after `kill -9` fails on the first implementation that writes outside a transaction, and it fails in a way I can read.

**The failure this prevents**: a correct-looking implementation that duplicates rows on a re-run, and a review that cannot say so, because nothing ever said a re-run was allowed.

## 3. Step 3: let the model draw the map, then redraw it yourself

Before code, I ask for the map: the module list with one sentence of responsibility each, the data flow, the dependency order, and the interface and invariant of every module.

```text
infra        config, logging, seeds, paths            tested with a temp directory
M7 store     SQLite: peptide / structure / run tables tested with :memory:
M1 sources   sequence space, naturalness files        tested with string literals
M2 esm       ESM-2 embeddings, cached                 tested with a stubbed model
M3 threshold fixed-threshold verdict                  tested with labelled rows
M6 schedule  generation loop and the first CLI verbs  tested with a fake scorer
M5 seed      neighbourhood sampling                   tested with a small alphabet
M4 bayes     Bayesian optimisation over parameters     tested with a toy objective
M8 analyse   read-only reports                        tested against a fixture store
```

That order is PepNat's actual build order, and it is the reason Phase 2 is the store and Phase 3 is the source layer. It is also the part of the workflow where the model is most useful and most dangerous at once: it can hold twenty modules in view and produce a dependency chain in seconds, and it can produce one that is plausible in the abstract and impossible in my build.

My review rule is one line: a module must be testable while the modules below it do not exist. The first architecture failed that rule. It gave me a `Pipeline` object that owned the database and the scorer, which meant the store could only be tested by constructing a pipeline, and a pipeline needed the model weights. I deleted the object. The store became a constructor argument with a `:memory:` default, and after that every storage test ran in milliseconds.

**The failure this prevents**: a monolith where nothing can be tested until everything exists, and therefore nothing is tested.

## 4. Step 4: one phase at a time, interface before implementation

A phase is done when its test passes, not when its file exists. The sequence I use for each one:

1. Ask for the interface only: the signatures, the invariants, the error model. No implementation.
2. Review the interface against the requirement. This is the cheap moment to change it.
3. Ask for the implementation.
4. Ask for the test from the requirement clause alone, with the implementation out of view.
5. Run it: `pytest`, the build with warnings on, and the store's integrity check.
6. Write the decision note: what was chosen, what was rejected.

PepNat's Phase 11 shows the sequence in its filenames: `2-11-1 P11 esm.py 完整增量版.md`, then `2-11-2 P11验证.md`, then `2-11-3 P11 收尾与验证结论.md`. The interface, the verification, and the conclusion are three files because they were three separate requests. Merging them into one prompt is how a phase ends up "done" with no evidence.

Step 4 is the one I used to skip. An AI asked to write both the implementation and the test writes a test that confirms what it just wrote — I have watched it assert the value its own code produced, which is not a test but a mirror. A test written from the clause, with the implementation hidden, can fail, and the first one usually does.

**The failure this prevents**: three thousand lines with no test, and a two-day debugging session on a defect a four-line test would have caught.

## 5. Step 5: the test is the contract the next phase relies on

A test is not proof. It is a fixed sentence about what a module promises, written where the next module can read it. For PepNat those sentences are in `pytest`, and they state three things:

```python
# idempotency: the promise M6 relies on when it resumes a run
insert(store, seq, length=5, line="C")
insert(store, seq, length=5, line="C")
assert count(store) == 1

# resumability: the promise the CLI relies on after kill -9
store = open_store(tmp_path / "lib.db")
write_batches(store, rows, batch=1000)
kill_after(store, 1000)
store = open_store(tmp_path / "lib.db")
assert store.last_committed == 1000

# read-only analysis: the promise `pepnat analyze` makes to its caller
before = (row_count(store), mtime(store))
run_analyze(store, tmp_path / "report")
assert (row_count(store), mtime(store)) == before
```

Three properties make a sentence a contract: it runs in isolation (a temporary SQLite file, no network), it fails loudly (assertion, non-zero exit), and it states its tolerance where a number is compared. The C chapters of this book do the same thing with `CRAFT_ASSERT`; the language differs, the obligation does not.

**The failure this prevents**: a module that "works" in the demo and cannot be relied on by the module above it, because nobody wrote down what it guaranteed.

## 6. What the workflow does not do

It does not make the model responsible for correctness. The five steps move the correctness question to four places I can check: the requirement (readable), the interface (small), the test (a few lines), the tolerance or invariant (stated). The model's output is checked against those, never against whether it sounds right.

It also does not remove the need to read the code. Every Field Note in this book is a defect that looked plausible. A plausible defect is caught by someone who knows what the code should do, and the workflow's only job is to keep that attention on the few places where it matters.

## 7. PepNat, as it actually went

PepNat — Themis-PepNat, a modularised SQLite-backed peptide library and structure pipeline — is the project where this workflow was built, and it is a better example than this repository because it is larger and its failures were more expensive.

The notes came first: `1-1` fixed the design, `1-2` split it into M1–M8 plus `infra`, `1-3` chose the build route, and `2-0` generated the tree from a script so that no directory was created by hand. Then one phase per file: infra, store, sources, ESM and thresholds, schedule and CLI, the classifier and its threshold calibration, seed neighbourhood, Bayesian optimisation, analysis. Phase 11 added the Parquet export and the bulk path, and Phase 12 wrote the test suite.

The result I care about is not the feature list. It is that a 3.2-million-row enumeration was killed with `kill -9` mid-run, reopened, and came back with `PRAGMA integrity_check = ok`, zero duplicate rows and a resumable store. That is the idempotency clause from Step 2, tested rather than hoped for. The second result is the manifest: a library built on one machine can be handed to a docking run on another, and the file says which PepNat version, schema, threshold version and model version produced it.

What still went wrong is worth writing down too. Two notes collided on `2-8`, and a prompt that said "the 2-8 note" got the bug list instead of the Bayesian phase. The classifier's threshold version has to travel with the manifest, or an old library cannot be explained by a new threshold, and I learned that after the first library had already been exported. Neither failure was the model's fault. Both were places where I had not written the contract down.

## Continue

- [Projects](projects.md) is what came out of this workflow.
- [Roadmap](roadmap.md) is what remains.
- [How to Use This Book](../start-here/how-to-use.md) is the lab protocol that this workflow feeds.
