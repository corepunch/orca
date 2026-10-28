(StackView Spacing=8 IgnoreHitTest=true ForegroundColor=(bind "DataContext/Ink")
  (Grid Name="PhotoFrame" Visible=(bind "DataContext/HasPhoto") IgnoreHitTest=true
    (VectorView Source="Octopus/assets/icons/image.svg" Width=28 Height=28 IgnoreHitTest=true
      ForegroundColor=(bind "DataContext/Dim") MarginLeft="auto" MarginRight="auto" MarginTop="auto"
      MarginBottom="auto"))
  (TextBlock ForegroundColor=(bind "DataContext/Ink") Visible=(bind "DataContext/HasCaption")
    Name="PostBodyCaption" Text=(bind "DataContext/Caption") FontSize=15 WordWrap=true IgnoreHitTest=true)
  (Grid Visible=(bind "DataContext/IsQuote") Columns="4px 1fr" Spacing=12 IgnoreHitTest=true
    (Grid BackgroundColor=(bind "DataContext/Accent") IgnoreHitTest=true)
    (StackView Spacing=4 IgnoreHitTest=true
      (TextBlock ForegroundColor=(bind "DataContext/Ink") Name="PostBodyQuote" Text=(bind "DataContext/Quote")
        FontSize=(bind "DataContext/QuoteSize") WordWrap=true IgnoreHitTest=true)
      (TextBlock Visible=(bind "DataContext/HasQuoteSource") Name="PostBodyQuoteSource"
        Text=(bind "DataContext/QuoteSource") FontSize=14 ForegroundColor=(bind "DataContext/Muted") WordWrap=true
        IgnoreHitTest=true)))
  (TextBlock ForegroundColor=(bind "DataContext/Ink") Visible=(bind "DataContext/HasThoughts")
    Name="PostBodyThoughts" Text=(bind "DataContext/Thoughts") FontSize=15 WordWrap=true IgnoreHitTest=true)
  (Grid Visible=(bind "DataContext/IsLink") Columns="40px 1fr" Spacing=12 IgnoreHitTest=true
    (Grid Width=40 Height=40 BackgroundColor=(bind "DataContext/Accent") IgnoreHitTest=true
      (VectorView Source="Octopus/assets/icons/link.svg" Width=20 Height=20 MarginLeft="auto" MarginRight="auto"
        MarginTop="auto" MarginBottom="auto" ForegroundColor=(bind "DataContext/OnAccent") IgnoreHitTest=true))
    (StackView Spacing=4 IgnoreHitTest=true
      (TextBlock Name="PostBodyLinkTitle" Text=(bind "DataContext/LinkTitle")
        FontSize=(bind "DataContext/LinkSize") ForegroundColor=(bind "DataContext/AccentText") WordWrap=true
        IgnoreHitTest=true)
      (TextBlock Name="PostBodyDomain" Text=(bind "DataContext/Domain") FontSize=13
        ForegroundColor=(bind "DataContext/Dim") TextOverflow="Ellipsis" TextWrapping="NoWrap" IgnoreHitTest=true)))
  (TextBlock ForegroundColor=(bind "DataContext/Ink") Visible=(bind "DataContext/IsText") Name="PostBodyTitle"
    Text=(bind "DataContext/Title") FontSize=(bind "DataContext/TitleSize") WordWrap=true IgnoreHitTest=true)
  (TextBlock Visible=(bind "DataContext/HasContent") Name="PostBodyContent" Text=(bind "DataContext/Content")
    FontSize=(bind "DataContext/ContentSize") ForegroundColor=(bind "DataContext/ContentColor") WordWrap=true
    IgnoreHitTest=true))
