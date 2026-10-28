(Grid Columns="36px 1fr" Spacing=12 MarginLeft=(bind "DataContext/Indent") ForegroundColor=(bind "DataContext/Ink")
  (Button Name="Avatar" Width=36 Height=36 BackgroundColor="#00000000" class="octopus-row")
  (StackView Spacing=4
    (Grid Columns="1fr auto" Spacing=8 IgnoreHitTest=true
      (TextBlock ForegroundColor=(bind "DataContext/Ink") Text=(bind "DataContext/AuthorName") FontSize=14
        FontWeight="Bold" TextOverflow="Ellipsis" TextWrapping="NoWrap" IgnoreHitTest=true)
      (TextBlock Text=(bind "DataContext/Age") FontSize=12 ForegroundColor=(bind "DataContext/Dim")
        IgnoreHitTest=true))
    (TextBlock ForegroundColor=(bind "DataContext/Ink") Text=(bind "DataContext/Body") FontSize=15 WordWrap=true
      IgnoreHitTest=true)
    (StackView Direction="Horizontal" AlignItems="Center" Spacing=20 Height=30
      (Button Name="Like" Height=30 Width=64 BackgroundColor="#00000000" class="octopus-row"
        (StackView Direction="Horizontal" AlignItems="Center" Spacing=6 IgnoreHitTest=true
          (VectorView Source="Octopus/assets/icons/heart.svg" Width=16 Height=16 MarginTop="auto"
            MarginBottom="auto" ForegroundColor=(bind "DataContext/LikeColor") IgnoreHitTest=true)
          (TextBlock Text=(bind "DataContext/Likes") FontSize=13 ForegroundColor=(bind "DataContext/LikeColor")
            IgnoreHitTest=true)))
      (Button Name="Reply" Text="reply" Height=30 Width=48 FontSize=13 TextHorizontalAlignment="Left"
        TextVerticalAlignment="Center" ForegroundColor=(bind "DataContext/AccentText") BackgroundColor="#00000000"
        class="octopus-action"))))
