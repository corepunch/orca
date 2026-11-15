# Encarta

An iPad-first, offline encyclopedia for emerging readers, written in the
spirit of the *Was ist Was* books: every topic starts from a question a child
would ask. The home screen is a wall of illustrated questions, or a bookshelf
of numbered covers, filtered by category or search. An article is read one
question per page, then a “Did you know?” page with the word lab, then a short
quiz. The library has 15 illustrated articles. Search, both reading levels, word
definitions, and reading checks work without a connection. Tap **Read aloud** in the reader to listen to the selected level.
The article follows the recording and marks each spoken word. Use **Pause**
and **Resume** to take a break, or **Stop** to start over. The app needs no
account or service.

## Run it

Open `encarta.xcodeproj` in Xcode, select `encarta-ios`, and choose an iPad
simulator. Select a signing team in Xcode to run it on a physical iPad. To
regenerate the project from the repository root:

```sh
sh tools/xcode/generate.sh encarta --display-name Encarta
```

For the Make-based simulator build, desktop preview, and focused test:

```sh
make ios-run PROJECT=apps/encarta BUNDLE_ID=org.orca.encarta DISPLAY_NAME=Encarta DEVICE_TYPE=iPad
make unite
build/bin/orca apps/encarta
make test-encarta
```

## Write another article

Follow the article voice in the repository's `AGENTS.md` (*Encarta: how to
write articles*).

Add one record to [Content/Articles.lua](Content/Articles.lua). It appears
automatically in browse and search, and can be linked from other articles by ID.
Keep the layout in [Screens/Library.xml](Screens/Library.xml) and [Prefabs/](Prefabs/),
and the visual treatment in [Styles/app.css](Styles/app.css).

| Fields | What to write |
| --- | --- |
| `id`, `title`, `category`, `deck`, `keywords` | Stable unique ID, visible topic metadata, and words a child might search for. A new category also needs a color in `package.lua`, a `.cat-<slug>` and `.chip-<slug>` rule in the stylesheet, and an icon in `Icons/`. |
| `question` | The cover question shown on the wall and the bookshelf, such as “Why is Mars red?”. |
| `fact` | One astonishing, checkable “Did you know?” sentence. |
| `cover` (optional) | A bundled cover path for the Questions overview and Bookshelf. Leave it out to reuse the article's illustration. If neither is supplied, the cover uses the category icon. |
| `first`, `more` | Two versions of the same topic. Each section is a question heading followed by one or more paragraphs, and is shown as its own page. A section can also have a named `image = { path = …, caption = … }` field. Use this for a picture or diagram that helps answer that question. |
| `image` (optional) | A bundled image path and a plain-language caption. Used on the first question of either reading level unless that section specifies its own image. |
| `words` | Word, plain pronunciation guide, and meaning in context. Pick words used in the topic. |
| `quiz` | One question with choices, a 1-based correct choice, and a short explanation. Make sure either reading level gives enough information to answer it. |
| `related` | IDs of other articles in the bundle. |
| `sources` | References for editorial review. The app never opens or fetches these URLs. |

Keep artwork under [Images/](Images/) as landscape 3:2 JPEGs. Keep the main
subject near the middle for the bookshelf crop. The reader shows the whole
image so diagram labels stay in view. See [artwork prompts](Artwork.md) for
the painted illustrations and diagrams.

## Update a recording

Each reading level has a WAV recording and word timings under `Narration/`.
The generator reads headings and paragraphs directly from `Content/Articles.lua`.
The app checks the transcript before showing the Read aloud button. Regenerate
the recordings after changing a heading or paragraph.

Run the generator on macOS with Python 3.12, `piper-tts==1.3.0`,
`pocketsphinx==5.0.4`, and the Kristin medium voice and adjacent ONNX config.
Download the voice from the [Kristin model and card](https://huggingface.co/rhasspy/piper-voices/tree/bae641d/en/en_US/kristin/medium).
The model card identifies LibriVox public-domain recordings as the dataset.
The model and tools are needed only to regenerate the audio.

```sh
python3 apps/encarta/Tools/generate_narration.py /path/to/en_US-kristin-medium.onnx
```

## Design reference

[Microsoft’s Encarta guide](https://download.microsoft.com/documents/uk/athome/SM_Learn_5MinEncarta_F.pdf)
shows a searchable index, text with related links, and a separate area for
optional media. [Microsoft’s Encarta Kids announcement](https://news.microsoft.com/source/2004/07/07/microsoft-encarta-provides-fun-features-to-a-new-generation-of-students/)
describes a separate library of articles for children, with pictures, color,
and themed categories. This app keeps the shared article structure and links.
Its two reading levels, Word Lab, and comprehension checks are new choices for
learning to read.

[The octopus illustration](Images/octopus.jpg) was created with built-in
ImageGen and encoded as a JPEG for the offline bundle.

> Create a single widescreen landscape hero illustration for a sophisticated
> offline kids encyclopedia iPad app, editorial natural history book
> illustration. Topic: a common octopus, one orange terracotta octopus with
> anatomically credible curving arms and visible pale suckers, close up in a
> rich very dark marine blue and teal rocky underwater habitat with a few
> small sea plants, light shafts filtered down from the top left, elegant
> bold graphic shapes, silkscreen texture and hand-painted edges, reminiscent
> of contemporary museum exhibit posters. Strong orange and cobalt color
> contrast. Compose the octopus in the middle-right with room around the
> subject so cropping to a wide banner works; fill the frame, no border, NO
> lettering, numbers, symbols, logos, or typography. Friendly and fascinating,
> neither cartoon mascot nor scary. Landscape 3:2.
