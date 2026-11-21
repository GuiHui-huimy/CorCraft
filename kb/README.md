# Local Knowledge Base

This directory is a working example of the method described in
`docs/meta/ai-assisted-workflow.md`. It is a plain folder of short, sourced
notes that can be searched before a question is handed to a model.

```
kb/
├── README.md          this file
├── index.md           one line per note: topic, source, date checked
├── c/                 one file per topic, facts copied from a primary source
├── cpp/
├── bio/
└── logs/              error output that cost more than ten minutes to fix
```

Three rules keep it useful:

1. Every note cites where the fact came from: a man page section, a standard
   clause number, a compiler version. A note without a source is a guess and
   gets deleted.
2. Every note records the date it was checked. Compiler behaviour changes; a
   note from three releases ago may describe a different default.
3. Every note is short enough to paste into a prompt. A note longer than a
   screen is a document, not a fact.

The knowledge base is not a substitute for reading the code. It exists so that
when a model is asked "why does this warning appear", the answer can be grounded
in a specific compiler version rather than in a general impression.
