(Button class="chat-action" Height=56 BorderWidth="0" BackgroundColor="#00000000"
  (Grid Columns="52px 1fr" Spacing=12 IgnoreHitTest=true
    (Grid Name="ContactAvatar" Height=52 BackgroundColor=(bind "DataContext/Tone") IgnoreHitTest=true
      (TextBlock Text=(bind "DataContext/Initials") ForegroundColor=(bind "DataContext/Ink")
        FontSize=18 TextHorizontalAlignment="Center" TextVerticalAlignment="Center" IgnoreHitTest=true))
    (StackView Spacing=1 JustifyContent="Center" IgnoreHitTest=true
      (TextBlock Text=(bind "DataContext/Label") ForegroundColor=(bind "DataContext/Text") FontSize=22
        TrimLeading=true TextOverflow="Ellipsis" TextWrapping="NoWrap" IgnoreHitTest=true)
      (TextBlock Text=(bind "DataContext/Username") ForegroundColor=(bind "DataContext/Muted") FontSize=14
        TextOverflow="Ellipsis" TextWrapping="NoWrap" IgnoreHitTest=true))))
