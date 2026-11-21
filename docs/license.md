# License

Two licences apply to this repository, and the split is deliberate: the prose is the part I want to protect as a book, the code is the part I want people to take. This page states where the boundary sits, how to attribute, and which files I did not write.

The short version: **the text and images are CC BY-NC-SA 4.0, the code is MIT, and the names and logo are reserved.** If you only need one sentence, that is it. The rest of this page is the reasoning and the details a careful reader will want before reusing something.

## 1. What each licence covers

| Path or content | Licence |
| --- | --- |
| `docs/**/*.md` — every chapter, exercise and note | CC BY-NC-SA 4.0 |
| `docs/assets/images/*` — diagrams and the logo as an image | CC BY-NC-SA 4.0, except the logo as a mark |
| Code blocks inside a page | MIT |
| `code/`, `labs/`, `tools/` | MIT |
| `CMakeLists.txt`, `mkdocs.yml`, `.gitignore` | MIT |
| `docs/material/templates`, `plugins`, `extensions` | MIT, from MkDocs Material |
| "C/C++ Craft", "CorCraft", the logo, the site identity | Not licensed; reserved |

The reason a code block inside a chapter is MIT while the paragraph around it is CC is practical. Readers copy snippets into their own projects, and a snippet that arrives under a non-commercial copyleft licence is a snippet most people cannot use. The paragraph explaining why the snippet works is a different thing: it is the part of the book I wrote, and it is what I do not want lifted wholesale into a paid course.

## 2. CC BY-NC-SA 4.0 in plain terms

You may copy, translate, adapt and redistribute the text and the diagrams, provided you meet four conditions:

1. **Attribution** — name the author, cite the book and the page, link to the licence, and say whether you changed anything.
2. **NonCommercial** — you may not use the material for a commercial purpose. A paid course, a paid membership, a book sold on a platform, or a content site whose revenue depends on this material is commercial. Reading it, teaching it in a tuition-free class, quoting it in a paper, or translating it for a study group is not.
3. **ShareAlike** — if you adapt the text, your adapted text must carry the same licence. You cannot take a chapter, rewrite it lightly, and put it behind a closed licence.
4. **No additional restrictions** — you may not add legal or technical terms that forbid others from doing what the licence allows.

The full legal text is at <https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode>, and the Chinese version at <https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode.zh-hans>. If this page and the legal text disagree, the legal text governs.

## 3. MIT in plain terms

You may use the code for anything, including commercial work, closed-source work and work you never publish. You must keep the copyright notice and the permission notice with any substantial portion you redistribute. There is no warranty: a lab that corrupts your heap is your heap.

This is the licence most readers want for code, and it is the one I would want if I were copying an example out of someone else's book.

## 4. How to attribute

For text or a diagram, a line of this shape is enough:

> Su, Guohui. *C/C++ Craft: Memory-First C and C++*. 2026. https://guihui-huimy.github.io/CorCraft/. CC BY-NC-SA 4.0. Adapted from the original.

For code, keep the notice that ships in the files, or add:

> Adapted from C/C++ Craft (MIT), Copyright (c) 2026 HUIMY, https://github.com/GuiHui-huimy/CorCraft.

Quoting a paragraph in a review, a lecture slide or a paper is ordinary quotation and does not require you to licence the whole of your work under CC. Reproducing a chapter is what the licence governs.

## 5. Commercial use, and how to ask

If you want to use the text in something commercial — a paid course, a training programme inside a company, a printed book, a paid newsletter — write to `suguohui_account@163.com` with the material you want, the audience, the channel and the duration. Silence is not permission. I answer most requests within a week, and for small uses the answer is usually yes with an attribution line.

The code does not need this conversation. MIT already permits commercial use, and asking would waste both our time.

## 6. Third-party components

The site is built with MkDocs and MkDocs Material, both MIT, and their files live under `docs/material/templates`, `docs/material/plugins` and `docs/material/extensions`. The theme is a build-time layer, so it does not change the licence of the chapters or the examples.

Coordinates mentioned in the biology chapters come from entries in the Protein Data Bank, under the wwPDB terms of use. The repository ships no coordinate files; `data/README.md` lists the accessions and the commands that fetch them. External links belong to their own owners.

## 7. What a licence cannot do

A licence governs copying, adaptation and distribution. It does not protect an idea, a method, a fact or a teaching sequence as such. Someone who reads the book and writes their own explanation of the same material is not infringing, even if the structure feels similar. What the licence does protect is the expression: the sentences, the diagrams, the selection and arrangement of the examples, and the code.

It also cannot be withdrawn. A version published under CC BY-NC-SA 4.0 stays available under those terms; a later edition can change licence, but the change applies forward only. That is why the choice was made once, at the start, rather than after the first complaint.

## 8. If you find a problem

If you believe a file here misuses someone else's work, or if you need a permission in writing, write to `suguohui_account@163.com`. I would rather fix an attribution than defend one.
