# Writing Encarta articles

Our first articles were too thin. Most sections were a heading and two or
three short sentences, so the child got an answer but never an explanation. A
*Was ist Was* page does more. It asks a question, answers it, and then keeps going: why it
happens, what it looks like, an example, a surprise, and what comes next. This
guide shows how they do that and sets the length and shape we write to.

The voice rules in the repository's `AGENTS.md` (*Encarta: how to write
articles*) still apply. The field layout is in the
[README](README.md#write-another-article). This file covers how much to write
and how to build a section.

## What we analysed

One real *Was ist Was* text: the publisher's 16-page reading sample of
**WAS IST WAS Edition – *Das Buch*** (Dr. Bernd Flessner, Tessloff 2019, ages 8
and up, 64 pages). It follows a book from the first idea to the shop shelf:
the idea, production, pictures and graphics, printing and binding, marketing,
the bookshop, and book fairs and libraries. Each chapter fills one two-page
spread.

We measured twelve sections of body text, each from its subheading to the next
box or subheading, and compared them with the 15 articles in
[Content/Articles.lua](Content/Articles.lua): first as they were before this
guide (the longer of the two reading levels the app had then), and again after
they were rewritten to it.

| Measure | *Was ist Was* | Encarta before | Encarta now |
| --- | --- | --- | --- |
| Words per section | **84** (37–144) | 53 | 119 (96–139) |
| Sentences per section | **6** (3–12) | 5 | 9 |
| Words per sentence | **13** (5–41) | 10.6 | 13 |
| Sections per topic | **4–6** per spread | 2 | 4–5 |
| Words per topic (body) | **about 450** | about 110 | about 540 |

A single spread held about four times as much text as one of our whole
articles. The sentences themselves were not much longer. The difference was
that each section had *more of them*, and each one added something.

## How a spread is built

1. **Chapter title and lead.** Each spread opens with a short lead in larger
   type, starting with a drop cap. It sets the scene in three to five
   sentences, and some leads open with a string of quick questions (one asks
   how big, how thick, how long a book should be) before answering.
2. **Three to five subheadings.** Each one has a block of body text of 50 to
   150 words. Some subheadings are questions (*What holds a book together?*,
   *Stitched or glued?*, *Does the sum add up?*). Others are short labels
   (*The author begins*, *A shared schedule*).
3. **Boxes beside the text.** A *Schon gewusst?* ("Did you know?") box has one
   or two sentences, for example where a word comes from. A side box with a
   question title has about three sentences. One labelled diagram names each
   part in one or two sentences. One checklist asks the questions a
   professional would ask.
4. **Captions.** Each photo gets a two- or three-word title (*Well informed*,
   *Step inside!*) and one or two sentences that link the picture to the text.

Our article maps onto this: `question` and `deck` are the lead, `sections`
are the subheadings, and `image` captions are the captions. The reader has no
side boxes or glossary, so the surprise and the new words live in the section
text itself.

## How a section fills up

A *Was ist Was* section is never "question, answer, stop". Following the
sentences in order, the same moves keep coming back:

1. **Answer or set the scene.** The first sentence carries the main point
   or picks up from the step before. ("Once everything is agreed, the
   author starts work.")
2. **Add the "but" or the "first".** The next sentence complicates or orders
   it. ("Before the writing comes the research.")
3. **Name the thing and explain it right away.** A new term comes with its
   meaning in the same or the next sentence: "that means…", "a kind of…",
   "this is called…", or the term in brackets after a plain description. In
   one 450-word stretch the sample does this about eight times.
4. **Give an example.** "For example" appears 17 times in the sample, about
   once per section. The example is concrete and familiar to a child: a book
   about ancient Egypt that sends the author to a museum, a puzzle book held
   together with two staples, a giraffe that must look exactly real in a
   nonfiction book but may talk in a picture book.
5. **Explain why.** "Because…" (*denn*) and "so that…" sentences give the
   reason. One example: some pictures cannot be photographed, because when
   dinosaurs lived nobody was there with a camera.
6. **Contrast two cases.** Large print runs versus small ones, thin booklets
   versus thick books, specialist shops versus general ones. Pairs make the
   rule visible.
7. **Give one surprise.** "What is amazing is that…" points to a single
   astonishing detail, such as every colour in the book being printed from
   just four inks.
8. **Connect it to the child's world.** Something the reader already knows
   ("much like your printer at home").
9. **End with a consequence or a handoff.** The last sentence draws a
   conclusion ("That is why…", "the more…, the more…") or passes the story to
   the next section ("…then the design can begin.").

Not every section uses all nine moves. A typical one uses five or six, in
roughly this order. Questions can also appear *inside* the text, as the
questions the people in the story ask themselves (should it be big or
small? which age group is it for?), and each one is answered straight after.

Sentence length varies on purpose. Short sentences of 5–8 words sit between
longer ones of 15–25 words, and the longest run up to about 40. The text reads
as explanation, not a list of facts.

## Our targets

| | Target |
| --- | --- |
| Sections | 4–5 |
| Paragraphs per section | 2 |
| Sentences per section | 7–10 |
| Words per section | 90–140 |
| Words per sentence | 10–15, some shorter, up to about 25 |
| Whole article | 450–650 words |

The reader page scrolls (`ReaderScroll` in
[Screens/Library.xml](Screens/Library.xml)), so a longer section is fine. One
section is still one question. If a section needs more than about 140 words,
it is answering two questions, so split it.

A section with only two or three sentences fails this guide unless it is a
short "try this" activity at the end.

## Recipe for one section

Write the heading as a question a child would ask. Then:

| # | Move | Sentences |
| --- | --- | --- |
| 1 | Answer the heading directly. | 1 |
| 2 | Show how or why it happens, step by step. | 2–3 |
| 3 | Name one new word and explain it in place. | 1 |
| 4 | Give one concrete example: a named animal, place, or object. | 1–2 |
| 5 | Make one number tangible with a comparison from the child's world. | 1 |
| 6 | Add a contrast, a surprise, or something to try. | 1 |
| 7 | End with a consequence or point to the next question. | 1 |

Split this into two paragraphs: moves 1–3 explain how it works, and moves
4–7 give the example and the wider picture.

### Worked example

This was the octopus colour section before the rewrite, two paragraphs and
five sentences (57 words):

> **A disguise that changes**
> Tiny sacs of color sit just under an octopus's skin. Muscles stretch these
> sacs to make the color easier to see. The octopus can switch from light to
> dark or make a spotted pattern.
> Some octopuses can make their skin bumpy as well. Matching the color and
> shape of the ground can make them very hard to spot.

The same section rewritten with the recipe:

> **How does an octopus change color?**
> An octopus changes color by stretching tiny bags of paint in its skin.
> Thousands of these bags sit just under the surface. Each one is about as
> small as a pencil dot and holds red, yellow, brown, or black color. Tiny
> muscles pull on the bag and spread it out, so the color shows. When the
> muscles relax, the bag shrinks to a dot and the color almost disappears.
> These color bags are called chromatophores.
>
> Because the octopus's brain controls the muscles directly, the change is
> fast. A frightened octopus can go from pale to dark in less than a second,
> before you could say its name. It can also raise little bumps on its skin
> so it looks rough like a rock or a piece of coral. Divers sometimes swim
> past an octopus without noticing it at all. Hiding like this keeps it safe
> from sharks and big fish, and it also lets the octopus sneak up on crabs.

It has 12 sentences and about 170 words, which is at the top of the range,
so a real article would move the bumpy-skin part into its own section, "Why
does an octopus's skin get bumpy?". Check every new number against the
article's `sources` before publishing.

## Captions

A caption is one or two sentences, 10–30 words, that tell the child what to
look for in the picture ("Look for the yellow pollen on its back leg"). It
should not repeat the body text.

## Checklist before you commit

- [ ] Every heading is a question, answered in the sentence below it.
- [ ] Every section meets the word and sentence ranges above.
- [ ] Each section has at least one example and one "because".
- [ ] Each new term is explained where it first appears.
- [ ] Each large number has a comparison a child can picture.
- [ ] No section is just a list; sentences link with *because*, *so*,
      *then*, *but*, *for example*.
- [ ] New facts and numbers are covered by `sources`.
- [ ] If the article has a recording in `Narration/`, regenerate it (see
      [Update a recording](README.md#update-a-recording)).

## Where *Was ist Was* and Encarta differ

- **Who it is for.** The sample is from the main series, for ages 8 and up.
  Our readers start at 6, so keep most sentences at 10–15 words and let only
  a few run longer, even where the sample would.
- **Addressing the reader.** The sample uses "you" only now and then, a
  couple of times in 16 pages. We use it more, as `AGENTS.md` asks. Use it
  for a single prompt ("Press two fingers on your wrist…") in a section,
  not in every sentence.
- **Headings.** In the sample only some subheadings are questions. Ours all
  are, because the question-led reading flow is the point of the app.
