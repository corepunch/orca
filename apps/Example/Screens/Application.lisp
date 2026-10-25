(Screen Name="Application" Width=1280 Height=800 ClearColor="$surface"
  StyleSheet="Example/Styles/app.css"
  (Grid Name="Root" Rows="64px 1fr" class="app-shell"
    (StackView Name="NavBar" class="panel navbar"
      (StackView Name="BrandMark" class="brand-mark"
        (VectorView Name="BrandIcon" class="brand-icon" Source="Example/Icons/blocks.svg")
        (TextBlock Name="Brand" class="brand-text" Text="ORCA"))
      (TextBlock Name="NavOverview" class="nav-link" Text="Overview")
      (TextBlock Name="NavFeatures" class="nav-link" Text="Features")
      (TextBlock Name="NavGallery" class="nav-link" Text="Gallery")
      (TextBlock Name="NavDocs" class="nav-link" Text="Docs"))
    (StackView Name="Body" class="app-body"
      Node.HorizontalPadding=(if (step 640 (bind "Screen.Width")) (vector2 40 40) (vector2 8 8))
      (Grid Name="Hero" Columns="1fr 1fr" class="hero" Node.Visible=(step 640 (bind "Screen.Width"))
        Grid.Columns=(if (step 640 (bind "Screen.Width")) "1fr 1fr" "1fr")
        (StackView Name="HeroContent" class="hero-content"
          (TextBlock Name="HeadlineLine1" class="hero-title" Text="Build stunning")
          (TextBlock Name="HeadlineLine2" class="hero-title-accent" Text="UI in real-time.")
          (Node2D Name="HeadlineSpacer" class="headline-spacer")
          (TextBlock Name="SubHeadline" class="hero-subtitle"
            Text="A modern UI engine for games and native apps. Compose layouts from grids, stacks, text, and images — all rendered at 60 fps.")
          (Node2D Name="MetricSpacer" class="metric-spacer")
          (GridBox Name="Metrics" Columns="1fr 1fr 1fr" class="grid"
            ItemsSource="Example/DataSources/ApplicationData:Metrics"
            ItemTemplate="Example/Prefabs/Metric.lisp"))
        (ImageView Name="HeroImage" Source="Example/Images/people" Stretch="UniformToFill"))
      (StackView Name="TabbedTechSection" class="section surface-section"
        Node.HorizontalPadding=(if (step 640 (bind "Screen.Width")) (vector2 40 40) (vector2 8 8))
        (TextBlock Name="TabbedTechHeading" class="section-heading" Text="Build the UI, not the ceremony")
        (TextBlock Name="TabbedTechSubheading" class="section-copy"
          Text="A clean Lisp surface, Lua where things move, MoonScript when the app grows, and one UI that can travel across targets.")
        (GridBox Name="OrcaSignals" Columns="1fr 1fr 1fr" class="grid"
          ItemsSource="Example/DataSources/ApplicationData:Signals"
          ItemTemplate="Example/Prefabs/SignalCard.lisp"
          Grid.Columns=(if (step 640 (bind "Screen.Width")) "1fr 1fr 1fr" "1fr 1fr"))
        (TabView Name="OrcaTabs" SelectedValue="xml"
          (TabBar Name="OrcaTabBar" class="panel tabbar"
            (Tab Name="XmlTab" Value="xml" class="tab" Text="Lisp")
            (Tab Name="LuaTab" Value="lua" class="tab" Text="Lua")
            (Tab Name="MoonTab" Value="moon" class="tab" Text="MoonScript")
            (Tab Name="DeployTab" Value="deploy" class="tab" Text="Deploy"))
          (StackView Name="xml" class="tab-panel"
            (LayerPrefabPlaceholder Name="XmlPanelHeader"
              PlaceholderTemplate="Example/Prefabs/TabPanelHeader.lisp"
              DataContextSource="Example/DataSources/ApplicationData:XmlPanelHeader/XmlPanelHeader")
            (Grid Name="XmlGrid" Columns="1fr 1fr" class="grid"
              (StackView Name="XmlCopy" class="copy-stack"
                (TextBlock Name="XmlBody" class="section-copy"
                  Text="The screen stays legible at a glance, which makes it easy to scan, compare, and reshape as the design evolves.")
                (StackView Name="XmlNotes" class="panel note-panel"
                  (TextBlock Name="XmlNote1" class="note-text"
                    Text="• Grids and stacks stay visible in the source.")
                  (TextBlock Name="XmlNote2" class="note-text"
                    Text="• Spacing, padding, and hierarchy remain explicit.")
                  (TextBlock Name="XmlNote3" class="note-text"
                    Text="• The shape of the screen is easy to read before it ever runs.")))
              (StackView Name="XmlModel" class="card stack-card model-panel"
                (TextBlock Name="XmlModelHeading" class="card-title" Text="The structure stays honest")
                (GridBox Name="XmlModelGrid" Columns="1fr" class="model-grid"
                  ItemsSource="Example/DataSources/ApplicationData:XmlModel"
                  ItemTemplate="Example/Prefabs/XmlModelNode.lisp"))))
          (StackView Name="lua" class="tab-panel"
            (LayerPrefabPlaceholder Name="LuaPanelHeader"
              PlaceholderTemplate="Example/Prefabs/TabPanelHeader.lisp"
              DataContextSource="Example/DataSources/ApplicationData:LuaPanelHeader/LuaPanelHeader")
            (TextBlock Name="LuaBody" class="section-copy"
              Text="The parts that move stay beside the screen that uses them, so interaction and state feel close and easy to change.")
            (GridBox Name="LuaGrid" Columns="1fr 1fr 1fr" class="grid"
              ItemsSource="Example/DataSources/ApplicationData:LuaCards"
              ItemTemplate="Example/Prefabs/IconCard.lisp"))
          (StackView Name="moon" class="tab-panel"
            (LayerPrefabPlaceholder Name="MoonPanelHeader"
              PlaceholderTemplate="Example/Prefabs/TabPanelHeader.lisp"
              DataContextSource="Example/DataSources/ApplicationData:MoonPanelHeader/MoonPanelHeader")
            (Grid Name="MoonGrid" Columns="1fr 1fr" class="grid"
              (StackView Name="MoonCopy" class="copy-stack"
                (TextBlock Name="MoonBody" class="section-copy"
                  Text="MoonScript keeps larger app code compact and expressive, while still compiling to the same Lua runtime.")
                (StackView Name="MoonNotes" class="panel note-panel"
                  (TextBlock Name="MoonNote1" class="note-text"
                    Text="• Helpful when app modules get a little larger.")
                  (TextBlock Name="MoonNote2" class="note-text"
                    Text="• The syntax stays concise, but the runtime stays Lua.")
                  (TextBlock Name="MoonNote3" class="note-text"
                    Text="• It pairs naturally with Lisp screens and Lua helpers.")))
              (StackView Name="MoonModel" class="card stack-card model-panel"
                (TextBlock Name="MoonModelHeading" class="card-title" Text="Same runtime, less ceremony")
                (TextBlock Name="MoonModelLine1" class="moon-model-line" Text="Cleaner class bodies")
                (TextBlock Name="MoonModelLine2" class="moon-model-line" Text="Readable app controllers")
                (TextBlock Name="MoonModelLine3" class="moon-model-line" Text="Plain Lua at runtime")
                (TextBlock Name="MoonModelLine4" class="moon-model-line"
                  Text="Comfortable beside Lisp layouts"))))
          (StackView Name="deploy" class="tab-panel"
            (LayerPrefabPlaceholder Name="DeployPanelHeader"
              PlaceholderTemplate="Example/Prefabs/TabPanelHeader.lisp"
              DataContextSource="Example/DataSources/ApplicationData:DeployPanelHeader/DeployPanelHeader")
            (TextBlock Name="DeployBody" class="section-copy"
              Text="Build once, tune the spacing, and carry the same design language across desktop, mobile, and web-oriented targets.")
            (GridBox Name="DeployGrid" Columns="1fr 1fr 1fr" class="grid"
              ItemsSource="Example/DataSources/ApplicationData:DeployCards"
              ItemTemplate="Example/Prefabs/IconCard.lisp")
            (GridBox Name="DeployImages" Columns="1fr 1fr" class="grid"
              ItemsSource="Example/DataSources/ApplicationData:DeployImages"
              ItemTemplate="Example/Prefabs/ImageCaptionCard.lisp")
            (StackView Name="DeployCommandLink" class="panel command-link"
              (VectorView Name="DeployCommandIcon" class="command-icon"
                Source="Example/Icons/monitor.svg")
              (StackView Name="DeployCommandCopy" class="command-copy"
                (TextBlock Name="DeployCommandLabel" class="command-label" Text="Desktop build")
                (TextBlock Name="DeployCommandValue" class="command-value"
                  Text="make clean && make -j8 && build/bin/orca apps/Example"))))))
      (GridBox Name="FeatureSection" Columns="1fr 1fr 1fr 1fr" class="grid feature-section"
        ItemsSource="Example/DataSources/ApplicationData:FeatureCards"
        ItemTemplate="Example/Prefabs/FeatureImageCard.lisp")
      (StackView Name="GallerySection" class="section surface-section gallery-section"
        Node.HorizontalPadding=(if (step 640 (bind "Screen.Width")) (vector2 40 40) (vector2 8 8))
        (TextBlock Name="GalleryHeading" class="section-heading" Text="Gallery")
        (TextBlock Name="GallerySubheading" class="section-copy"
          Text="Screenshots and scenes rendered entirely by the ORCA engine.")
        (GridBox Name="GalleryGrid" Columns="1fr 1fr 1fr" Spacing=16
          ItemsSource="Example/DataSources/ApplicationData:GalleryCards"
          ItemTemplate="Example/Prefabs/GalleryCard.lisp"))
      (StackView Name="WorkflowSection" class="section deep-section workflow-section"
        Node.HorizontalPadding=(if (step 640 (bind "Screen.Width")) (vector2 40 40) (vector2 8 8))
        (TextBlock Name="WorkflowHeading" class="section-heading" Text="How It Works")
        (TextBlock Name="WorkflowSubheading" class="section-copy"
          Text="From Lisp layout to a live rendered scene in four simple steps.")
        (GridBox Name="WorkflowSteps" Columns="1fr 1fr 1fr 1fr" class="grid"
          ItemsSource="Example/DataSources/ApplicationData:WorkflowSteps"
          ItemTemplate="Example/Prefabs/WorkflowStep.lisp"
          Grid.Columns=(if (step 640 (bind "Screen.Width")) "1fr 1fr 1fr 1fr" "1fr 1fr")))
      (StackView Name="TestimonialsSection" class="section surface-section testimonials-section"
        Node.HorizontalPadding=(if (step 640 (bind "Screen.Width")) (vector2 40 40) (vector2 8 8))
        (TextBlock Name="TestimonialsHeading" class="section-heading" Text="What Developers Say")
        (GridBox Name="TestimonialsGrid" Columns="1fr 1fr 1fr" class="grid"
          ItemsSource="Example/DataSources/ApplicationData:Quotes"
          ItemTemplate="Example/Prefabs/Quote.lisp"))
      (StackView Name="CtaSection" class="section panel cta-section"
        Node.HorizontalPadding=(if (step 640 (bind "Screen.Width")) (vector2 60 60) (vector2 16 16))
        (TextBlock Name="CtaHeading" class="cta-heading" Text="Ready to build something great?")
        (TextBlock Name="CtaBody" class="cta-body"
          Text="Download ORCA, open a sample project, and have your first screen running in minutes.")
        (StackView Name="CtaButtons" class="cta-button-row"
          (TextBlock Name="CtaButtonPrimary" class="button cta-button primary-button" Text="Get Started"
            LeftButtonUp=(Screen.ShowModal Path="Example/Screens/GetStartedPopup.lisp"))
          (TextBlock Name="CtaButtonSecondary" class="button cta-button secondary-button"
            Text="Learn more"))))))
