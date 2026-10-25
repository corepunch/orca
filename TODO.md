# TODO

- [ ] Profile `apps/chat` swipes on a physical device and record frame times,
  layout cost, and dropped frames before claiming smoothness. Windows Phone 7
  emphasized GPU/composition-thread animation and minimal UI-thread work
  ([Microsoft reference](https://download.microsoft.com/download/3/1/b/31b6c512-d66f-4138-b639-94711e95faca/msdn_0411dg.pdf)).
  ORCA still performs layout during animation; use the device profile to guide
  reducing that work and confirm the resulting swipe behavior is smooth.
