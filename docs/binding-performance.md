# Binding performance

The manual benchmark for issue #436 builds an offline Octopus Home feed using
the actual `Feeds.xml` datasource and `PostCard.xml` templates. Text, photo,
quote and link posts repeat in a fixed order. No network client or image
downloads are involved. Fixture creation, queued startup work and garbage
collection are outside the timed interval. Each sample measures 200 unchanged
`advanceFrame()` + `runAllPrograms(screen)` passes with `os.clock()` CPU time;
five samples report their median, minimum and maximum. The script asserts the
requested row count so an empty or partially loaded feed cannot look fast.

```sh
make bench-bindings
BENCH_POSTS=50 BENCH_FRAMES=200 BENCH_TRIALS=5 make bench-bindings
BENCH_POSTS=100 BENCH_FRAMES=200 BENCH_TRIALS=5 make bench-bindings
BENCH_POSTS=200 BENCH_FRAMES=200 BENCH_TRIALS=5 make bench-bindings
```

After building, run `BENCH_POSTS=200 build/bin/orca -test=tests/bench_bindings.lua`
to avoid rebuilding between repeated measurements. This benchmark requires the
default XML scene build and is deliberately excluded from CI timing assertions.

## Measurements

Measured on 31 October 2026 in the same Linux cloud container with `-O2 -g`.
The baseline engine and templates were from `bb7a9c06`; the same benchmark
script, deterministic fixture, viewport, frame count and sample count were
used before and after the change. The node counts remained identical.

| Posts | Nodes | Before median (range), ms/pass | After median (range), ms/pass |
| ---: | ---: | ---: | ---: |
| 50 | 2,653 | 0.968 (0.913–1.182) | 0.125 (0.112–0.137) |
| 100 | 5,303 | 1.925 (1.900–2.340) | 0.244 (0.237–0.377) |
| 200 | 10,603 | 10.527 (9.616–11.778) | 0.521 (0.511–0.542) |

The unchanged 200-post pass is about 20 times faster and below the issue's
2 ms target. These are cloud CPU measurements, with host-load variability;
they do not measure iPhone performance, layout, painting or total frame time.
The final follow-up changes invalidate caches on array mutations and reset
restored binding state; they do not alter the unchanged benchmark path.

## Binding modes and invalidation

Row templates in Chat and Octopus now explicitly use data-backed snapshots:

```xml
<TextBlock Text="{Binding DataContext/Post/Title, Mode=OneTime}"/>
```

`OneTime` evaluates when its source becomes available, then skips execution
on unchanged passes. It is refreshed after DataObject changes and after
hierarchy or DataContext invalidation, so edits, asynchronous media state and
replacement records can still reach existing rows. Serialization preserves
the mode. Binding programs cache resolved property paths only while their
invalidation epoch remains valid.

Invalidation currently uses conservative global data and graph epochs. A
DataObject change invalidates data-backed snapshots globally; it is not yet a
per-record or per-field subscription mechanism. This trades some work on
changed frames for simple lifetime safety and a cheap unchanged path. Object
destruction, component changes, reparenting and object-valued path replacement
must invalidate resolved pointers before another evaluation can use them.

Use `OneTime` for record-derived values. Keep default `OneWay` for bindings
that depend on live UI geometry, window dimensions, animation or other
non-record state. `TwoWay` editing behavior remains unchanged. The templates
were migrated only for plain `DataContext/...` paths; no template compiler or
etlua dependency was introduced. The existing row tree is retained.

`tests/xml/test_one_time_bindings_xml.lua` covers unchanged-pass skipping,
record mutation, inherited context replacement, reparenting, destruction and
rebinding, XML shorthand, mode serialization and live `OneWay` geometry.
