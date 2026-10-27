(Screen id="Application" Width=1280 Height=800 class="bg-gray-950"
  (Grid id="Root" Rows="64px 1fr" class="gap-0"
    (Node2D id="NavBar" class="bg-gray-900"
      (StackView id="NavLinks" Direction="Horizontal" class="flex-row gap-8 items-center m-10"
        (TextBlock id="Brand" class="text-xl text-violet-500 w-18" Text="ORCA")
        (TextBlock id="NavOverview" class="text-sm text-slate-400" Text="Overview")
        (TextBlock id="NavFeatures" class="text-sm text-slate-400" Text="Features")
        (TextBlock id="NavGallery" class="text-sm text-slate-400" Text="Gallery")
        (TextBlock id="NavDocs" class="text-sm text-slate-400" Text="Docs")))
    (StackView id="Body" Direction="Vertical" class="flex-col gap-0 overflow-y-scroll overflow-hidden"
      (Grid id="Hero" Columns="1fr 1fr" class="gap-6 bg-gray-950"
        (Node2D id="HeroContent" class="p-14"
          (StackView id="HeroStack" Direction="Vertical" class="flex-col gap-0"
            (TextBlock id="HeadlineLine1" class="text-5xl text-slate-100" Text="Build stunning")
            (TextBlock id="HeadlineLine2" class="text-5xl text-violet-500" Text="UI in real-time.")
            (Node2D id="HeadlineSpacer" class="h-5")
            (TextBlock id="SubHeadline" class="text-sm text-slate-500 flex-wrap"
              Text="A modern UI engine for games and native apps. Compose layouts from grids, stacks, text, and images — all rendered at 60 fps.")
            (Node2D id="MetricSpacer" class="h-10")
            (Grid id="Metrics" Columns="1fr 1fr 1fr" class="gap-4"
              (Node2D id="MetricUsers" class="bg-gray-900 p-5 w-32"
                (StackView Direction="Vertical" class="flex-col gap-1"
                  (TextBlock id="MetricUsersValue" class="text-3xl text-violet-500" Text="2.4 M")
                  (TextBlock id="MetricUsersLabel" class="text-xs text-gray-600" Text="Active users")))
              (Node2D id="MetricUptime" class="bg-gray-900 p-5 w-32"
                (StackView Direction="Vertical" class="flex-col gap-1"
                  (TextBlock id="MetricUptimeValue" class="text-3xl text-emerald-500" Text="99.9 %")
                  (TextBlock id="MetricUptimeLabel" class="text-xs text-gray-600" Text="Uptime SLA")))
              (Node2D id="MetricFrame" class="bg-gray-900 p-5 w-32"
                (StackView Direction="Vertical" class="flex-col gap-1"
                  (TextBlock id="MetricFrameValue" class="text-3xl text-blue-500" Text="1.2 ms")
                  (TextBlock id="MetricFrameLabel" class="text-xs text-gray-600" Text="Frame time"))))))
        (ImageView id="HeroImage" Source="Example/Images/people" Stretch="UniformToFill" class="mr-14"))
      (Node2D id="FeatureSection" class="bg-gray-950 p-10"
        (Grid id="FeatureCards" Columns="1fr 1fr 1fr" class="gap-5"
          (Node2D id="CardText" class="bg-gray-900 p-7"
            (StackView id="CardTextStack" Direction="Vertical" class="flex-col gap-2"
              (TextBlock id="CardTextIcon"
                class="text-xl text-violet-500 bg-violet-950 w-11 h-10 text-center align-middle"
                Text="Aa")
              (Node2D id="CardTextSpacer" class="h-2")
              (TextBlock id="CardTextTitle" class="text-base text-slate-100" Text="Text Rendering")
              (TextBlock id="CardTextBody" class="text-xs text-gray-600 flex-wrap"
                Text="Pixel-perfect typography with kerning, multi-language support, and full font control.")))
          (Node2D id="CardLayout" class="bg-gray-900 p-7"
            (StackView id="CardLayoutStack" Direction="Vertical" class="flex-col gap-2"
              (TextBlock id="CardLayoutIcon"
                class="text-xl text-emerald-500 bg-emerald-950 w-11 h-10 text-center align-middle"
                Text="#")
              (Node2D id="CardLayoutSpacer" class="h-2")
              (TextBlock id="CardLayoutTitle" class="text-base text-slate-100" Text="Flexible Layouts")
              (TextBlock id="CardLayoutBody" class="text-xs text-gray-600 flex-wrap"
                Text="Grid and stack containers that adapt to any resolution, defined declaratively in Lisp.")))
          (Node2D id="CardImage" class="bg-gray-900 p-7"
            (StackView id="CardImageStack" Direction="Vertical" class="flex-col gap-2"
              (ImageView id="CardImageThumb" Source="Example/Images/dog" class="w-48 h-20")
              (TextBlock id="CardImageTitle" class="text-base text-slate-100" Text="Image Integration")
              (TextBlock id="CardImageBody" class="text-xs text-gray-600 flex-wrap"
                Text="Fast texture loading with stretch, crop, and nine-patch rendering built in."))))))))
