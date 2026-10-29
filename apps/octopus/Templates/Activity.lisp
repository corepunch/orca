(Button Click="OpenPost" BackgroundColor="#00000000" class="octopus-row" StateClasses=(bind "DataContext/Kind")
  (Grid Columns="44px 1fr" Spacing=8 IgnoreHitTest=true
    (Grid Width=44 Height=44 StateClasses=(bind "DataContext/Tone") IgnoreHitTest=true
      (TextBlock Text=(bind "DataContext/Initials") FontSize=19 class="avatar-initials" TextHorizontalAlignment="Center" TextVerticalAlignment="Center" IgnoreHitTest=true)
      (ImageView Name="AvatarPhoto" Visible=(bind "DataContext/HasAvatar") Src=(bind "DataContext/AvatarPath") ForegroundColor="#ffffff" Stretch="UniformToFill" IgnoreHitTest=true))
    (StackView Spacing=4 JustifyContent="Center" IgnoreHitTest=true
      (Grid Columns="18px 1fr auto" Spacing=8 IgnoreHitTest=true
        (VectorView Source=(bind "DataContext/Icon") Width=16 Height=16 MarginTop="auto" MarginBottom="auto" class="activity-icon" IgnoreHitTest=true)
        (TextBlock Text=(bind "DataContext/Line") FontSize=15 FontWeight="Bold" TextOverflow="Ellipsis" TextWrapping="NoWrap" class="octopus-ink" IgnoreHitTest=true)
        (TextBlock Text=(bind "DataContext/Age") FontSize=13 class="octopus-dim" IgnoreHitTest=true))
      (TextBlock Visible=(bind "DataContext/HasBody") Text=(bind "DataContext/Excerpt") FontSize=15 WordWrap=true class="octopus-ink" IgnoreHitTest=true)
      (TextBlock Visible=(bind "DataContext/HasSummary") Text=(bind "DataContext/Summary") FontSize=13 TextOverflow="Ellipsis" TextWrapping="NoWrap" class="octopus-dim" IgnoreHitTest=true))))
