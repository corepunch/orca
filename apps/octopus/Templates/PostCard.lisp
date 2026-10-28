(Grid Columns="44px 1fr" Spacing=8 ForegroundColor=(bind "DataContext/Ink")
  (Button Name="Avatar" Width=44 Height=44 MarginTop=(bind "DataContext/AvatarTop") BackgroundColor="#00000000"
    class="octopus-row")
  (StackView Spacing=4
    (Button Name="RepostedBy" Visible=(bind "DataContext/IsRepost") Height=20 BackgroundColor="#00000000"
      class="octopus-row"
      (StackView Direction="Horizontal" AlignItems="Center" Spacing=4 IgnoreHitTest=true
        (VectorView Source="Octopus/assets/icons/repeat-2.svg" Width=14 Height=14 MarginTop="auto"
          MarginBottom="auto" ForegroundColor=(bind "DataContext/Dim") IgnoreHitTest=true)
        (TextBlock Text=(bind "DataContext/RepostedText") FontSize=13 ForegroundColor=(bind "DataContext/Dim")
          TextOverflow="Ellipsis" TextWrapping="NoWrap" IgnoreHitTest=true)))
    (Button Name="Author" Height=24 BackgroundColor="#00000000" class="octopus-row"
      (Grid Columns="1fr auto" Spacing=8 IgnoreHitTest=true
        (TextBlock ForegroundColor=(bind "DataContext/Ink") Text=(bind "DataContext/AuthorName") FontSize=16
          FontWeight="Bold" TextOverflow="Ellipsis" TextWrapping="NoWrap" IgnoreHitTest=true)
        (TextBlock Text=(bind "DataContext/Age") FontSize=13 ForegroundColor=(bind "DataContext/Dim")
          IgnoreHitTest=true)))
    (Button Name="Open" BackgroundColor="#00000000" class="octopus-row")
    (StackView Name="TagsHost" Visible=(bind "DataContext/HasTags"))
    (Grid Name="Actions" Columns="1fr 1fr 1fr" Height=44
      (Button Name="Comment" Height=44 BackgroundColor="#00000000" class="octopus-row"
        (StackView Direction="Horizontal" AlignItems="Center" Spacing=8 Height=28 MarginBottom="auto"
          IgnoreHitTest=true
          (VectorView Source="Octopus/assets/icons/message-circle.svg" Width=20 Height=20 MarginTop="auto"
            MarginBottom="auto" ForegroundColor=(bind "DataContext/Dim") IgnoreHitTest=true)
          (TextBlock Text=(bind "DataContext/Comments") FontSize=13 ForegroundColor=(bind "DataContext/Dim")
            IgnoreHitTest=true)))
      (Button Name="Repost" Height=44 BackgroundColor="#00000000" class="octopus-row"
        (StackView Direction="Horizontal" AlignItems="Center" Spacing=8 Height=28 MarginBottom="auto"
          IgnoreHitTest=true
          (VectorView Source="Octopus/assets/icons/repeat-2.svg" Width=20 Height=20 MarginTop="auto"
            MarginBottom="auto" ForegroundColor=(bind "DataContext/RepostColor") IgnoreHitTest=true)
          (TextBlock Text=(bind "DataContext/Reposts") FontSize=13 ForegroundColor=(bind "DataContext/RepostColor")
            IgnoreHitTest=true)))
      (Button Name="Like" Height=44 BackgroundColor="#00000000" class="octopus-row"
        (StackView Direction="Horizontal" AlignItems="Center" Spacing=8 Height=28 MarginBottom="auto"
          IgnoreHitTest=true
          (VectorView Source="Octopus/assets/icons/heart.svg" Width=20 Height=20 MarginTop="auto"
            MarginBottom="auto" ForegroundColor=(bind "DataContext/LikeColor") IgnoreHitTest=true)
          (TextBlock Text=(bind "DataContext/Likes") FontSize=13 ForegroundColor=(bind "DataContext/LikeColor")
            IgnoreHitTest=true))))))
