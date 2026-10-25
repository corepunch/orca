(Screen Width=1280 Height=800 bg="gray-950"
  (Grid Rows="64px 1fr" gap=0
    (Node2D bg="gray-900"
      (StackView dir="horizontal" gap=8 items="center" m=10
        (TextBlock text="xl violet-500" w=18 Text="ORCA")
        (TextBlock text="sm slate-400" Text="Overview")
        (TextBlock text="sm slate-400" Text="Features")
        (TextBlock text="sm slate-400" Text="Gallery")
        (TextBlock text="sm slate-400" Text="Docs")))
    (StackView dir="vertical" gap=0 overflow-y="scroll" overflow="hidden"
      (Grid Columns="1fr 1fr" gap=6 bg="gray-950"
        (Node2D p=14
          (StackView dir="vertical" gap=0
            (TextBlock text="5xl slate-100" Text="Build stunning")
            (TextBlock text="5xl violet-500" Text="UI in real-time.")
            (Node2D h=5)
            (TextBlock text="sm slate-500" wrap=true
              Text="A modern UI engine for games and native apps. Compose layouts from grids, stacks, text, and images — all rendered at 60 fps.")
            (Node2D h=10)
            (Grid Columns="1fr 1fr 1fr" gap=4
              (Node2D bg="gray-900" p=5 w=32
                (StackView dir="vertical" gap=1
                  (TextBlock text="3xl violet-500" Text="2.4 M")
                  (TextBlock text="xs gray-600" Text="Active users")))
              (Node2D bg="gray-900" p=5 w=32
                (StackView dir="vertical" gap=1
                  (TextBlock text="3xl emerald-500" Text="99.9 %")
                  (TextBlock text="xs gray-600" Text="Uptime SLA")))
              (Node2D bg="gray-900" p=5 w=32
                (StackView dir="vertical" gap=1
                  (TextBlock text="3xl blue-500" Text="1.2 ms")
                  (TextBlock text="xs gray-600" Text="Frame time"))))))
        (ImageView src="Example/Images/people" stretch="uniform-fill" mr=14))
      (Node2D bg="gray-950" p=10
        (Grid Columns="1fr 1fr 1fr" gap=5
          (Node2D bg="gray-900" p=7
            (StackView dir="vertical" gap=2
              (TextBlock text="xl violet-500" bg="violet-950" w=11 h=10 align="center middle" Text="Aa")
              (Node2D h=2)
              (TextBlock text="base slate-100" Text="Text Rendering")
              (TextBlock text="xs gray-600" wrap=true
                Text="Pixel-perfect typography with kerning, multi-language support, and full font control.")))
          (Node2D bg="gray-900" p=7
            (StackView dir="vertical" gap=2
              (TextBlock text="xl emerald-500" bg="emerald-950" w=11 h=10 align="center middle" Text="#")
              (Node2D h=2)
              (TextBlock text="base slate-100" Text="Flexible Layouts")
              (TextBlock text="xs gray-600" wrap=true
                Text="Grid and stack containers that adapt to any resolution, defined declaratively in Lisp.")))
          (Node2D bg="gray-900" p=7
            (StackView dir="vertical" gap=2
              (ImageView src="Example/Images/dog" w=48 h=20)
              (TextBlock text="base slate-100" Text="Image Integration")
              (TextBlock text="xs gray-600" wrap=true
                Text="Fast texture loading with stretch, crop, and nine-patch rendering built in."))))))))
